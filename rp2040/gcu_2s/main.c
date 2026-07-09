#include "hoja.h"
#include "board_config.h"
#include "main.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"

#include "hal/adc_hal.h"
#include "drivers/mux/tmux1204.h"
#include "utilities/settings.h"
#include "input/input_config.h"

#define GCU_INPUT_CODE_A    INPUT_CODE_SOUTH
#define GCU_INPUT_CODE_X    INPUT_CODE_EAST
#define GCU_INPUT_CODE_B    INPUT_CODE_WEST
#define GCU_INPUT_CODE_Y    INPUT_CODE_NORTH
#define GCU_INPUT_CODE_PWR  INPUT_CODE_MISC3

mux_tmux1204_driver_s mux_driver = {
    .a0_gpio = 23,
    .a1_gpio = 24,
};

static const hoja_config_s _hoja_config = {
    .spi = {
        .instance_0 = {
            .enabled      = true,
            .clk_gpio     = 6,
            .mosi_gpio    = 7,
            .miso_gpio    = 4,
        },
        .instance_1 = {
            .enabled = false,
        },
    },

    // IMU config (type shaped by HOJA_IMU_DRIVER = LSM6DSR). Dual sensors on SPI0.
    .imu = {
        .channel_count = 2,
        .channel_a = {
            .bus = LSM6DSR_BUS_SPI, .spi_instance = 0, .cs_gpio = 0,
            .gyro  = { .invert_y = true },
            .accel = { .invert_y = true },
        },
        .channel_b = {
            .bus = LSM6DSR_BUS_SPI, .spi_instance = 0, .cs_gpio = 2,
            .gyro  = { .invert_y = true },
            .accel = { .invert_y = true },
        },
    },

    .battery_capacity_mah     = 0,
    .battery_part_code        = NULL,
    .battery_shutdown_percent = 0,

    // Device identity + app-facing URLs.
    .device_name   = "GCU-2S",
    .device_maker  = "HHL",
    .manifest_url  = "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/gcu_2s/manifest.json",
    .firmware_url  = "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/gcu_2s/gcu_2s.uf2",
    .manual_url    = "https://docs.handheldlegend.com/s/portal/doc/user-guide-UoDtIku68z",
    .fcc_id        = "N/A",

    .usb_vid = 0x2E8A,
    .usb_pid = 0x10DD,

    .sinput = {
        .gamepad_type       = SINPUT_SDL_GAMEPAD_TYPE_GAMECUBE,
        .face_buttons_style = SINPUT_SDL_GAMEPAD_FACE_STYLE_AXBY,
        .gamepad_subtype    = 0,
    },

    .sewn_layout = SEWN_LAYOUT_AXBY,
    .tourney_macro_code = INPUT_CODE_UNUSED,

    // Power button hold → ship mode.
    .shipping_macro_code = { GCU_INPUT_CODE_PWR, INPUT_CODE_UNUSED },

    // Bluetooth pairing: hold Plus at boot (no BT on this board; kept for profile parity).
    .sync_on_boot_code = INPUT_CODE_START,
    .sync_macro_code   = { INPUT_CODE_UNUSED, INPUT_CODE_UNUSED },

    // Boot combos.
    .usb_bootloader_code = { INPUT_CODE_LB, INPUT_CODE_START },
    .wlan_force_code     = INPUT_CODE_UNUSED,

    // Shared N64/GameCube joybus line (data on GPIO20).
    .joybus = {
        .data_pin = 20,
    },

    .nesbus = {
        .data_pin  = 20,
        .clock_pin = 21,
        .latch_pin = 19,
    },

    // Physical inputs: explicit code + scan type + config-tool label.
    .inputs = {
        .slots = {
            { .code = GCU_INPUT_CODE_A,     .type = INPUT_TYPE_DIGITAL,  .name = "A" },
            { .code = GCU_INPUT_CODE_X,     .type = INPUT_TYPE_DIGITAL,  .name = "X" },
            { .code = GCU_INPUT_CODE_B,     .type = INPUT_TYPE_DIGITAL,  .name = "B" },
            { .code = GCU_INPUT_CODE_Y,     .type = INPUT_TYPE_DIGITAL,  .name = "Y" },
            { .code = INPUT_CODE_UP,        .type = INPUT_TYPE_DIGITAL,  .name = "D Up" },
            { .code = INPUT_CODE_DOWN,      .type = INPUT_TYPE_DIGITAL,  .name = "D Down" },
            { .code = INPUT_CODE_LEFT,      .type = INPUT_TYPE_DIGITAL,  .name = "D Left" },
            { .code = INPUT_CODE_RIGHT,     .type = INPUT_TYPE_DIGITAL,  .name = "D Right" },
            { .code = INPUT_CODE_LB,        .type = INPUT_TYPE_DIGITAL,  .name = "ZL" },
            { .code = INPUT_CODE_RB,        .type = INPUT_TYPE_DIGITAL,  .name = "ZR" },
            { .code = INPUT_CODE_LT,        .type = INPUT_TYPE_DIGITAL,  .name = "L" },
            { .code = INPUT_CODE_LT_ANALOG, .type = INPUT_TYPE_HOVER,    .name = "LT" },
            { .code = INPUT_CODE_RT,        .type = INPUT_TYPE_DIGITAL,  .name = "R" },
            { .code = INPUT_CODE_RT_ANALOG, .type = INPUT_TYPE_HOVER,    .name = "RT" },
            { .code = INPUT_CODE_START,     .type = INPUT_TYPE_DIGITAL,  .name = "Plus" },
            { .code = INPUT_CODE_SELECT,    .type = INPUT_TYPE_DIGITAL,  .name = "Minus" },
            { .code = INPUT_CODE_HOME,      .type = INPUT_TYPE_DIGITAL,  .name = "Home" },
            { .code = INPUT_CODE_SHARE,     .type = INPUT_TYPE_DIGITAL,  .name = "Capture" },
            { .code = GCU_INPUT_CODE_PWR,   .type = INPUT_TYPE_DIGITAL,  .name = "Power" },
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
            INPUT_DEFAULT(GCU_INPUT_CODE_A,     SWITCH_CODE_A),
            INPUT_DEFAULT(GCU_INPUT_CODE_X,     SWITCH_CODE_X),
            INPUT_DEFAULT(GCU_INPUT_CODE_B,     SWITCH_CODE_B),
            INPUT_DEFAULT(GCU_INPUT_CODE_Y,     SWITCH_CODE_Y),
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
            INPUT_DEFAULT(GCU_INPUT_CODE_A,         SNES_CODE_A),
            INPUT_DEFAULT(GCU_INPUT_CODE_X,         SNES_CODE_X),
            INPUT_DEFAULT(GCU_INPUT_CODE_B,         SNES_CODE_B),
            INPUT_DEFAULT(GCU_INPUT_CODE_Y,         SNES_CODE_Y),
            INPUT_DEFAULT(INPUT_CODE_UP,            SNES_CODE_UP),
            INPUT_DEFAULT(INPUT_CODE_DOWN,          SNES_CODE_DOWN),
            INPUT_DEFAULT(INPUT_CODE_LEFT,          SNES_CODE_LEFT),
            INPUT_DEFAULT(INPUT_CODE_RIGHT,         SNES_CODE_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_LT,            SNES_CODE_L),
            INPUT_DEFAULT(INPUT_CODE_RT,            SNES_CODE_R),
            INPUT_DEFAULT(INPUT_CODE_LT_ANALOG,     SNES_CODE_L),
            INPUT_DEFAULT(INPUT_CODE_RT_ANALOG,     SNES_CODE_R),
            INPUT_DEFAULT(INPUT_CODE_START,         SNES_CODE_START),
            INPUT_DEFAULT(INPUT_CODE_SELECT,        SNES_CODE_SELECT),
            INPUT_DEFAULTS_END,
        },
    },

    .defaults_n64 = {
        .maps = {
            INPUT_DEFAULT(GCU_INPUT_CODE_A,     N64_CODE_A),
            INPUT_DEFAULT(GCU_INPUT_CODE_X,     N64_CODE_CDOWN),
            INPUT_DEFAULT(GCU_INPUT_CODE_B,     N64_CODE_B),
            INPUT_DEFAULT(GCU_INPUT_CODE_Y,     N64_CODE_CUP),
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
            INPUT_DEFAULT(GCU_INPUT_CODE_A,     GAMECUBE_CODE_A),
            INPUT_DEFAULT(GCU_INPUT_CODE_X,     GAMECUBE_CODE_X),
            INPUT_DEFAULT(GCU_INPUT_CODE_B,     GAMECUBE_CODE_B),
            INPUT_DEFAULT(GCU_INPUT_CODE_Y,     GAMECUBE_CODE_Y),
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
            INPUT_DEFAULT(GCU_INPUT_CODE_A,     XINPUT_CODE_A),
            INPUT_DEFAULT(GCU_INPUT_CODE_X,     XINPUT_CODE_X),
            INPUT_DEFAULT(GCU_INPUT_CODE_B,     XINPUT_CODE_B),
            INPUT_DEFAULT(GCU_INPUT_CODE_Y,     XINPUT_CODE_Y),
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
            INPUT_DEFAULT(GCU_INPUT_CODE_A,     SINPUT_CODE_SOUTH),
            INPUT_DEFAULT(GCU_INPUT_CODE_X,     SINPUT_CODE_EAST),
            INPUT_DEFAULT(GCU_INPUT_CODE_B,     SINPUT_CODE_WEST),
            INPUT_DEFAULT(GCU_INPUT_CODE_Y,     SINPUT_CODE_NORTH),
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
            INPUT_DEFAULT(GCU_INPUT_CODE_PWR,   SINPUT_CODE_MISC_3),
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
        .channel_a_pin    = 8,
        .channel_b_pin    = 3,
        .channel_b_enable = true,
        .channel_swap     = true,
        .intensity_max    = 0.35f,
        .intensity_min_lo = 0.15f,
        .intensity_min_hi = 0.10f,
    },

    .rgb_driver = {
        .gpio_pin = 27,
    },

    .rgb = {
        .groups = {
            { .name = "L Stick",  .leds = RGB_GROUP(0, 1, 2, 3) },
            { .name = "R Stick",  .leds = RGB_GROUP(13, 14, 15, 16) },
            { .name = "A",        .leds = RGB_GROUP(11) },
            { .name = "B",        .leds = RGB_GROUP(9) },
            { .name = "X",        .leds = RGB_GROUP(12) },
            { .name = "Y",        .leds = RGB_GROUP(10) },
            { .name = "D Up",     .leds = RGB_GROUP(7) },
            { .name = "D Down",   .leds = RGB_GROUP(5) },
            { .name = "D Left",   .leds = RGB_GROUP(4) },
            { .name = "D Right",  .leds = RGB_GROUP(6) },
            { .name = "Power",    .leds = RGB_GROUP(8) },
        },

        .notification_group_index = 10,
        .notification_group_size  = 1,
        .player_group_index = -1,

        .key_mapping_count = 11,
        .key_mappings = {
            { INPUT_CODE_LX_RIGHT, 0 },
            { INPUT_CODE_RX_RIGHT, 1 },
            { GCU_INPUT_CODE_A,    2 },
            { GCU_INPUT_CODE_B,    3 },
            { GCU_INPUT_CODE_X,    4 },
            { GCU_INPUT_CODE_Y,    5 },
            { INPUT_CODE_UP,       6 },
            { INPUT_CODE_DOWN,     7 },
            { INPUT_CODE_LEFT,     8 },
            { INPUT_CODE_RIGHT,    9 },
            { GCU_INPUT_CODE_PWR,  10 },
        },
    },
};

adc_hal_driver_s joystick_driver = {
    .gpio = 26,
};

adc_hal_driver_s trigger_driver_l = {
    .gpio = 29,
};

adc_hal_driver_s trigger_driver_r = {
    .gpio = 28,
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

void cb_hoja_init()
{
    stdio_init_all();

    _local_setup_btn(PGPIO_BUTTON_MODE);
    _local_setup_scan(PGPIO_SCAN_A);
    _local_setup_scan(PGPIO_SCAN_B);
    _local_setup_scan(PGPIO_SCAN_C);
    _local_setup_scan(PGPIO_SCAN_D);
    _local_setup_scan(PGPIO_SCAN_E);
    _local_setup_scan(PGPIO_SCAN_F);

    _local_setup_push(PGPIO_PUSH_G);
    _local_setup_push(PGPIO_PUSH_H);
    _local_setup_push(PGPIO_PUSH_I);

    tmux1204_init(&mux_driver);
    adc_hal_init(&trigger_driver_l);
    adc_hal_init(&trigger_driver_r);
    adc_hal_init(&joystick_driver);
}

#define BUTTON_SLEEP_US 15

void cb_hoja_read_input(mapper_input_s *input)
{
    adc_hal_read(&trigger_driver_l);
    adc_hal_read(&trigger_driver_r);

    bool *out = input->presses;

    gpio_put(PGPIO_SCAN_A, false);
    sleep_us(BUTTON_SLEEP_US);
    out[INPUT_CODE_UP] = !gpio_get(PGPIO_PUSH_H);
    out[INPUT_CODE_RS] = !gpio_get(PGPIO_PUSH_G);
    out[INPUT_CODE_START] = !gpio_get(PGPIO_PUSH_I);
    gpio_put(PGPIO_SCAN_A, true);

    gpio_put(PGPIO_SCAN_B, false);
    sleep_us(BUTTON_SLEEP_US);
    out[INPUT_CODE_DOWN] = !gpio_get(PGPIO_PUSH_H);
    out[GCU_INPUT_CODE_A] = !gpio_get(PGPIO_PUSH_G);
    out[INPUT_CODE_SELECT] = !gpio_get(PGPIO_PUSH_I);
    gpio_put(PGPIO_SCAN_B, true);

    gpio_put(PGPIO_SCAN_C, false);
    sleep_us(BUTTON_SLEEP_US);
    out[INPUT_CODE_LEFT] = !gpio_get(PGPIO_PUSH_H);
    out[GCU_INPUT_CODE_X] = !gpio_get(PGPIO_PUSH_G);
    out[INPUT_CODE_HOME] = !gpio_get(PGPIO_PUSH_I);
    gpio_put(PGPIO_SCAN_C, true);

    gpio_put(PGPIO_SCAN_D, false);
    sleep_us(BUTTON_SLEEP_US);
    out[INPUT_CODE_LB] = !gpio_get(PGPIO_PUSH_H);
    out[GCU_INPUT_CODE_Y] = !gpio_get(PGPIO_PUSH_G);
    out[INPUT_CODE_SHARE] = !gpio_get(PGPIO_PUSH_I);
    gpio_put(PGPIO_SCAN_D, true);

    gpio_put(PGPIO_SCAN_E, false);
    sleep_us(BUTTON_SLEEP_US);
    out[INPUT_CODE_LS] = !gpio_get(PGPIO_PUSH_H);
    out[INPUT_CODE_RB] = !gpio_get(PGPIO_PUSH_G);
    out[INPUT_CODE_RIGHT] = !gpio_get(PGPIO_PUSH_I);
    gpio_put(PGPIO_SCAN_E, true);

    gpio_put(PGPIO_SCAN_F, false);
    sleep_us(BUTTON_SLEEP_US);
    out[INPUT_CODE_LT] = !gpio_get(PGPIO_PUSH_H);
    out[INPUT_CODE_RT] = !gpio_get(PGPIO_PUSH_G);
    out[GCU_INPUT_CODE_B] = !gpio_get(PGPIO_PUSH_I);
    gpio_put(PGPIO_SCAN_F, true);

    out[GCU_INPUT_CODE_PWR] = !gpio_get(PGPIO_BUTTON_MODE);

    input->inputs[INPUT_CODE_LT_ANALOG] = 0xFFF - trigger_driver_l.output;
    input->inputs[INPUT_CODE_RT_ANALOG] = 0xFFF - trigger_driver_r.output;

    const uint16_t threshold = 500;

    if(input->inputs[INPUT_CODE_LT_ANALOG] > (hover_config->config[INPUT_CODE_LT_ANALOG].max + threshold))
        out[INPUT_CODE_LT] |= 1;
    else
        out[INPUT_CODE_LT] |= 0;

    if(input->inputs[INPUT_CODE_RT_ANALOG] > (hover_config->config[INPUT_CODE_RT_ANALOG].max + threshold))
        out[INPUT_CODE_RT] |= 1;
    else
        out[INPUT_CODE_RT] |= 0;
}

void cb_hoja_read_joystick(uint16_t *input)
{
    tmux1204_switch_channel(&mux_driver, 0);
    adc_hal_read(&joystick_driver);
    input[0] = joystick_driver.output;

    tmux1204_switch_channel(&mux_driver, 2);
    adc_hal_read(&joystick_driver);
    input[1] = joystick_driver.output;

    tmux1204_switch_channel(&mux_driver, 3);
    adc_hal_read(&joystick_driver);
    input[2] = joystick_driver.output;

    tmux1204_switch_channel(&mux_driver, 1);
    adc_hal_read(&joystick_driver);
    input[3] = joystick_driver.output;
}

int main()
{
    hoja_init(&_hoja_config);
}
