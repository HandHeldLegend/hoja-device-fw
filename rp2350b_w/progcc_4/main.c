#include "hoja.h"
#include "board_config.h"
#include "main.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"

#include "hal/adc_hal.h"
#include "input/input_config.h"
#include "input_shared_types.h"

// Sticks and the hall sensors' shared outputs are on RP2350B's high-bank ADC pins (40-47), which
// only exist on the B package
adc_hal_driver_s stick_lx_driver = { .gpio = PGPIO_STICK_LX };
adc_hal_driver_s stick_ly_driver = { .gpio = PGPIO_STICK_LY };
adc_hal_driver_s stick_rx_driver = { .gpio = PGPIO_STICK_RX };
adc_hal_driver_s stick_ry_driver = { .gpio = PGPIO_STICK_RY };

adc_hal_driver_s hover_driver_0 = { .gpio = PGPIO_HOVER_ADC_0 };
adc_hal_driver_s hover_driver_1 = { .gpio = PGPIO_HOVER_ADC_1 };
adc_hal_driver_s hover_driver_2 = { .gpio = PGPIO_HOVER_ADC_2 };
adc_hal_driver_s hover_driver_3 = { .gpio = PGPIO_HOVER_ADC_3 };

static const hoja_config_s _hoja_config = {
    // I2C1 on the USB board's connector: BQ25180 charger and BQ27621 fuel gauge
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
            .clk_gpio  = 18,
            .mosi_gpio = 19,
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

    // Dual LSM6DSR on SPI instance 0, one CS each. Both sit on the back of the board flipped left to
    // right against the Latte Pro's front-mounted pair, which turns their X and Z around: the Latte
    // Pro inverts Y, so this board inverts all three (measured on the assembled prototype).
    .imu = {
        .channel_count = 2,
        .channel_a = {
            .bus = LSM6DSR_BUS_SPI, .spi_instance = 0, .cs_gpio = 17,
            .gyro  = { .invert_x = true, .invert_y = true, .invert_z = true },
            .accel = { .invert_x = true, .invert_y = true, .invert_z = true },
        },
        .channel_b = {
            .bus = LSM6DSR_BUS_SPI, .spi_instance = 0, .cs_gpio = 21,
            .gyro  = { .invert_x = true, .invert_y = true, .invert_z = true },
            .accel = { .invert_x = true, .invert_y = true, .invert_z = true },
        },
    },

    // LRA_L on GPIO 11, LRA_R on GPIO 31, each through a low-side NPN. Intensities start from the
    // Latte Pro's (the same drive).
    .haptics = {
        .channel_a_pin    = 11,
        .channel_b_pin    = 31,
        .channel_b_enable = true,
        .channel_swap     = false,
        .intensity_max    = 0.5f,
        .intensity_min_lo = 0.085f,
        .intensity_min_hi = 0.085f,
    },

    .battery_capacity_mah     = 1200,
    .battery_part_code        = "BDT 903035",
    .battery_shutdown_percent = 1, // TPS63001 buck-boost lets us run the cell to empty

    .device_name   = "ProGCC 4",
    .device_maker  = "HHL",
    .manifest_url  = "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/progcc_4/manifest.json",
    .firmware_url  = "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/progcc_4/progcc_4.uf2",
    .manual_url    = "https://docs.handheldlegend.com/s/portal/doc/user-guide-UoDtIku68z",
    .fcc_id        = NULL,

    .usb_vid = 0x2E8A, // Raspberry Pi
    .usb_pid = 0x10DF, // ProGCC PID

    .sinput = {
        .gamepad_type       = SINPUT_SDL_GAMEPAD_TYPE_NINTENDO_PRO,
        .face_buttons_style = SINPUT_SDL_GAMEPAD_FACE_STYLE_BAYX,
        .gamepad_subtype    = 1, // ProGCC 4
    },

    .sewn_layout = SEWN_LAYOUT_BAYX,

    .tourney_macro_code = INPUT_CODE_UNUSED,

    // MISC3 is the dedicated power button on this board.
    .shipping_macro_code = { INPUT_CODE_MISC3, INPUT_CODE_UNUSED },

    // Bluetooth pairing: hold Plus at boot. Boot combo for the USB bootloader: L + Plus.
    .sync_on_boot_code   = INPUT_CODE_START,
    .sync_macro_code     = { INPUT_CODE_UNUSED, INPUT_CODE_UNUSED },
    .usb_bootloader_code = { INPUT_CODE_LB, INPUT_CODE_START },

    // Joybus and NES share the USB-C port's level-shifted lines (USB board Q1/Q2)
    .joybus = { .data_pin = 4 },

    .nesbus = {
        .data_pin  = 4,
        .clock_pin = 6,
        .latch_pin = 5,
    },

    .rgb_driver = {
        .gpio_pin = 35,
    },

    // Chain order: North, East, South, West (a pair on each), the right stick board, the four player
    // LEDs left to right, D Down, D Right, D Up, D Left, then the left stick board.
    .rgb = {
        .groups = {
            /* 0 */ { .name = "L Stick", .leds = RGB_GROUP(16, 17, 18, 19) },
            /* 1 */ { .name = "R Stick", .leds = RGB_GROUP(4, 5, 6, 7) },
            /* 2 */ { .name = "South",   .leds = RGB_GROUP(2) },
            /* 3 */ { .name = "East",    .leds = RGB_GROUP(1) },
            /* 4 */ { .name = "West",    .leds = RGB_GROUP(3) },
            /* 5 */ { .name = "North",   .leds = RGB_GROUP(0) },
            /* 6 */ { .name = "D Down",  .leds = RGB_GROUP(12) },
            /* 7 */ { .name = "D Right", .leds = RGB_GROUP(13) },
            /* 8 */ { .name = "D Left",  .leds = RGB_GROUP(15) },
            /* 9 */ { .name = "D Up",    .leds = RGB_GROUP(14) },
            /*10 */ { .name = "Player",  .leds = RGB_GROUP(8, 9, 10, 11) },
        },

        // Every group is a real control, so no LED is spare for notifications
        .notification_group_index = -1,
        .notification_group_size  = 0,
        .player_group_index       = 10,

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
            { .code = INPUT_CODE_SOUTH,     .type = INPUT_TYPE_HOVER,    .name = "B" },
            { .code = INPUT_CODE_EAST,      .type = INPUT_TYPE_HOVER,    .name = "A" },
            { .code = INPUT_CODE_WEST,      .type = INPUT_TYPE_HOVER,    .name = "Y" },
            { .code = INPUT_CODE_NORTH,     .type = INPUT_TYPE_HOVER,    .name = "X" },
            { .code = INPUT_CODE_UP,        .type = INPUT_TYPE_HOVER,    .name = "D Up" },
            { .code = INPUT_CODE_DOWN,      .type = INPUT_TYPE_HOVER,    .name = "D Down" },
            { .code = INPUT_CODE_LEFT,      .type = INPUT_TYPE_HOVER,    .name = "D Left" },
            { .code = INPUT_CODE_RIGHT,     .type = INPUT_TYPE_HOVER,    .name = "D Right" },
            { .code = INPUT_CODE_LB,        .type = INPUT_TYPE_HOVER,    .name = "L" },
            { .code = INPUT_CODE_RB,        .type = INPUT_TYPE_HOVER,    .name = "R" },
            { .code = INPUT_CODE_LT_ANALOG, .type = INPUT_TYPE_HOVER,    .name = "ZL" },
            { .code = INPUT_CODE_RT_ANALOG, .type = INPUT_TYPE_HOVER,    .name = "ZR" },
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

static void _pgc4_setup_btn(uint32_t gpio)
{
    gpio_init(gpio);
    gpio_pull_up(gpio);
}

// An active-low sensor enable, idle high (bank off)
static void _pgc4_setup_enable(uint32_t gpio)
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

    _pgc4_setup_enable(PGPIO_EN_SEWN);
    _pgc4_setup_enable(PGPIO_EN_DPAD);
    _pgc4_setup_enable(PGPIO_EN_TRIGGER_L);
    _pgc4_setup_enable(PGPIO_EN_TRIGGER_R);

    _pgc4_setup_btn(PGPIO_BTN_POWER);
    _pgc4_setup_btn(PGPIO_BTN_GL);
    _pgc4_setup_btn(PGPIO_BTN_LS);
    _pgc4_setup_btn(PGPIO_BTN_CAPTURE);
    _pgc4_setup_btn(PGPIO_BTN_MINUS);
    _pgc4_setup_btn(PGPIO_BTN_PLUS);
    _pgc4_setup_btn(PGPIO_BTN_HOME);
    _pgc4_setup_btn(PGPIO_BTN_GR);
    _pgc4_setup_btn(PGPIO_BTN_RS);

    adc_hal_init(&stick_lx_driver);
    adc_hal_init(&stick_ly_driver);
    adc_hal_init(&stick_rx_driver);
    adc_hal_init(&stick_ry_driver);
    adc_hal_init(&hover_driver_0);
    adc_hal_init(&hover_driver_1);
    adc_hal_init(&hover_driver_2);
    adc_hal_init(&hover_driver_3);
}

// One bank of hall sensors on the four shared outputs: enable it, let the outputs settle, read
static void _pgc4_read_bank(uint32_t enable_gpio, uint32_t enable_gpio_2)
{
    gpio_put(enable_gpio, false);
    gpio_put(enable_gpio_2, false);
    sleep_us(BUTTON_SLEEP_US);
    adc_hal_read(&hover_driver_0);
    adc_hal_read(&hover_driver_1);
    adc_hal_read(&hover_driver_2);
    adc_hal_read(&hover_driver_3);
    gpio_put(enable_gpio, true);
    gpio_put(enable_gpio_2, true);
}

// Sensor to output from the schematic: U7 West, U3 South, U1 North, U6 East
static void _pgc4_scan_sewn(uint16_t *a_vals)
{
    _pgc4_read_bank(PGPIO_EN_SEWN, PGPIO_EN_SEWN);
    a_vals[INPUT_CODE_WEST]  = hover_driver_0.output;
    a_vals[INPUT_CODE_SOUTH] = hover_driver_1.output;
    a_vals[INPUT_CODE_NORTH] = hover_driver_2.output;
    a_vals[INPUT_CODE_EAST]  = hover_driver_3.output;
}

// Each trigger FPC has its own enable and two outputs (left: ADC_0/ADC_1 on J11, right: ADC_2/ADC_3
// on J7), so both sides read in one pass. Which output is the bumper was measured on the assembled
// prototype; the FPC netlist only shows which pairs share a function
static void _pgc4_scan_triggers(uint16_t *a_vals)
{
    _pgc4_read_bank(PGPIO_EN_TRIGGER_L, PGPIO_EN_TRIGGER_R);
    a_vals[INPUT_CODE_LT_ANALOG] = hover_driver_0.output;
    a_vals[INPUT_CODE_LB]        = hover_driver_1.output;
    a_vals[INPUT_CODE_RB]        = hover_driver_2.output;
    a_vals[INPUT_CODE_RT_ANALOG] = hover_driver_3.output;
}

// Sensor to output from the schematic: U11 Right, U9 Down, U8 Up, U10 Left
static void _pgc4_scan_dpad(uint16_t *a_vals)
{
    _pgc4_read_bank(PGPIO_EN_DPAD, PGPIO_EN_DPAD);
    a_vals[INPUT_CODE_RIGHT] = hover_driver_0.output;
    a_vals[INPUT_CODE_DOWN]  = hover_driver_1.output;
    a_vals[INPUT_CODE_UP]    = hover_driver_2.output;
    a_vals[INPUT_CODE_LEFT]  = hover_driver_3.output;
}

void cb_hoja_read_input(mapper_input_s *input)
{
    static uint8_t  read_idx = 0;
    static uint16_t a_vals[MAPPER_INPUT_COUNT] = {0};
    static bool     primed = false;

    bool *out = input->presses;

    out[INPUT_CODE_LS]     = !gpio_get(PGPIO_BTN_LS);
    out[INPUT_CODE_LP1]    = !gpio_get(PGPIO_BTN_GL);
    out[INPUT_CODE_SELECT] = !gpio_get(PGPIO_BTN_MINUS);
    out[INPUT_CODE_SHARE]  = !gpio_get(PGPIO_BTN_CAPTURE);
    out[INPUT_CODE_HOME]   = !gpio_get(PGPIO_BTN_HOME);
    out[INPUT_CODE_START]  = !gpio_get(PGPIO_BTN_PLUS);
    out[INPUT_CODE_RP1]    = !gpio_get(PGPIO_BTN_GR);
    out[INPUT_CODE_RS]     = !gpio_get(PGPIO_BTN_RS);
    out[INPUT_CODE_MISC3]  = !gpio_get(PGPIO_BTN_POWER);

    // hover_init() takes its boot snapshot from a single call, and boot.c reads the boot combos out
    // of it, so the first call reads every bank; after that one bank a call spreads the ADC cost
    if (!primed)
    {
        _pgc4_scan_sewn(a_vals);
        _pgc4_scan_triggers(a_vals);
        _pgc4_scan_dpad(a_vals);
        primed = true;
    }
    else
    {
        switch (read_idx)
        {
            default:
                _pgc4_scan_sewn(a_vals);
                break;

            case 1:
                _pgc4_scan_triggers(a_vals);
                break;

            case 2:
                _pgc4_scan_dpad(a_vals);
                break;
        }
    }

    for (int i = 0; i < MAPPER_INPUT_COUNT; i++)
        input->inputs[i] = a_vals[i];

    read_idx++;
    if (read_idx > 2)
        read_idx = 0;
}

void cb_hoja_read_joystick(uint16_t *input)
{
    adc_hal_read(&stick_lx_driver);
    adc_hal_read(&stick_ly_driver);
    adc_hal_read(&stick_rx_driver);
    adc_hal_read(&stick_ry_driver);

    // Measured on the assembled prototype: the left stick reads both axes backwards, and the right
    // stick's GPIO 46/47 carry its Y (backwards) and X, not X and Y
    input[0] = 0xFFF - stick_lx_driver.output;
    input[1] = 0xFFF - stick_ly_driver.output;
    input[2] = stick_ry_driver.output;
    input[3] = 0xFFF - stick_rx_driver.output;
}

int main()
{
    hoja_init(&_hoja_config);
}
