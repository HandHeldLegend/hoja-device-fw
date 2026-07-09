#include "hoja.h"
#include "board_config.h"
#include "main.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"

#include "hal/adc_hal.h"
#include "input/input_config.h"

adc_hal_driver_s joystick_driver_ly = { .gpio = 26 };
adc_hal_driver_s joystick_driver_lx = { .gpio = 27 };
adc_hal_driver_s joystick_driver_rx = { .gpio = 28 };
adc_hal_driver_s joystick_driver_ry = { .gpio = 29 };

static const hoja_config_s _hoja_config = {
    .i2c = {
        .instance_0 = { .enabled = false },
        .instance_1 = { .enabled = false },
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

    .imu = {
        .channel_count = 2,
        .channel_a = {
            .bus = LSM6DSR_BUS_SPI, .spi_instance = 0, .cs_gpio = 19,
            .gyro  = { .invert_x = true },
            .accel = { .invert_x = true },
        },
        .channel_b = {
            .bus = LSM6DSR_BUS_SPI, .spi_instance = 0, .cs_gpio = 21,
            .gyro  = { .invert_y = true },
            .accel = { .invert_y = true },
        },
    },

    .battery_capacity_mah     = 0,
    .battery_part_code        = NULL,
    .battery_shutdown_percent = 0,

    .device_name   = "ProGCC 3S",
    .device_maker  = "HHL",
    .manifest_url  = "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/progcc_3s/manifest.json",
    .firmware_url  = "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/progcc_3s/progcc_3s.uf2",
    .manual_url    = "https://docs.handheldlegend.com/s/portal/doc/user-guide-wQRaUfU6EM",
    .fcc_id        = NULL,

    .usb_vid = 0x2E8A,
    .usb_pid = 0x10DF,

    .sinput = {
        .gamepad_type       = (sinput_sdl_gamepad_type_t)7,
        .face_buttons_style = (sinput_sdl_face_style_t)3,
        .gamepad_subtype    = 0,
    },

    .sewn_layout = SEWN_LAYOUT_BAYX,
    .tourney_macro_code = INPUT_CODE_MISC3,

    .shipping_macro_code = { INPUT_CODE_RS, INPUT_CODE_LS },

    .sync_on_boot_code = INPUT_CODE_START,
    .sync_macro_code   = { INPUT_CODE_MISC3, INPUT_CODE_UNUSED },

    .usb_bootloader_code = { INPUT_CODE_LB, INPUT_CODE_START },
    .wlan_force_code     = INPUT_CODE_UNUSED,

    .joybus = { .data_pin = 0 },

    .nesbus = {
        .data_pin  = 0,
        .clock_pin = 1,
        .latch_pin = 2,
    },

    .inputs = {
        .slots = {
            { .code = INPUT_CODE_SOUTH,     .type = INPUT_TYPE_DIGITAL,  .name = "B" },
            { .code = INPUT_CODE_EAST,      .type = INPUT_TYPE_DIGITAL,  .name = "A" },
            { .code = INPUT_CODE_WEST,      .type = INPUT_TYPE_DIGITAL,  .name = "Y" },
            { .code = INPUT_CODE_NORTH,     .type = INPUT_TYPE_DIGITAL,  .name = "X" },
            { .code = INPUT_CODE_UP,        .type = INPUT_TYPE_DIGITAL,  .name = "D Up" },
            { .code = INPUT_CODE_DOWN,      .type = INPUT_TYPE_DIGITAL,  .name = "D Down" },
            { .code = INPUT_CODE_LEFT,      .type = INPUT_TYPE_DIGITAL,  .name = "D Left" },
            { .code = INPUT_CODE_RIGHT,     .type = INPUT_TYPE_DIGITAL,  .name = "D Right" },
            { .code = INPUT_CODE_LB,        .type = INPUT_TYPE_DIGITAL,  .name = "L" },
            { .code = INPUT_CODE_RB,        .type = INPUT_TYPE_DIGITAL,  .name = "R" },
            { .code = INPUT_CODE_LT,        .type = INPUT_TYPE_DIGITAL,  .name = "ZL" },
            { .code = INPUT_CODE_LT_ANALOG, .type = INPUT_TYPE_UNUSED,   .name = "" },
            { .code = INPUT_CODE_RT,        .type = INPUT_TYPE_DIGITAL,  .name = "ZR" },
            { .code = INPUT_CODE_RT_ANALOG, .type = INPUT_TYPE_UNUSED,   .name = "" },
            { .code = INPUT_CODE_START,     .type = INPUT_TYPE_DIGITAL,  .name = "Plus" },
            { .code = INPUT_CODE_SELECT,    .type = INPUT_TYPE_DIGITAL,  .name = "Minus" },
            { .code = INPUT_CODE_HOME,      .type = INPUT_TYPE_DIGITAL,  .name = "Home" },
            { .code = INPUT_CODE_SHARE,     .type = INPUT_TYPE_DIGITAL,  .name = "Capture" },
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
            INPUT_DEFAULT(INPUT_CODE_RT,        SWITCH_CODE_ZR),
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
            INPUT_DEFAULT(INPUT_CODE_LT,        SNES_CODE_L),
            INPUT_DEFAULT(INPUT_CODE_RT,        SNES_CODE_R),
            INPUT_DEFAULT(INPUT_CODE_START,     SNES_CODE_START),
            INPUT_DEFAULT(INPUT_CODE_SELECT,    SNES_CODE_SELECT),
            INPUT_DEFAULTS_END,
        },
    },

    .defaults_n64 = {
        .maps = {
            INPUT_DEFAULT(INPUT_CODE_SOUTH,     N64_CODE_B),
            INPUT_DEFAULT(INPUT_CODE_EAST,      N64_CODE_A),
            INPUT_DEFAULT(INPUT_CODE_WEST,      N64_CODE_CDOWN),
            INPUT_DEFAULT(INPUT_CODE_NORTH,     N64_CODE_CUP),
            INPUT_DEFAULT(INPUT_CODE_UP,        N64_CODE_UP),
            INPUT_DEFAULT(INPUT_CODE_DOWN,      N64_CODE_DOWN),
            INPUT_DEFAULT(INPUT_CODE_LEFT,      N64_CODE_LEFT),
            INPUT_DEFAULT(INPUT_CODE_RIGHT,     N64_CODE_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_LB,        N64_CODE_CLEFT),
            INPUT_DEFAULT(INPUT_CODE_RB,        N64_CODE_CRIGHT),
            INPUT_DEFAULT(INPUT_CODE_LT,        N64_CODE_Z),
            INPUT_DEFAULT(INPUT_CODE_RT,        N64_CODE_R),
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
            INPUT_DEFAULT(INPUT_CODE_RT,        GAMECUBE_CODE_R),
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
            INPUT_DEFAULT(INPUT_CODE_LT,        XINPUT_CODE_LT_ANALOG),
            INPUT_DEFAULT(INPUT_CODE_RT,        XINPUT_CODE_RT_ANALOG),
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
            INPUT_DEFAULT(INPUT_CODE_RT,        SINPUT_CODE_RT),
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

    .haptics = {
        .channel_a_pin    = 25,
        .channel_b_pin    = 14,
        .channel_b_enable = true,
        .channel_swap     = false,
        .intensity_max    = 0.35f,
        .intensity_min_lo = 0.15f,
        .intensity_min_hi = 0.10f,
    },

    .rgb_driver = {
        .gpio_pin = 24,
    },

    .rgb = {
        .groups = {
            { .name = "A",        .leds = RGB_GROUP(22) },
            { .name = "B",        .leds = RGB_GROUP(23) },
            { .name = "X",        .leds = RGB_GROUP(21) },
            { .name = "Y",        .leds = RGB_GROUP(20) },
            { .name = "D Up",     .leds = RGB_GROUP(15) },
            { .name = "D Down",   .leds = RGB_GROUP(13) },
            { .name = "D Left",   .leds = RGB_GROUP(12) },
            { .name = "D Right",  .leds = RGB_GROUP(14) },
            { .name = "L Stick",  .leds = RGB_GROUP(8, 9, 10, 11) },
            { .name = "R Stick",  .leds = RGB_GROUP(0, 1, 2, 3) },
            { .name = "L",        .leds = RGB_GROUP(26) },
            { .name = "R",        .leds = RGB_GROUP(25) },
            { .name = "ZL",       .leds = RGB_GROUP(27) },
            { .name = "ZR",       .leds = RGB_GROUP(24) },
            { .name = "Home",     .leds = RGB_GROUP(18) },
            { .name = "Capture",  .leds = RGB_GROUP(17) },
            { .name = "Plus",     .leds = RGB_GROUP(19) },
            { .name = "Minus",    .leds = RGB_GROUP(16) },
            { .name = "Player",   .leds = RGB_GROUP(7, 6, 5, 4) },
        },

        .notification_group_index = 14,
        .notification_group_size  = 1,
        .player_group_index       = 18,

        .key_mapping_count = 18,
        .key_mappings = {
            { INPUT_CODE_EAST,      0 },
            { INPUT_CODE_SOUTH,     1 },
            { INPUT_CODE_NORTH,     2 },
            { INPUT_CODE_WEST,      3 },
            { INPUT_CODE_UP,        4 },
            { INPUT_CODE_DOWN,      5 },
            { INPUT_CODE_LEFT,      6 },
            { INPUT_CODE_RIGHT,     7 },
            { INPUT_CODE_LX_RIGHT,  8 },
            { INPUT_CODE_RX_RIGHT,  9 },
            { INPUT_CODE_LB,        10 },
            { INPUT_CODE_RB,        11 },
            { INPUT_CODE_LT,        12 },
            { INPUT_CODE_RT,        13 },
            { INPUT_CODE_HOME,      14 },
            { INPUT_CODE_SHARE,     15 },
            { INPUT_CODE_START,     16 },
            { INPUT_CODE_SELECT,    17 },
        },
    },
};

void _local_setup_btn(uint32_t gpio)
{
    gpio_init(gpio);
    gpio_pull_up(gpio);
    gpio_set_dir(gpio, GPIO_IN);
}

void _local_setup_push(uint32_t gpio)
{
    gpio_init(gpio);
    gpio_pull_up(gpio);
    gpio_set_dir(gpio, GPIO_IN);
}

void _local_setup_scan(uint32_t gpio)
{
    gpio_init(gpio);
    gpio_pull_up(gpio);
    gpio_set_dir(gpio, GPIO_OUT);
    gpio_put(gpio, true);
}

void cb_hoja_read_joystick(uint16_t *input)
{
    adc_hal_read(&joystick_driver_lx);
    adc_hal_read(&joystick_driver_ly);
    adc_hal_read(&joystick_driver_rx);
    adc_hal_read(&joystick_driver_ry);

    input[0] = joystick_driver_lx.output;
    input[1] = joystick_driver_ly.output;
    input[2] = joystick_driver_rx.output;
    input[3] = joystick_driver_ry.output;
}

void cb_hoja_init()
{
    _local_setup_btn(PGPIO_BUTTON_RS);
    _local_setup_btn(PGPIO_BUTTON_LS);
    _local_setup_btn(PGPIO_BUTTON_MODE);

    _local_setup_push(PGPIO_PUSH_A);
    _local_setup_push(PGPIO_PUSH_B);
    _local_setup_push(PGPIO_PUSH_C);
    _local_setup_push(PGPIO_PUSH_D);

    _local_setup_scan(PGPIO_SCAN_A);
    _local_setup_scan(PGPIO_SCAN_B);
    _local_setup_scan(PGPIO_SCAN_C);
    _local_setup_scan(PGPIO_SCAN_D);

    adc_hal_init(&joystick_driver_ly);
    adc_hal_init(&joystick_driver_lx);
    adc_hal_init(&joystick_driver_ry);
    adc_hal_init(&joystick_driver_rx);
}

#define SCAN_TIME_US 25

void cb_hoja_read_input(mapper_input_s *input)
{
    bool *out = input->presses;

    gpio_put(PGPIO_SCAN_A, false);
    sleep_us(SCAN_TIME_US);
    out[INPUT_CODE_EAST]  = !gpio_get(PGPIO_PUSH_C);
    out[INPUT_CODE_SOUTH] = !gpio_get(PGPIO_PUSH_D);
    out[INPUT_CODE_NORTH] = !gpio_get(PGPIO_PUSH_A);
    out[INPUT_CODE_WEST]  = !gpio_get(PGPIO_PUSH_B);
    gpio_put(PGPIO_SCAN_A, true);

    gpio_put(PGPIO_SCAN_B, false);
    sleep_us(SCAN_TIME_US);
    out[INPUT_CODE_LEFT]  = !gpio_get(PGPIO_PUSH_D);
    out[INPUT_CODE_RIGHT] = !gpio_get(PGPIO_PUSH_C);
    out[INPUT_CODE_DOWN]  = !gpio_get(PGPIO_PUSH_B);
    out[INPUT_CODE_UP]    = !gpio_get(PGPIO_PUSH_A);
    gpio_put(PGPIO_SCAN_B, true);

    gpio_put(PGPIO_SCAN_C, false);
    sleep_us(SCAN_TIME_US);
    out[INPUT_CODE_START]  = !gpio_get(PGPIO_PUSH_A);
    out[INPUT_CODE_HOME]   = !gpio_get(PGPIO_PUSH_B);
    out[INPUT_CODE_SHARE]  = !gpio_get(PGPIO_PUSH_D);
    out[INPUT_CODE_SELECT] = !gpio_get(PGPIO_PUSH_C);
    gpio_put(PGPIO_SCAN_C, true);

    gpio_put(PGPIO_SCAN_D, false);
    sleep_us(SCAN_TIME_US);
    out[INPUT_CODE_RB] = !gpio_get(PGPIO_PUSH_B);
    out[INPUT_CODE_LB] = !gpio_get(PGPIO_PUSH_D);
    out[INPUT_CODE_LT] = !gpio_get(PGPIO_PUSH_A);
    out[INPUT_CODE_RT] = !gpio_get(PGPIO_PUSH_C);
    gpio_put(PGPIO_SCAN_D, true);

    out[INPUT_CODE_RS] = !gpio_get(PGPIO_BUTTON_RS);
    out[INPUT_CODE_LS] = !gpio_get(PGPIO_BUTTON_LS);
    out[INPUT_CODE_MISC3] = !gpio_get(PGPIO_BUTTON_MODE);
}

int main()
{
    hoja_init(&_hoja_config);
}
