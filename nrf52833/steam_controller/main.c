#include "hoja.h"
#include "board_config.h"
#include "main.h"

#include "hal/gpio_hal.h"
#include "hal/i2c_hal.h"
#include "hal/sys_hal.h"
#include "nrf52_hal.h"

#include "input/input_config.h"
#include "input_shared_types.h"

// Valve Steam Controller (2026). What isn't read yet: the trackpads, the grip sensors and the
// sticks' touch sensing (all behind the trackpad controller, which is left idle), and the
// trackpads' own haptics. Off the cable it is a Switch 2's Pro Controller 2 over Bluetooth LE, the
// one mode its radio carries here (HOJA_NS2_BLE); on the cable, the library's USB modes.
//
// The pins and the parts on them come from the community's work on the stock firmware. Buttons,
// sticks, triggers and motion have been checked on a controller; the entries marked "to confirm"
// below have not.

// Sticks: the whole supply range, read the way the stock firmware does (4 samples, 3 us each)
static const nrf52_adc_channel_s _stick_channels[4] = {
    {AIN_STICK_LX, SAADC_CH_CONFIG_GAIN_Gain1_4, SAADC_CH_CONFIG_REFSEL_VDD1_4, SAADC_CH_CONFIG_TACQ_3us},
    {AIN_STICK_LY, SAADC_CH_CONFIG_GAIN_Gain1_4, SAADC_CH_CONFIG_REFSEL_VDD1_4, SAADC_CH_CONFIG_TACQ_3us},
    {AIN_STICK_RX, SAADC_CH_CONFIG_GAIN_Gain1_4, SAADC_CH_CONFIG_REFSEL_VDD1_4, SAADC_CH_CONFIG_TACQ_3us},
    {AIN_STICK_RY, SAADC_CH_CONFIG_GAIN_Gain1_4, SAADC_CH_CONFIG_REFSEL_VDD1_4, SAADC_CH_CONFIG_TACQ_3us},
};

// Triggers: their sensors swing over half the supply, so twice the gain
static const nrf52_adc_channel_s _trigger_channels[2] = {
    {AIN_TRIGGER_L, SAADC_CH_CONFIG_GAIN_Gain1_2, SAADC_CH_CONFIG_REFSEL_VDD1_4, SAADC_CH_CONFIG_TACQ_3us},
    {AIN_TRIGGER_R, SAADC_CH_CONFIG_GAIN_Gain1_2, SAADC_CH_CONFIG_REFSEL_VDD1_4, SAADC_CH_CONFIG_TACQ_3us},
};

// Battery: against the 0.6 V reference at gain 1/6 (3.6 V full scale), a slow high-impedance divider
static const nrf52_adc_channel_s _battery_channel = {
    AIN_BATTERY, SAADC_CH_CONFIG_GAIN_Gain1_6, SAADC_CH_CONFIG_REFSEL_Internal, SAADC_CH_CONFIG_TACQ_40us,
};

#define ANALOG_SAMPLES          4
#define BATTERY_SAMPLES         8
// The battery divider: 221 k over 301 k, so the pack is 522/301 of what the pin sees
#define BATTERY_DIVIDER_TOP     522u
#define BATTERY_DIVIDER_BOTTOM  301u
#define BATTERY_ADC_FULL_MV     3600u
// The fuel gauge driver expects a 12-bit reading of half the pack against 3.3 V
#define FUELGAUGE_FULL_MV       6600u

#define BUTTONS_I2C_TIMEOUT_US  2000
#define BUTTONS_ALL_RELEASED    0xFFFFu

static const hoja_config_s _hoja_config = {
    .i2c = {
        .instance_0 = {
            .enabled      = true,
            .sda_gpio     = PGPIO_I2C0_SDA,
            .scl_gpio     = PGPIO_I2C0_SCL,
            .baudrate_khz = 400,
        },
        .instance_1 = {
            .enabled      = true,
            .sda_gpio     = PGPIO_I2C1_SDA,
            .scl_gpio     = PGPIO_I2C1_SCL,
            .baudrate_khz = 400,
        },
    },

    .spi = {
        .instance_0 = { .enabled = false },
        .instance_1 = { .enabled = false },
    },

    .battery = {
        .i2c_instance  = 0,
        // The chip sees the cable's power the moment it arrives
        .input_present = nrf52_usb_power_present,
    },

    .fuelgauge = {
        .voltage_offset = 0.0f,
    },

    // The sensor's axes in HOJA's frame (+X to the gamepad's left, +Y to its front edge, +Z up),
    // checked by tilting the pad: left side down reads -1 g on X, front edge down -1 g on Y
    .imu = {
        .i2c_instance = 0,
        .address      = 0x6A,
        .x = { .source = LSM6DSV16X_AXIS_Y, .invert = true },
        .y = { .source = LSM6DSV16X_AXIS_X, .invert = true },
        .z = { .source = LSM6DSV16X_AXIS_Z, .invert = false },
    },

    // Both grips work, and left and right are the right way round with the channels swapped. A
    // quarter of the amplifier's full scale felt right at a low rumble request; a full-strength
    // request is still to confirm.
    .haptics = {
        .sck_pin          = PGPIO_I2S_SCK,
        .lrck_pin         = PGPIO_I2S_LRCK,
        .sdout_pin        = PGPIO_I2S_SDOUT,
        .enable_pin       = PGPIO_HAPTIC_AMP_ENABLE,
        .channel_b_enable = true,
        .channel_swap     = true,
        .intensity_max    = 0.25f,
    },

    .battery_capacity_mah     = 2200, // 8.39 Wh
    .battery_part_code        = NULL,
    .battery_shutdown_percent = 0,

    .device_name   = "Steam Controller",
    .device_maker  = "Valve",
    // No updates through the config app: the firmware goes in through Valve's bootloader
    .manifest_url  = NULL,
    .firmware_url  = NULL,
    .manual_url    = NULL,
    .fcc_id        = NULL,

    // The library's defaults
    .usb_vid = 0,
    .usb_pid = 0,

    .sinput = {
        .gamepad_type       = SINPUT_SDL_GAMEPAD_TYPE_STEAM,
        .face_buttons_style = SINPUT_SDL_GAMEPAD_FACE_STYLE_ABXY,
        .gamepad_subtype    = 0,
    },

    .sewn_layout = SEWN_LAYOUT_ABXY,

    .tourney_macro_code = INPUT_CODE_UNUSED,

    // Steam held: off, as on the stock firmware
    .shipping_macro_code = { INPUT_CODE_HOME, INPUT_CODE_UNUSED },

    // Menu held at power-on, as Start is on the other gamepads: found by any Switch 2, though one is
    // saved. (With none saved it always is.) With L1 as well it is the bootloader instead.
    .sync_on_boot_code   = INPUT_CODE_START,
    .sync_macro_code     = { INPUT_CODE_UNUSED, INPUT_CODE_UNUSED },

    // Into Valve's bootloader. (The bootloader's own way in, whatever firmware is installed: hold
    // View, Menu and A, then press Steam.)
    .usb_bootloader_code = { INPUT_CODE_LB, INPUT_CODE_START },

    .rgb_driver = {
        .pin_r      = PGPIO_LED_RED,
        .pin_g      = PGPIO_LED_GREEN,
        .pin_b      = PGPIO_LED_BLUE,
        .pin_w      = PGPIO_LED_WHITE,
        .active_low = true,
    },

    .rgb = {
        .groups = {
            { .name = "Status", .leds = RGB_GROUP(0) },
        },

        .notification_group_index = 0,
        .notification_group_size  = 1,
        .player_group_index       = -1,

        .key_mapping_count = 0,
    },

    // The sticks read the right way round as wired; the app may still flip them
    .analog_invert_allowed = 1,

    .inputs = {
        .slots = {
            { .code = INPUT_CODE_SOUTH,     .type = INPUT_TYPE_DIGITAL,  .name = "A" },
            { .code = INPUT_CODE_EAST,      .type = INPUT_TYPE_DIGITAL,  .name = "B" },
            { .code = INPUT_CODE_WEST,      .type = INPUT_TYPE_DIGITAL,  .name = "X" },
            { .code = INPUT_CODE_NORTH,     .type = INPUT_TYPE_DIGITAL,  .name = "Y" },
            { .code = INPUT_CODE_UP,        .type = INPUT_TYPE_DIGITAL,  .name = "D Up" },
            { .code = INPUT_CODE_DOWN,      .type = INPUT_TYPE_DIGITAL,  .name = "D Down" },
            { .code = INPUT_CODE_LEFT,      .type = INPUT_TYPE_DIGITAL,  .name = "D Left" },
            { .code = INPUT_CODE_RIGHT,     .type = INPUT_TYPE_DIGITAL,  .name = "D Right" },
            { .code = INPUT_CODE_LB,        .type = INPUT_TYPE_DIGITAL,  .name = "L1" },
            { .code = INPUT_CODE_RB,        .type = INPUT_TYPE_DIGITAL,  .name = "R1" },
            { .code = INPUT_CODE_LT_ANALOG, .type = INPUT_TYPE_HOVER,    .name = "L2" },
            { .code = INPUT_CODE_RT_ANALOG, .type = INPUT_TYPE_HOVER,    .name = "R2" },
            { .code = INPUT_CODE_LP1,       .type = INPUT_TYPE_DIGITAL,  .name = "L4" },
            { .code = INPUT_CODE_RP1,       .type = INPUT_TYPE_DIGITAL,  .name = "R4" },
            { .code = INPUT_CODE_LP2,       .type = INPUT_TYPE_DIGITAL,  .name = "L5" },
            { .code = INPUT_CODE_RP2,       .type = INPUT_TYPE_DIGITAL,  .name = "R5" },
            // Menu (three lines), View (two windows), Steam and Quick Access (three dots), by the
            // names of the config app's Steam glyphs. A name is 8 characters at most, which leaves
            // no room for the space in "S Options".
            { .code = INPUT_CODE_START,     .type = INPUT_TYPE_DIGITAL,  .name = "SOptions" },
            { .code = INPUT_CODE_SELECT,    .type = INPUT_TYPE_DIGITAL,  .name = "S View" },
            { .code = INPUT_CODE_HOME,      .type = INPUT_TYPE_DIGITAL,  .name = "S Guide" },
            { .code = INPUT_CODE_SHARE,     .type = INPUT_TYPE_DIGITAL,  .name = "S Select" },
            { .code = INPUT_CODE_LS,        .type = INPUT_TYPE_DIGITAL,  .name = "L3" },
            { .code = INPUT_CODE_LX_RIGHT,  .type = INPUT_TYPE_JOYSTICK, .name = "LX+" },
            { .code = INPUT_CODE_LX_LEFT,   .type = INPUT_TYPE_JOYSTICK, .name = "LX-" },
            { .code = INPUT_CODE_LY_UP,     .type = INPUT_TYPE_JOYSTICK, .name = "LY+" },
            { .code = INPUT_CODE_LY_DOWN,   .type = INPUT_TYPE_JOYSTICK, .name = "LY-" },
            { .code = INPUT_CODE_RS,        .type = INPUT_TYPE_DIGITAL,  .name = "R3" },
            { .code = INPUT_CODE_RX_RIGHT,  .type = INPUT_TYPE_JOYSTICK, .name = "RX+" },
            { .code = INPUT_CODE_RX_LEFT,   .type = INPUT_TYPE_JOYSTICK, .name = "RX-" },
            { .code = INPUT_CODE_RY_UP,     .type = INPUT_TYPE_JOYSTICK, .name = "RY+" },
            { .code = INPUT_CODE_RY_DOWN,   .type = INPUT_TYPE_JOYSTICK, .name = "RY-" },
        },
    },

    // Nintendo modes go by where a button sits, not by its letter: the bottom face button is B
    .defaults_switch = {
        .maps = {
            INPUT_DEFAULT(INPUT_CODE_SOUTH,     SWITCH_CODE_B),
            INPUT_DEFAULT(INPUT_CODE_EAST,      SWITCH_CODE_A),
            INPUT_DEFAULT(INPUT_CODE_WEST,      SWITCH_CODE_Y),
            INPUT_DEFAULT(INPUT_CODE_NORTH,     SWITCH_CODE_X),
            INPUT_DEFAULT(INPUT_CODE_UP,        SWITCH_CODE_UP),
            INPUT_DEFAULT(INPUT_CODE_DOWN,      SWITCH_CODE_DOWN),
            INPUT_DEFAULT(INPUT_CODE_LEFT,      SWITCH_CODE_LEFT),
            INPUT_DEFAULT(INPUT_CODE_RIGHT,     SWITCH_CODE_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_LB,        SWITCH_CODE_L),
            INPUT_DEFAULT(INPUT_CODE_RB,        SWITCH_CODE_R),
            INPUT_DEFAULT(INPUT_CODE_LT_ANALOG, SWITCH_CODE_ZL),
            INPUT_DEFAULT(INPUT_CODE_RT_ANALOG, SWITCH_CODE_ZR),
            INPUT_DEFAULT(INPUT_CODE_START,     SWITCH_CODE_PLUS),
            INPUT_DEFAULT(INPUT_CODE_SELECT,    SWITCH_CODE_MINUS),
            INPUT_DEFAULT(INPUT_CODE_HOME,      SWITCH_CODE_HOME),
            INPUT_DEFAULT(INPUT_CODE_SHARE,     SWITCH_CODE_CAPTURE),
            INPUT_DEFAULT(INPUT_CODE_LS,        SWITCH_CODE_LS),
            INPUT_DEFAULT(INPUT_CODE_LX_RIGHT,  SWITCH_CODE_LX_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_LX_LEFT,   SWITCH_CODE_LX_LEFT),
            INPUT_DEFAULT(INPUT_CODE_LY_UP,     SWITCH_CODE_LY_UP),
            INPUT_DEFAULT(INPUT_CODE_LY_DOWN,   SWITCH_CODE_LY_DOWN),
            INPUT_DEFAULT(INPUT_CODE_RS,        SWITCH_CODE_RS),
            INPUT_DEFAULT(INPUT_CODE_RX_RIGHT,  SWITCH_CODE_RX_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_RX_LEFT,   SWITCH_CODE_RX_LEFT),
            INPUT_DEFAULT(INPUT_CODE_RY_UP,     SWITCH_CODE_RY_UP),
            INPUT_DEFAULT(INPUT_CODE_RY_DOWN,   SWITCH_CODE_RY_DOWN),
            INPUT_DEFAULTS_END,
        },
    },

    .defaults_snes = {
        .maps = {
            INPUT_DEFAULT(INPUT_CODE_SOUTH,     SNES_CODE_B),
            INPUT_DEFAULT(INPUT_CODE_EAST,      SNES_CODE_A),
            INPUT_DEFAULT(INPUT_CODE_WEST,      SNES_CODE_Y),
            INPUT_DEFAULT(INPUT_CODE_NORTH,     SNES_CODE_X),
            INPUT_DEFAULT(INPUT_CODE_UP,        SNES_CODE_UP),
            INPUT_DEFAULT(INPUT_CODE_DOWN,      SNES_CODE_DOWN),
            INPUT_DEFAULT(INPUT_CODE_LEFT,      SNES_CODE_LEFT),
            INPUT_DEFAULT(INPUT_CODE_RIGHT,     SNES_CODE_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_LB,        SNES_CODE_L),
            INPUT_DEFAULT(INPUT_CODE_RB,        SNES_CODE_R),
            INPUT_DEFAULT(INPUT_CODE_START,     SNES_CODE_START),
            INPUT_DEFAULT(INPUT_CODE_SELECT,    SNES_CODE_SELECT),
            INPUT_DEFAULTS_END,
        },
    },

    .defaults_n64 = {
        .maps = {
            INPUT_DEFAULT(INPUT_CODE_SOUTH,     N64_CODE_A),
            INPUT_DEFAULT(INPUT_CODE_WEST,      N64_CODE_B),
            INPUT_DEFAULT(INPUT_CODE_UP,        N64_CODE_UP),
            INPUT_DEFAULT(INPUT_CODE_DOWN,      N64_CODE_DOWN),
            INPUT_DEFAULT(INPUT_CODE_LEFT,      N64_CODE_LEFT),
            INPUT_DEFAULT(INPUT_CODE_RIGHT,     N64_CODE_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_LB,        N64_CODE_L),
            INPUT_DEFAULT(INPUT_CODE_RB,        N64_CODE_R),
            INPUT_DEFAULT(INPUT_CODE_LT_ANALOG, N64_CODE_Z),
            INPUT_DEFAULT(INPUT_CODE_START,     N64_CODE_START),
            INPUT_DEFAULT(INPUT_CODE_LX_RIGHT,  N64_CODE_LX_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_LX_LEFT,   N64_CODE_LX_LEFT),
            INPUT_DEFAULT(INPUT_CODE_LY_UP,     N64_CODE_LY_UP),
            INPUT_DEFAULT(INPUT_CODE_LY_DOWN,   N64_CODE_LY_DOWN),
            INPUT_DEFAULT(INPUT_CODE_RX_RIGHT,  N64_CODE_CRIGHT),
            INPUT_DEFAULT(INPUT_CODE_RX_LEFT,   N64_CODE_CLEFT),
            INPUT_DEFAULT(INPUT_CODE_RY_UP,     N64_CODE_CUP),
            INPUT_DEFAULT(INPUT_CODE_RY_DOWN,   N64_CODE_CDOWN),
            INPUT_DEFAULTS_END,
        },
    },

    .defaults_gamecube = {
        .maps = {
            INPUT_DEFAULT(INPUT_CODE_SOUTH,     GAMECUBE_CODE_A),
            INPUT_DEFAULT(INPUT_CODE_WEST,      GAMECUBE_CODE_B),
            INPUT_DEFAULT(INPUT_CODE_EAST,      GAMECUBE_CODE_X),
            INPUT_DEFAULT(INPUT_CODE_NORTH,     GAMECUBE_CODE_Y),
            INPUT_DEFAULT(INPUT_CODE_UP,        GAMECUBE_CODE_UP),
            INPUT_DEFAULT(INPUT_CODE_DOWN,      GAMECUBE_CODE_DOWN),
            INPUT_DEFAULT(INPUT_CODE_LEFT,      GAMECUBE_CODE_LEFT),
            INPUT_DEFAULT(INPUT_CODE_RIGHT,     GAMECUBE_CODE_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_RB,        GAMECUBE_CODE_Z),
            INPUT_DEFAULT(INPUT_CODE_LT_ANALOG, GAMECUBE_CODE_L_ANALOG),
            INPUT_DEFAULT(INPUT_CODE_RT_ANALOG, GAMECUBE_CODE_R_ANALOG),
            INPUT_DEFAULT(INPUT_CODE_START,     GAMECUBE_CODE_START),
            INPUT_DEFAULT(INPUT_CODE_LX_RIGHT,  GAMECUBE_CODE_LX_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_LX_LEFT,   GAMECUBE_CODE_LX_LEFT),
            INPUT_DEFAULT(INPUT_CODE_LY_UP,     GAMECUBE_CODE_LY_UP),
            INPUT_DEFAULT(INPUT_CODE_LY_DOWN,   GAMECUBE_CODE_LY_DOWN),
            INPUT_DEFAULT(INPUT_CODE_RX_RIGHT,  GAMECUBE_CODE_RX_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_RX_LEFT,   GAMECUBE_CODE_RX_LEFT),
            INPUT_DEFAULT(INPUT_CODE_RY_UP,     GAMECUBE_CODE_RY_UP),
            INPUT_DEFAULT(INPUT_CODE_RY_DOWN,   GAMECUBE_CODE_RY_DOWN),
            INPUT_DEFAULTS_END,
        },
    },

    .defaults_xinput = {
        .maps = {
            INPUT_DEFAULT(INPUT_CODE_SOUTH,     XINPUT_CODE_A),
            INPUT_DEFAULT(INPUT_CODE_EAST,      XINPUT_CODE_B),
            INPUT_DEFAULT(INPUT_CODE_WEST,      XINPUT_CODE_X),
            INPUT_DEFAULT(INPUT_CODE_NORTH,     XINPUT_CODE_Y),
            INPUT_DEFAULT(INPUT_CODE_UP,        XINPUT_CODE_UP),
            INPUT_DEFAULT(INPUT_CODE_DOWN,      XINPUT_CODE_DOWN),
            INPUT_DEFAULT(INPUT_CODE_LEFT,      XINPUT_CODE_LEFT),
            INPUT_DEFAULT(INPUT_CODE_RIGHT,     XINPUT_CODE_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_LB,        XINPUT_CODE_LB),
            INPUT_DEFAULT(INPUT_CODE_RB,        XINPUT_CODE_RB),
            INPUT_DEFAULT(INPUT_CODE_LT_ANALOG, XINPUT_CODE_LT_ANALOG),
            INPUT_DEFAULT(INPUT_CODE_RT_ANALOG, XINPUT_CODE_RT_ANALOG),
            INPUT_DEFAULT(INPUT_CODE_START,     XINPUT_CODE_START),
            INPUT_DEFAULT(INPUT_CODE_SELECT,    XINPUT_CODE_BACK),
            INPUT_DEFAULT(INPUT_CODE_HOME,      XINPUT_CODE_GUIDE),
            INPUT_DEFAULT(INPUT_CODE_LS,        XINPUT_CODE_LS),
            INPUT_DEFAULT(INPUT_CODE_LX_RIGHT,  XINPUT_CODE_LX_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_LX_LEFT,   XINPUT_CODE_LX_LEFT),
            INPUT_DEFAULT(INPUT_CODE_LY_UP,     XINPUT_CODE_LY_UP),
            INPUT_DEFAULT(INPUT_CODE_LY_DOWN,   XINPUT_CODE_LY_DOWN),
            INPUT_DEFAULT(INPUT_CODE_RS,        XINPUT_CODE_RS),
            INPUT_DEFAULT(INPUT_CODE_RX_RIGHT,  XINPUT_CODE_RX_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_RX_LEFT,   XINPUT_CODE_RX_LEFT),
            INPUT_DEFAULT(INPUT_CODE_RY_UP,     XINPUT_CODE_RY_UP),
            INPUT_DEFAULT(INPUT_CODE_RY_DOWN,   XINPUT_CODE_RY_DOWN),
            INPUT_DEFAULTS_END,
        },
    },

    .defaults_sinput = {
        .maps = {
            INPUT_DEFAULT(INPUT_CODE_SOUTH,     SINPUT_CODE_SOUTH),
            INPUT_DEFAULT(INPUT_CODE_EAST,      SINPUT_CODE_EAST),
            INPUT_DEFAULT(INPUT_CODE_WEST,      SINPUT_CODE_WEST),
            INPUT_DEFAULT(INPUT_CODE_NORTH,     SINPUT_CODE_NORTH),
            INPUT_DEFAULT(INPUT_CODE_UP,        SINPUT_CODE_UP),
            INPUT_DEFAULT(INPUT_CODE_DOWN,      SINPUT_CODE_DOWN),
            INPUT_DEFAULT(INPUT_CODE_LEFT,      SINPUT_CODE_LEFT),
            INPUT_DEFAULT(INPUT_CODE_RIGHT,     SINPUT_CODE_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_LB,        SINPUT_CODE_LB),
            INPUT_DEFAULT(INPUT_CODE_RB,        SINPUT_CODE_RB),
            INPUT_DEFAULT(INPUT_CODE_LT_ANALOG, SINPUT_CODE_LT_ANALOG),
            INPUT_DEFAULT(INPUT_CODE_RT_ANALOG, SINPUT_CODE_RT_ANALOG),
            INPUT_DEFAULT(INPUT_CODE_LP1,       SINPUT_CODE_LP_1),
            INPUT_DEFAULT(INPUT_CODE_RP1,       SINPUT_CODE_RP_1),
            INPUT_DEFAULT(INPUT_CODE_LP2,       SINPUT_CODE_LP_2),
            INPUT_DEFAULT(INPUT_CODE_RP2,       SINPUT_CODE_RP_2),
            INPUT_DEFAULT(INPUT_CODE_START,     SINPUT_CODE_START),
            INPUT_DEFAULT(INPUT_CODE_SELECT,    SINPUT_CODE_SELECT),
            INPUT_DEFAULT(INPUT_CODE_HOME,      SINPUT_CODE_GUIDE),
            INPUT_DEFAULT(INPUT_CODE_SHARE,     SINPUT_CODE_SHARE),
            INPUT_DEFAULT(INPUT_CODE_LS,        SINPUT_CODE_LS),
            INPUT_DEFAULT(INPUT_CODE_LX_RIGHT,  SINPUT_CODE_LX_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_LX_LEFT,   SINPUT_CODE_LX_LEFT),
            INPUT_DEFAULT(INPUT_CODE_LY_UP,     SINPUT_CODE_LY_UP),
            INPUT_DEFAULT(INPUT_CODE_LY_DOWN,   SINPUT_CODE_LY_DOWN),
            INPUT_DEFAULT(INPUT_CODE_RS,        SINPUT_CODE_RS),
            INPUT_DEFAULT(INPUT_CODE_RX_RIGHT,  SINPUT_CODE_RX_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_RX_LEFT,   SINPUT_CODE_RX_LEFT),
            INPUT_DEFAULT(INPUT_CODE_RY_UP,     SINPUT_CODE_RY_UP),
            INPUT_DEFAULT(INPUT_CODE_RY_DOWN,   SINPUT_CODE_RY_DOWN),
            INPUT_DEFAULTS_END,
        },
    },
};

#if defined(STEAM_CONTROLLER_NS2_BLE_TEST)
// Bench: the Switch 2 mode's link (transport/ns2_ble.h) also runs beside the USB modes, where its
// log can be read: FLOSS's log and the link's counters go to the log read over the config interface
// (utilities/bench_log.h). The buttons are right in Switch mode over USB only, whose profile the
// link reads. Off the cable the mode runs as it does in any build.
#include <stdio.h>

#include "cores/cores.h"
#include "floss/floss.h"
#include "hal/nrf52/ble_controller.h"
#include "hal/nrf52/bluetooth.h"
#include "ns2_lib_gatt.h"
#include "transport/ns2_ble.h"
#include "transport/transport.h"
#include "transport/transport_bt.h"
#include "utilities/bench_log.h"
#include "utilities/boot.h"

#define BLE_TEST_ATT_NOTIFY     0x1Bu
#define BLE_TEST_ATT_WRITE_CMD  0x52u
#define BLE_TEST_EVT_COMPLETED_PACKETS 0x13u
// A console that lets go of the link and is back within this has not left (it does so while
// pairing, 14 s into one run): the restart back to USB waits for one that stays away
#define BLE_TEST_RETURN_AFTER_US 6000000u
#define BLE_TEST_LINE_US        2000000u
#define BLE_TEST_LINK_LINE_EVERY 5u

static bool _ble_test_running = false;

#if defined(STEAM_CONTROLLER_BLE_SNOOP)
// Every HCI packet but the input reports, the rumble writes and the controller's count of packets
// sent, which would bury the rest (each goes 200 times a second)
static void _ble_test_snoop(uint8_t type, bool is_rx, const uint8_t *pkt, uint16_t len, uint32_t t_us)
{
    // ACL header (4), L2CAP header (4), then the ATT opcode and the handle it is about
    if ((type == FLOSS_PKT_ACL) && !is_rx && (len > 10u) && (pkt[8] == BLE_TEST_ATT_NOTIFY) &&
        (pkt[9] == NS2_HANDLE_PRO2_INPUT_VALUE) && (pkt[10] == 0))
        return;
    if ((type == FLOSS_PKT_ACL) && is_rx && (len > 10u) && (pkt[8] == BLE_TEST_ATT_WRITE_CMD) &&
        (pkt[9] == NS2_HANDLE_VIBRATION_VALUE) && (pkt[10] == 0))
        return;
    if ((type == FLOSS_PKT_EVT) && is_rx && len && (pkt[0] == BLE_TEST_EVT_COMPLETED_PACKETS))
        return;
    floss_snoop_log_hook(type, is_rx, pkt, len, t_us);
}
#endif

static void _ble_test_start(void)
{
#if defined(STEAM_CONTROLLER_BLE_SNOOP)
    floss_snoop_set_hook(_ble_test_snoop);
#endif
    // A start that will be the Bluetooth mode (asked for by the restart before, or off the cable)
    // is left to start the link itself, as it does in any build
    boot_memory_s asked = {0};
    boot_get_memory(&asked);
    if ((asked.gamepad_method == GAMEPAD_METHOD_BLUETOOTH) || !nrf52_usb_power_present())
        return;
    // A log kept from the Bluetooth mode is waiting to be read: a link started now would bury it
    if (bench_log_waiting())
        return;

    _ble_test_running = nrf52_bluetooth_start(false);
    if (!_ble_test_running)
    {
        static const char failed[] = "BENCH Pro Controller 2 start failed\n";
        bench_log_write(failed, sizeof(failed) - 1u);
    }
}

static void _ble_test_task(void)
{
    const bool wireless = core_current_params()->transport_type == GAMEPAD_TRANSPORT_BLUETOOTH;

#if defined(STEAM_CONTROLLER_BLE_RETURN_S)
    // The Bluetooth mode has no USB to read its log over. It says where it stands and restarts,
    // which on the cable is back to USB with the log kept: a little after a console that had the
    // link lets go of it, or when none has taken it in the time given.
    static bool was_linked = false;
    static uint64_t let_go_us = 0;
    const uint64_t return_now = sys_hal_now_us();
    if (ns2_ble_linked())
    {
        was_linked = true;
        let_go_us = 0;
    }
    else if (was_linked && !let_go_us)
        let_go_us = return_now;

    const bool let_go = let_go_us && ((return_now - let_go_us) >= BLE_TEST_RETURN_AFTER_US);
    const bool none_came = !was_linked && (return_now >= (STEAM_CONTROLLER_BLE_RETURN_S * 1000000ull));
    if (wireless && (let_go || none_came))
    {
        char line[120];
        const int n = snprintf(line, sizeof(line), "BENCH back to USB: mode up %u, radio up %u, running %u, "
                                                   "linked %u, connection %u\n",
                               core_current_params()->transport_task != NULL, transport_bt_radio_up(),
                               ns2_ble_running(), ns2_ble_linked(), (unsigned)transport_current_connection());
        bench_log_write(line, (size_t)n);
        sys_hal_reboot();
    }
#endif

    // The wireless mode runs the link from its own task
    if (wireless)
        _ble_test_running = transport_bt_radio_up();
    if (!_ble_test_running)
        return;
    if (!wireless)
        nrf52_bluetooth_task();

    // How the main loop kept up (passes, and the longest wait between two) and the link layer's
    // counters, every tenth second
    static uint64_t next_us = 0;
    static uint64_t last_us = 0;
    static uint32_t passes = 0;
    static uint32_t longest_us = 0;
    static uint8_t turn = 0;
    const uint64_t now = sys_hal_now_us();
    if (last_us && ((now - last_us) > longest_us))
        longest_us = (uint32_t)(now - last_us);
    last_us = now;
    passes++;
    if (now < next_us)
        return;
    next_us = now + BLE_TEST_LINE_US;

    if (!(turn++ % BLE_TEST_LINK_LINE_EVERY))
    {
        char line[260];
        int n = snprintf(line, sizeof(line), "LOOP linked %u passes %lu longest %lu us | ", ns2_ble_linked(),
                         (unsigned long)passes, (unsigned long)longest_us);
        n += nrf52_ble_bench_stats(&line[n], sizeof(line) - (size_t)n);
        bench_log_write(line, (size_t)n);
    }
    passes = 0;
    longest_us = 0;
}
#endif

static void _local_output(uint32_t gpio, bool level)
{
    gpio_hal_write(gpio, level);
    gpio_hal_init(gpio, false, false);
    gpio_hal_write(gpio, level);
}

// Valve's bootloader stays in its update mode (a USB serial port) when it finds this in the last
// 16 bytes of RAM after a reset
#define BOOTLOADER_REQUEST_ADDRESS 0x2001FFF0u

void cb_nrf52_bootloader_request(void)
{
    static const uint32_t request[4] = {0xCD595B80u, 0x93541DA0u, 0x0078143Cu, 0u};
    volatile uint32_t *mailbox = (volatile uint32_t *)BOOTLOADER_REQUEST_ADDRESS;

    for (int i = 0; i < 4; i++)
        mailbox[i] = request[i];
}

// The pack's voltage as the fuel gauge driver wants it: 12 bits of half the pack against 3.3 V
uint16_t cb_hoja_read_battery()
{
    uint16_t counts = 0;
    nrf52_adc_read(&_battery_channel, 1, BATTERY_SAMPLES, &counts);

    // 64 bits: counts x millivolts x the divider passes 32
    const uint32_t pack_mv = (uint32_t)(((uint64_t)counts * BATTERY_ADC_FULL_MV * BATTERY_DIVIDER_TOP) /
                                        (4095u * BATTERY_DIVIDER_BOTTOM));
    const uint32_t scaled = (pack_mv * 4095u) / FUELGAUGE_FULL_MV;
    return (uint16_t)((scaled > 4095u) ? 4095u : scaled);
}

void cb_hoja_read_joystick(uint16_t *input)
{
    nrf52_adc_read(_stick_channels, 4, ANALOG_SAMPLES, input);
}

void cb_hoja_init()
{
    // Parts this firmware doesn't drive, held quiet. The trackpad controller shares the buttons'
    // I2C bus: it is reset and left running, never spoken to, which is the state the bus is known
    // to work in.
    _local_output(PGPIO_TRACKPAD_RESET, false);
    sys_hal_sleep_ms(TRACKPAD_RESET_MS);
    gpio_hal_write(PGPIO_TRACKPAD_RESET, true);
    _local_output(PGPIO_PAD_HAPTIC_ENABLE, false);
    _local_output(PGPIO_PAD_HAPTIC_LEFT, false);
    _local_output(PGPIO_PAD_HAPTIC_RIGHT, false);
    _local_output(PGPIO_HAPTIC_AMP_ENABLE, false);

    // The sticks, triggers and battery divider stay powered: the stock firmware switches them on
    // for each read to save the battery, which only matters off the cable
    _local_output(PGPIO_ANALOG_ENABLE, true);
    _local_output(PGPIO_BATTERY_SENSE_EN, true);

    gpio_hal_init(PGPIO_BTN_A, true, true);
    gpio_hal_init(PGPIO_BTN_B, true, true);
    gpio_hal_init(PGPIO_BTN_MENU, true, true);
    gpio_hal_init(PGPIO_BTN_VIEW, true, true);
    gpio_hal_init(PGPIO_BTN_STEAM, true, true);
    nrf52_gpio_init_input(PGPIO_BUTTONS_INT, false, false);

#if defined(STEAM_CONTROLLER_NS2_BLE_TEST)
    _ble_test_start();
#endif
}

void cb_hoja_shutdown()
{
    gpio_hal_write(PGPIO_ANALOG_ENABLE, false);
    gpio_hal_write(PGPIO_BATTERY_SENSE_EN, false);
    gpio_hal_write(PGPIO_HAPTIC_AMP_ENABLE, false);
}

// The buttons behind the GreenPAK: bit n low while button n is pressed. A read that fails gives
// every button released.
static uint16_t _local_read_buttons(void)
{
    const uint8_t command = BUTTONS_READ_COMMAND;
    uint8_t raw[2] = {0xFF, 0xFF};

    if (i2c_hal_write_read_timeout_us(BUTTONS_I2C_INSTANCE, BUTTONS_I2C_ADDRESS, &command, 1, raw, 2,
                                      BUTTONS_I2C_TIMEOUT_US) != 2)
        return BUTTONS_ALL_RELEASED;

    return (uint16_t)(raw[0] | (raw[1] << 8));
}

#define BUTTON_PRESSED(word, bit) (((word) & (1u << (bit))) == 0)

void cb_hoja_read_input(mapper_input_s *input)
{
    bool *out = input->presses;

#if defined(STEAM_CONTROLLER_NS2_BLE_TEST)
    _ble_test_task();
#endif

    out[INPUT_CODE_SOUTH]  = !gpio_hal_read(PGPIO_BTN_A);
    out[INPUT_CODE_EAST]   = !gpio_hal_read(PGPIO_BTN_B);
    out[INPUT_CODE_HOME]   = !gpio_hal_read(PGPIO_BTN_STEAM);
    out[INPUT_CODE_START]  = !gpio_hal_read(PGPIO_BTN_MENU);
    out[INPUT_CODE_SELECT] = !gpio_hal_read(PGPIO_BTN_VIEW);

    const uint16_t buttons = _local_read_buttons();
    out[INPUT_CODE_UP]    = BUTTON_PRESSED(buttons, BUTTONS_BIT_DPAD_UP);
    out[INPUT_CODE_DOWN]  = BUTTON_PRESSED(buttons, BUTTONS_BIT_DPAD_DOWN);
    out[INPUT_CODE_LEFT]  = BUTTON_PRESSED(buttons, BUTTONS_BIT_DPAD_LEFT);
    out[INPUT_CODE_RIGHT] = BUTTON_PRESSED(buttons, BUTTONS_BIT_DPAD_RIGHT);
    out[INPUT_CODE_SHARE] = BUTTON_PRESSED(buttons, BUTTONS_BIT_QUICK);
    out[INPUT_CODE_LP1]   = BUTTON_PRESSED(buttons, BUTTONS_BIT_L4);
    out[INPUT_CODE_LP2]   = BUTTON_PRESSED(buttons, BUTTONS_BIT_L5);
    out[INPUT_CODE_RP1]   = BUTTON_PRESSED(buttons, BUTTONS_BIT_R4);
    out[INPUT_CODE_RP2]   = BUTTON_PRESSED(buttons, BUTTONS_BIT_R5);
    out[INPUT_CODE_LB]    = BUTTON_PRESSED(buttons, BUTTONS_BIT_LB);
    out[INPUT_CODE_RB]    = BUTTON_PRESSED(buttons, BUTTONS_BIT_RB);
    out[INPUT_CODE_WEST]  = BUTTON_PRESSED(buttons, BUTTONS_BIT_X);
    out[INPUT_CODE_NORTH] = BUTTON_PRESSED(buttons, BUTTONS_BIT_Y);
    out[INPUT_CODE_LS]    = BUTTON_PRESSED(buttons, BUTTONS_BIT_LS);
    out[INPUT_CODE_RS]    = BUTTON_PRESSED(buttons, BUTTONS_BIT_RS);

    // A pull lowers the reading (board_config.h HOJA_HOVER_INVERTS turns it round)
    uint16_t triggers[2] = {0};
    nrf52_adc_read(_trigger_channels, 2, ANALOG_SAMPLES, triggers);
    input->inputs[INPUT_CODE_LT_ANALOG] = triggers[0];
    input->inputs[INPUT_CODE_RT_ANALOG] = triggers[1];
}

int main()
{
    hoja_init(&_hoja_config);
}
