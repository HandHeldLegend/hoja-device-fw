#include "hoja.h"
#include "board_config.h"
#include "main.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"

#include "hal/adc_hal.h"
#include "input/input_config.h"
#include "input_shared_types.h"

// Joysticks and the muxed hover inputs sit on RP2350B's high-bank ADC pins
// (40-47), which only exist on the B variant.
adc_hal_driver_s joystick_driver_1 = { .gpio = 44 };
adc_hal_driver_s joystick_driver_2 = { .gpio = 45 };
adc_hal_driver_s joystick_driver_3 = { .gpio = 46 };
adc_hal_driver_s joystick_driver_4 = { .gpio = 47 };

adc_hal_driver_s input_driver_1 = { .gpio = 40 };
adc_hal_driver_s input_driver_2 = { .gpio = 41 };
adc_hal_driver_s input_driver_3 = { .gpio = 42 };
adc_hal_driver_s input_driver_4 = { .gpio = 43 };

static const hoja_config_s _hoja_config = {
    .i2c = {
        .instance_0 = { .enabled = false },
        .instance_1 = {
            .enabled      = true,
            .sda_gpio     = 2,
            .scl_gpio     = 3,
            .baudrate_khz = 400,
        },
    },

    .spi = {
        .instance_0 = {
            .enabled   = true,
            .clk_gpio  = 22,
            .mosi_gpio = 23,
            .miso_gpio = 20,
        },
        .instance_1 = { .enabled = false },
    },

    // Battery PMIC config (type shaped by HOJA_BATTERY_DRIVER = BQ25180).
    .battery = {
        .i2c_instance   = 1,
        .charge_rate_ma = 100,
    },

    // Fuel gauge config (type shaped by HOJA_FUELGAUGE_DRIVER = BQ27621G1).
    .fuelgauge = {
        .i2c_instance = 1,
        .terminate_mv = 3000,
        .taper_ma     = 100,
    },

    // Dual LSM6DSR on SPI instance 0, one CS each. The old board_config
    // expressed this as HOJA_IMU_CHAN_x_INVERT_FLAGS 0b010010 on both channels;
    // the set bits are the "y" bit of each half, so it decodes to y-inverted on
    // both gyro and accel regardless of which half is which.
    .imu = {
        .channel_count = 2,
        .channel_a = {
            .bus = LSM6DSR_BUS_SPI, .spi_instance = 0, .cs_gpio = 15,
            .gyro  = { .invert_y = true },
            .accel = { .invert_y = true },
        },
        .channel_b = {
            .bus = LSM6DSR_BUS_SPI, .spi_instance = 0, .cs_gpio = 21,
            .gyro  = { .invert_y = true },
            .accel = { .invert_y = true },
        },
    },

    .haptics = {
        .channel_a_pin    = 12,
        .channel_b_pin    = 25,
        .channel_b_enable = true,
        .channel_swap     = false,
        .intensity_max    = 0.5f,
        .intensity_min_lo = 0.085f,
        .intensity_min_hi = 0.085f,
    },

    .battery_capacity_mah     = 1200,
    .battery_part_code        = "BDT 903035",
    .battery_shutdown_percent = 0,

    .device_name   = "Latte Pro",
    .device_maker  = "HHL",
    .manifest_url  = "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/latte_pro/manifest.json",
    .firmware_url  = "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/latte_pro/latte_pro.uf2",
    .manual_url    = "https://docs.handheldlegend.com/s/portal/doc/user-guide-UoDtIku68z",
    .fcc_id        = NULL,

    // Old board_config had VID/PID commented out, so stay on library defaults.
    .usb_vid = 0,
    .usb_pid = 0,

    .sinput = {
        .gamepad_type       = (sinput_sdl_gamepad_type_t)0, // Default
        .face_buttons_style = (sinput_sdl_face_style_t)3,   // Nintendo
        .gamepad_subtype    = 0,
    },

    .sewn_layout = SEWN_LAYOUT_BAYX,

    // Old board_config set HOJA_DISABLE_TOURNEY_MACRO.
    .tourney_macro_code = INPUT_CODE_UNUSED,

    // MISC3 is the dedicated power button on this board.
    .shipping_macro_code = { INPUT_CODE_MISC3, INPUT_CODE_UNUSED },

    // Old cb_hoja_boot set pairing_mode when Plus was held at boot, and
    // bootloader when Plus + L were held together.
    .sync_on_boot_code   = INPUT_CODE_START,
    .sync_macro_code     = { INPUT_CODE_UNUSED, INPUT_CODE_UNUSED },
    .usb_bootloader_code = { INPUT_CODE_LB, INPUT_CODE_START },
    .wlan_force_code     = INPUT_CODE_UNUSED,

    .joybus = { .data_pin = 4 },

    .nesbus = {
        .data_pin  = 4,
        .clock_pin = 6,
        .latch_pin = 5,
    },

    .rgb_driver = {
        .gpio_pin = 28,
    },

    .rgb = {
        .groups = {
            /* 0 */ { .name = "L Stick", .leds = RGB_GROUP(16, 17, 18, 19) },
            /* 1 */ { .name = "R Stick", .leds = RGB_GROUP(4, 5, 6, 7) },
            /* 2 */ { .name = "South",   .leds = RGB_GROUP(2) },
            /* 3 */ { .name = "East",    .leds = RGB_GROUP(1) },
            /* 4 */ { .name = "West",    .leds = RGB_GROUP(3) },
            /* 5 */ { .name = "North",   .leds = RGB_GROUP(0) },
            /* 6 */ { .name = "D Down",  .leds = RGB_GROUP(13) },
            /* 7 */ { .name = "D Right", .leds = RGB_GROUP(12) },
            /* 8 */ { .name = "D Left",  .leds = RGB_GROUP(14) },
            /* 9 */ { .name = "D Up",    .leds = RGB_GROUP(15) },
            /*10 */ { .name = "Player",  .leds = RGB_GROUP(8, 9, 10, 11) },
        },

        // Notifications are off (-1). Every group on this board is a real
        // control, so there is no spare LED to reserve. The old board_config
        // pointed this at group 5 ("North"), and Authentic/Chroma skips the
        // notification group's key_mapping lookup, which left North showing a
        // static user colour instead of its era-matched one.
        .notification_group_index = -1,
        .notification_group_size  = 0,
        .player_group_index       = 10,

        // Drives Authentic + React (reactive) lighting. The old board_config
        // carried these as .rgb_group fields on the input slots and several were
        // wrong: East pointed at the West group and vice versa, LX pointed at
        // the R Stick group, and the four dpad groups were never referenced at
        // all, so the dpad stayed dark under React. Corrected here.
        .key_mapping_count = 10,
        .key_mappings = {
            { INPUT_CODE_SOUTH,     2 },
            { INPUT_CODE_EAST,      3 },
            { INPUT_CODE_WEST,      4 },
            { INPUT_CODE_NORTH,     5 },
            { INPUT_CODE_DOWN,      6 },
            { INPUT_CODE_RIGHT,     7 },
            { INPUT_CODE_LEFT,      8 },
            { INPUT_CODE_UP,        9 },
            { INPUT_CODE_LX_RIGHT,  0 },
            { INPUT_CODE_RX_RIGHT,  1 },
        },
    },

    .inputs = {
        .slots = {
            { .code = INPUT_CODE_SOUTH,     .type = INPUT_TYPE_HOVER,    .name = "South" },
            { .code = INPUT_CODE_EAST,      .type = INPUT_TYPE_HOVER,    .name = "East" },
            { .code = INPUT_CODE_WEST,      .type = INPUT_TYPE_HOVER,    .name = "West" },
            { .code = INPUT_CODE_NORTH,     .type = INPUT_TYPE_HOVER,    .name = "North" },
            { .code = INPUT_CODE_UP,        .type = INPUT_TYPE_HOVER,    .name = "D Up" },
            { .code = INPUT_CODE_DOWN,      .type = INPUT_TYPE_HOVER,    .name = "D Down" },
            { .code = INPUT_CODE_LEFT,      .type = INPUT_TYPE_HOVER,    .name = "D Left" },
            { .code = INPUT_CODE_RIGHT,     .type = INPUT_TYPE_HOVER,    .name = "D Right" },
            { .code = INPUT_CODE_LB,        .type = INPUT_TYPE_HOVER,    .name = "L" },
            { .code = INPUT_CODE_RB,        .type = INPUT_TYPE_HOVER,    .name = "R" },
            { .code = INPUT_CODE_LT_ANALOG, .type = INPUT_TYPE_HOVER,    .name = "LT" },
            { .code = INPUT_CODE_RT_ANALOG, .type = INPUT_TYPE_HOVER,    .name = "RT" },
            { .code = INPUT_CODE_LP1,       .type = INPUT_TYPE_DIGITAL,  .name = "GL" },
            { .code = INPUT_CODE_RP1,       .type = INPUT_TYPE_DIGITAL,  .name = "GR" },
            { .code = INPUT_CODE_START,     .type = INPUT_TYPE_DIGITAL,  .name = "Plus" },
            { .code = INPUT_CODE_SELECT,    .type = INPUT_TYPE_DIGITAL,  .name = "Minus" },
            { .code = INPUT_CODE_HOME,      .type = INPUT_TYPE_DIGITAL,  .name = "Home" },
            { .code = INPUT_CODE_SHARE,     .type = INPUT_TYPE_DIGITAL,  .name = "Capture" },
            { .code = INPUT_CODE_MISC3,     .type = INPUT_TYPE_DIGITAL,  .name = "Power" },
            { .code = INPUT_CODE_LS,        .type = INPUT_TYPE_DIGITAL,  .name = "LS" },
            { .code = INPUT_CODE_LX_RIGHT,  .type = INPUT_TYPE_JOYSTICK, .name = "LX+" },
            { .code = INPUT_CODE_LX_LEFT,   .type = INPUT_TYPE_JOYSTICK, .name = "LX-" },
            { .code = INPUT_CODE_LY_UP,     .type = INPUT_TYPE_JOYSTICK, .name = "LY+" },
            { .code = INPUT_CODE_LY_DOWN,   .type = INPUT_TYPE_JOYSTICK, .name = "LY-" },
            { .code = INPUT_CODE_RS,        .type = INPUT_TYPE_DIGITAL,  .name = "RS" },
            { .code = INPUT_CODE_RX_RIGHT,  .type = INPUT_TYPE_JOYSTICK, .name = "RX+" },
            { .code = INPUT_CODE_RX_LEFT,   .type = INPUT_TYPE_JOYSTICK, .name = "RX-" },
            { .code = INPUT_CODE_RY_UP,     .type = INPUT_TYPE_JOYSTICK, .name = "RY+" },
            { .code = INPUT_CODE_RY_DOWN,   .type = INPUT_TYPE_JOYSTICK, .name = "RY-" },
        },
    },

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
            INPUT_DEFAULT(INPUT_CODE_LT,        SWITCH_CODE_ZL),
            INPUT_DEFAULT(INPUT_CODE_LT_ANALOG, SWITCH_CODE_ZL),
            INPUT_DEFAULT(INPUT_CODE_RT,        SWITCH_CODE_ZR),
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
            INPUT_DEFAULT(INPUT_CODE_EAST,      N64_CODE_CDOWN),
            INPUT_DEFAULT(INPUT_CODE_WEST,      N64_CODE_B),
            INPUT_DEFAULT(INPUT_CODE_NORTH,     N64_CODE_CUP),
            INPUT_DEFAULT(INPUT_CODE_UP,        N64_CODE_UP),
            INPUT_DEFAULT(INPUT_CODE_DOWN,      N64_CODE_DOWN),
            INPUT_DEFAULT(INPUT_CODE_LEFT,      N64_CODE_LEFT),
            INPUT_DEFAULT(INPUT_CODE_RIGHT,     N64_CODE_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_LB,        N64_CODE_CLEFT),
            INPUT_DEFAULT(INPUT_CODE_RB,        N64_CODE_CRIGHT),
            INPUT_DEFAULT(INPUT_CODE_LT,        N64_CODE_Z),
            INPUT_DEFAULT(INPUT_CODE_LT_ANALOG, N64_CODE_Z),
            INPUT_DEFAULT(INPUT_CODE_RT,        N64_CODE_R),
            INPUT_DEFAULT(INPUT_CODE_RT_ANALOG, N64_CODE_R),
            INPUT_DEFAULT(INPUT_CODE_START,     N64_CODE_START),
            INPUT_DEFAULT(INPUT_CODE_SELECT,    N64_CODE_L),
            INPUT_DEFAULT(INPUT_CODE_LX_RIGHT,  N64_CODE_LX_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_LX_LEFT,   N64_CODE_LX_LEFT),
            INPUT_DEFAULT(INPUT_CODE_LY_UP,     N64_CODE_LY_UP),
            INPUT_DEFAULT(INPUT_CODE_LY_DOWN,   N64_CODE_LY_DOWN),
            INPUT_DEFAULTS_END,
        },
    },

    .defaults_gamecube = {
        .maps = {
            INPUT_DEFAULT(INPUT_CODE_SOUTH,     GAMECUBE_CODE_B),
            INPUT_DEFAULT(INPUT_CODE_EAST,      GAMECUBE_CODE_A),
            INPUT_DEFAULT(INPUT_CODE_WEST,      GAMECUBE_CODE_Y),
            INPUT_DEFAULT(INPUT_CODE_NORTH,     GAMECUBE_CODE_X),
            INPUT_DEFAULT(INPUT_CODE_UP,        GAMECUBE_CODE_UP),
            INPUT_DEFAULT(INPUT_CODE_DOWN,      GAMECUBE_CODE_DOWN),
            INPUT_DEFAULT(INPUT_CODE_LEFT,      GAMECUBE_CODE_LEFT),
            INPUT_DEFAULT(INPUT_CODE_RIGHT,     GAMECUBE_CODE_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_RB,        GAMECUBE_CODE_Z),
            INPUT_DEFAULT(INPUT_CODE_LT,        GAMECUBE_CODE_L),
            INPUT_DEFAULT(INPUT_CODE_LT_ANALOG, GAMECUBE_CODE_L_ANALOG),
            INPUT_DEFAULT(INPUT_CODE_RT,        GAMECUBE_CODE_R),
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
            INPUT_DEFAULT(INPUT_CODE_SOUTH,     XINPUT_CODE_B),
            INPUT_DEFAULT(INPUT_CODE_EAST,      XINPUT_CODE_A),
            INPUT_DEFAULT(INPUT_CODE_WEST,      XINPUT_CODE_Y),
            INPUT_DEFAULT(INPUT_CODE_NORTH,     XINPUT_CODE_X),
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
            INPUT_DEFAULT(INPUT_CODE_LT,        SINPUT_CODE_LT),
            INPUT_DEFAULT(INPUT_CODE_LT_ANALOG, SINPUT_CODE_LT_ANALOG),
            INPUT_DEFAULT(INPUT_CODE_RT,        SINPUT_CODE_RT),
            INPUT_DEFAULT(INPUT_CODE_RT_ANALOG, SINPUT_CODE_RT_ANALOG),
            INPUT_DEFAULT(INPUT_CODE_START,     SINPUT_CODE_START),
            INPUT_DEFAULT(INPUT_CODE_SELECT,    SINPUT_CODE_SELECT),
            INPUT_DEFAULT(INPUT_CODE_HOME,      SINPUT_CODE_GUIDE),
            INPUT_DEFAULT(INPUT_CODE_SHARE,     SINPUT_CODE_SHARE),
            INPUT_DEFAULT(INPUT_CODE_MISC3,     SINPUT_CODE_MISC_3),
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

#define BUTTON_SLEEP_US 16

void _local_setup_btn(uint32_t gpio)
{
    gpio_init(gpio);
    gpio_pull_up(gpio);
}

void _local_setup_push(uint32_t gpio)
{
    gpio_init(gpio);              // Reset the pad to GPIO function
    gpio_put(gpio, true);         // Set the latch to HIGH first
    gpio_set_dir(gpio, GPIO_OUT); // THEN enable output
    gpio_disable_pulls(gpio);
    gpio_set_drive_strength(gpio, GPIO_DRIVE_STRENGTH_8MA);
}

void cb_hoja_init()
{
    stdio_init_all();

    _local_setup_push(PGPIO_EN_DPAD);
    _local_setup_push(PGPIO_EN_TRIGGER_L);
    _local_setup_push(PGPIO_EN_SEWN);
    _local_setup_push(PGPIO_EN_TRIGGER_R);

    // Set up GPIO for input buttons
    _local_setup_btn(PGPIO_BTN_POWER);
    _local_setup_btn(PGPIO_BTN_POWER2);
    _local_setup_btn(PGPIO_BTN_LS);
    _local_setup_btn(PGPIO_BTN_GL);
    _local_setup_btn(PGPIO_BTN_MINUS);
    _local_setup_btn(PGPIO_BTN_CAPTURE);
    _local_setup_btn(PGPIO_BTN_HOME);
    _local_setup_btn(PGPIO_BTN_PLUS);
    _local_setup_btn(PGPIO_BTN_GR);
    _local_setup_btn(PGPIO_BTN_RS);

    adc_hal_init(&joystick_driver_1);
    adc_hal_init(&joystick_driver_2);
    adc_hal_init(&joystick_driver_3);
    adc_hal_init(&joystick_driver_4);
    adc_hal_init(&input_driver_1);
    adc_hal_init(&input_driver_2);
    adc_hal_init(&input_driver_3);
    adc_hal_init(&input_driver_4);
}

// The four analog channels are shared across three latched banks. Each bank
// gets its own helper so the boot pass can sample all three at once.
static void _latte_scan_sewn(uint16_t *a_vals)
{
    gpio_put(PGPIO_EN_SEWN, false);
    sleep_us(BUTTON_SLEEP_US);
    adc_hal_read(&input_driver_1);
    adc_hal_read(&input_driver_2);
    adc_hal_read(&input_driver_3);
    adc_hal_read(&input_driver_4);

    a_vals[INPUT_CODE_NORTH] = input_driver_1.output;
    a_vals[INPUT_CODE_WEST]  = input_driver_2.output;
    a_vals[INPUT_CODE_EAST]  = input_driver_3.output;
    a_vals[INPUT_CODE_SOUTH] = input_driver_4.output;
    gpio_put(PGPIO_EN_SEWN, true);
}

static void _latte_scan_triggers(uint16_t *a_vals)
{
    gpio_put(PGPIO_EN_TRIGGER_L, false);
    gpio_put(PGPIO_EN_TRIGGER_R, false);
    sleep_us(BUTTON_SLEEP_US);
    adc_hal_read(&input_driver_1);
    adc_hal_read(&input_driver_2);
    adc_hal_read(&input_driver_3);
    adc_hal_read(&input_driver_4);

    a_vals[INPUT_CODE_RB]        = input_driver_1.output;
    a_vals[INPUT_CODE_RT_ANALOG] = input_driver_2.output;
    a_vals[INPUT_CODE_LB]        = input_driver_3.output;
    a_vals[INPUT_CODE_LT_ANALOG] = input_driver_4.output;
    gpio_put(PGPIO_EN_TRIGGER_L, true);
    gpio_put(PGPIO_EN_TRIGGER_R, true);
}

static void _latte_scan_dpad(uint16_t *a_vals)
{
    gpio_put(PGPIO_EN_DPAD, false);
    sleep_us(BUTTON_SLEEP_US);
    adc_hal_read(&input_driver_1);
    adc_hal_read(&input_driver_2);
    adc_hal_read(&input_driver_3);
    adc_hal_read(&input_driver_4);

    a_vals[INPUT_CODE_UP]    = input_driver_1.output;
    a_vals[INPUT_CODE_RIGHT] = input_driver_2.output;
    a_vals[INPUT_CODE_LEFT]  = input_driver_3.output;
    a_vals[INPUT_CODE_DOWN]  = input_driver_4.output;
    gpio_put(PGPIO_EN_DPAD, true);
}

void cb_hoja_read_input(mapper_input_s *input)
{
    static uint8_t  read_idx = 0;
    static uint16_t a_vals[MAPPER_INPUT_COUNT] = {0};


    bool *out = input->presses;

    out[INPUT_CODE_LS]     = !gpio_get(PGPIO_BTN_LS);
    out[INPUT_CODE_LP1]    = !gpio_get(PGPIO_BTN_GL);
    out[INPUT_CODE_SELECT] = !gpio_get(PGPIO_BTN_MINUS);
    out[INPUT_CODE_SHARE]  = !gpio_get(PGPIO_BTN_CAPTURE);
    out[INPUT_CODE_HOME]   = !gpio_get(PGPIO_BTN_HOME);

    out[INPUT_CODE_START] = !gpio_get(PGPIO_BTN_PLUS);
    out[INPUT_CODE_RP1]   = !gpio_get(PGPIO_BTN_GR);
    out[INPUT_CODE_RS]    = !gpio_get(PGPIO_BTN_RS);

    // hover_init() takes the library's boot snapshot from a SINGLE call to
    // this function, so a purely rotating scan would leave two of the three
    // banks at their initialiser value on that call. boot.c resolves the
    // d-pad boot modes out of that snapshot, so prime every bank on the
    // first call, then rotate to spread the ADC cost as before.
    static bool primed = false;
    if (!primed)
    {
        _latte_scan_sewn(a_vals);
        _latte_scan_triggers(a_vals);
        _latte_scan_dpad(a_vals);
        primed = true;
    }
    else
    {
        switch (read_idx)
        {
            default: _latte_scan_sewn(a_vals);     break;
            case 1:  _latte_scan_triggers(a_vals); break;
            case 2:  _latte_scan_dpad(a_vals);     break;
        }
    }


    for (int i = 0; i < MAPPER_INPUT_COUNT; i++)
    {
        input->inputs[i] = a_vals[i];
    }

    read_idx++;
    if (read_idx > 2) read_idx = 0;

    // mapper_input_s no longer carries button_shipping / button_sync. The
    // library derives both from the codes in hoja_config_s
    // (.shipping_macro_code = MISC3, .sync_on_boot_code = START), so this just
    // publishes the raw button states; START is already read above.
    out[INPUT_CODE_MISC3] = !gpio_get(PGPIO_BTN_POWER);
}

void cb_hoja_read_joystick(uint16_t *input)
{
    adc_hal_read(&joystick_driver_1);
    adc_hal_read(&joystick_driver_2);
    adc_hal_read(&joystick_driver_3);
    adc_hal_read(&joystick_driver_4);

    input[2] = 0xFFF - joystick_driver_1.output;
    input[3] = joystick_driver_2.output;
    input[0] = 0xFFF - joystick_driver_3.output;
    input[1] = 0xFFF - joystick_driver_4.output;
}

int main()
{
    hoja_init(&_hoja_config);
}
