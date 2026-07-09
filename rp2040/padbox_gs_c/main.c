#include "hoja.h"
#include "board_config.h"
#include "main.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"

#include "hal/adc_hal.h"
#include "input/input_config.h"

// Platform Edition PadBox GS: single physical left analog stick; the "right
// stick" and paddle inputs are buttons (RX/RY/NB1/NB2/L3/R3). The right stick
// axes are reported centered.
adc_hal_driver_s joystick_driver_lx = {
    .gpio = 29
};

adc_hal_driver_s joystick_driver_ly = {
    .gpio = 28
};

static const hoja_config_s _hoja_config = {
    .battery_capacity_mah     = 0,
    .battery_part_code        = NULL,
    .battery_shutdown_percent = 0,

    .device_name   = "Padbox GS-C",
    .device_maker  = "arkodd.com",
    .manifest_url  = "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/padbox_gs_c/manifest.json",
    .firmware_url  = "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/padbox_gs_c/padbox_gs_c.uf2",
    .manual_url    = "https://arkodd.com/the-padbox-gs-detailed-description/",
    .fcc_id        = NULL,

    .usb_vid = 0,
    .usb_pid = 0,

    .sinput = {
        .gamepad_type       = (sinput_sdl_gamepad_type_t)1,
        .face_buttons_style = (sinput_sdl_face_style_t)1,
        .gamepad_subtype    = 0,
    },

    .sewn_layout = SEWN_LAYOUT_ABXY,
    .tourney_macro_code = INPUT_CODE_UNUSED,

    .shipping_macro_code = { INPUT_CODE_UNUSED, INPUT_CODE_UNUSED },

    .sync_on_boot_code = INPUT_CODE_UNUSED,
    .sync_macro_code   = { INPUT_CODE_UNUSED, INPUT_CODE_UNUSED },

    .usb_bootloader_code = { INPUT_CODE_LB, INPUT_CODE_START },
    .wlan_force_code     = INPUT_CODE_UNUSED,

    // Shared N64/GameCube joybus line (data on the SNES/NESBUS data pin).
    .joybus = {
        .data_pin = 24,
    },

    // SNES serial line.
    .nesbus = {
        .data_pin  = 24,
        .clock_pin = 25,
        .latch_pin = 23,
    },

    .inputs = {
        .slots = {
            { .code = INPUT_CODE_SOUTH,    .type = INPUT_TYPE_DIGITAL,  .name = "A" },
            { .code = INPUT_CODE_EAST,     .type = INPUT_TYPE_DIGITAL,  .name = "B" },
            { .code = INPUT_CODE_WEST,     .type = INPUT_TYPE_DIGITAL,  .name = "X" },
            { .code = INPUT_CODE_NORTH,    .type = INPUT_TYPE_DIGITAL,  .name = "Y" },
            { .code = INPUT_CODE_UP,       .type = INPUT_TYPE_DIGITAL,  .name = "D Up" },
            { .code = INPUT_CODE_DOWN,     .type = INPUT_TYPE_DIGITAL,  .name = "D Down" },
            { .code = INPUT_CODE_LEFT,     .type = INPUT_TYPE_DIGITAL,  .name = "D Left" },
            { .code = INPUT_CODE_RIGHT,    .type = INPUT_TYPE_DIGITAL,  .name = "D Right" },
            { .code = INPUT_CODE_LB,       .type = INPUT_TYPE_DIGITAL,  .name = "L\u2197" },
            { .code = INPUT_CODE_RB,       .type = INPUT_TYPE_DIGITAL,  .name = "R\u2197" },
            { .code = INPUT_CODE_LT,       .type = INPUT_TYPE_DIGITAL,  .name = "L\u2198" },
            { .code = INPUT_CODE_RT,       .type = INPUT_TYPE_DIGITAL,  .name = "R\u2198" },
            { .code = INPUT_CODE_LP1,      .type = INPUT_TYPE_DIGITAL,  .name = "NB1" },
            { .code = INPUT_CODE_RP1,      .type = INPUT_TYPE_DIGITAL,  .name = "NB2" },
            { .code = INPUT_CODE_LP2,      .type = INPUT_TYPE_DIGITAL,  .name = "RX" },
            { .code = INPUT_CODE_RP2,      .type = INPUT_TYPE_DIGITAL,  .name = "RY" },
            { .code = INPUT_CODE_START,    .type = INPUT_TYPE_DIGITAL,  .name = "Start" },
            { .code = INPUT_CODE_SELECT,   .type = INPUT_TYPE_DIGITAL,  .name = "Select" },
            { .code = INPUT_CODE_HOME,     .type = INPUT_TYPE_DIGITAL,  .name = "Home" },
            { .code = INPUT_CODE_SHARE,    .type = INPUT_TYPE_DIGITAL,  .name = "Capture" },
            { .code = INPUT_CODE_MISC3,    .type = INPUT_TYPE_DIGITAL,  .name = "R3" },
            { .code = INPUT_CODE_LS,       .type = INPUT_TYPE_DIGITAL,  .name = "L3" },
            { .code = INPUT_CODE_LX_RIGHT, .type = INPUT_TYPE_JOYSTICK, .name = "LX+" },
            { .code = INPUT_CODE_LX_LEFT,  .type = INPUT_TYPE_JOYSTICK, .name = "LX-" },
            { .code = INPUT_CODE_LY_UP,    .type = INPUT_TYPE_JOYSTICK, .name = "LY+" },
            { .code = INPUT_CODE_LY_DOWN,  .type = INPUT_TYPE_JOYSTICK, .name = "LY-" },
        },
    },

    .defaults_switch = {
        .maps = {
            INPUT_DEFAULT(INPUT_CODE_SOUTH,     SWITCH_CODE_A),
            INPUT_DEFAULT(INPUT_CODE_EAST,      SWITCH_CODE_B),
            INPUT_DEFAULT(INPUT_CODE_WEST,      SWITCH_CODE_X),
            INPUT_DEFAULT(INPUT_CODE_NORTH,     SWITCH_CODE_Y),
            INPUT_DEFAULT(INPUT_CODE_UP,        SWITCH_CODE_UP),
            INPUT_DEFAULT(INPUT_CODE_DOWN,      SWITCH_CODE_DOWN),
            INPUT_DEFAULT(INPUT_CODE_LEFT,      SWITCH_CODE_LEFT),
            INPUT_DEFAULT(INPUT_CODE_RIGHT,     SWITCH_CODE_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_LB,        SWITCH_CODE_L),
            INPUT_DEFAULT(INPUT_CODE_RB,        SWITCH_CODE_ZR),
            INPUT_DEFAULT(INPUT_CODE_LP1,       SWITCH_CODE_ZL),
            INPUT_DEFAULT(INPUT_CODE_RP1,       SWITCH_CODE_A),
            INPUT_DEFAULT(INPUT_CODE_LP2,       SWITCH_CODE_RY_UP),
            INPUT_DEFAULT(INPUT_CODE_RP2,       SWITCH_CODE_RX_LEFT),
            INPUT_DEFAULT(INPUT_CODE_START,     SWITCH_CODE_PLUS),
            INPUT_DEFAULT(INPUT_CODE_SELECT,    SWITCH_CODE_MINUS),
            INPUT_DEFAULT(INPUT_CODE_HOME,      SWITCH_CODE_HOME),
            INPUT_DEFAULT(INPUT_CODE_SHARE,     SWITCH_CODE_CAPTURE),
            INPUT_DEFAULT(INPUT_CODE_MISC3,     SWITCH_CODE_RX_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_LS,        SWITCH_CODE_RY_DOWN),
            INPUT_DEFAULT(INPUT_CODE_LX_RIGHT,  SWITCH_CODE_LX_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_LX_LEFT,   SWITCH_CODE_LX_LEFT),
            INPUT_DEFAULT(INPUT_CODE_LY_UP,     SWITCH_CODE_LY_UP),
            INPUT_DEFAULT(INPUT_CODE_LY_DOWN,   SWITCH_CODE_LY_DOWN),
            INPUT_DEFAULTS_END,
        },
    },

    .defaults_snes = {
        .maps = {
            INPUT_DEFAULT(INPUT_CODE_SOUTH,     SNES_CODE_A),
            INPUT_DEFAULT(INPUT_CODE_EAST,      SNES_CODE_B),
            INPUT_DEFAULT(INPUT_CODE_WEST,      SNES_CODE_X),
            INPUT_DEFAULT(INPUT_CODE_NORTH,     SNES_CODE_Y),
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
            INPUT_DEFAULT(INPUT_CODE_EAST,      N64_CODE_R),
            INPUT_DEFAULT(INPUT_CODE_WEST,      N64_CODE_B),
            INPUT_DEFAULT(INPUT_CODE_NORTH,     N64_CODE_L),
            INPUT_DEFAULT(INPUT_CODE_UP,        N64_CODE_UP),
            INPUT_DEFAULT(INPUT_CODE_DOWN,      N64_CODE_DOWN),
            INPUT_DEFAULT(INPUT_CODE_LEFT,      N64_CODE_LEFT),
            INPUT_DEFAULT(INPUT_CODE_RIGHT,     N64_CODE_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_LP1,       N64_CODE_Z),
            INPUT_DEFAULT(INPUT_CODE_LP2,       N64_CODE_CUP),
            INPUT_DEFAULT(INPUT_CODE_RP2,       N64_CODE_CLEFT),
            INPUT_DEFAULT(INPUT_CODE_START,     N64_CODE_START),
            INPUT_DEFAULT(INPUT_CODE_SELECT,    N64_CODE_L),
            INPUT_DEFAULT(INPUT_CODE_MISC3,     N64_CODE_CRIGHT),
            INPUT_DEFAULT(INPUT_CODE_LS,        N64_CODE_CDOWN),
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
            INPUT_DEFAULT(INPUT_CODE_EAST,      GAMECUBE_CODE_X),
            INPUT_DEFAULT(INPUT_CODE_WEST,      GAMECUBE_CODE_R),
            INPUT_DEFAULT(INPUT_CODE_NORTH,     GAMECUBE_CODE_Y),
            INPUT_DEFAULT(INPUT_CODE_UP,        GAMECUBE_CODE_UP),
            INPUT_DEFAULT(INPUT_CODE_DOWN,      GAMECUBE_CODE_DOWN),
            INPUT_DEFAULT(INPUT_CODE_LEFT,      GAMECUBE_CODE_LEFT),
            INPUT_DEFAULT(INPUT_CODE_RIGHT,     GAMECUBE_CODE_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_RB,        GAMECUBE_CODE_Z),
            INPUT_DEFAULT(INPUT_CODE_LT,        GAMECUBE_CODE_L_ANALOG),
            INPUT_DEFAULT(INPUT_CODE_RT,        GAMECUBE_CODE_R_ANALOG),
            INPUT_DEFAULT(INPUT_CODE_LP1,       GAMECUBE_CODE_L),
            INPUT_DEFAULT(INPUT_CODE_RP1,       GAMECUBE_CODE_A),
            INPUT_DEFAULT(INPUT_CODE_LP2,       GAMECUBE_CODE_RY_UP),
            INPUT_DEFAULT(INPUT_CODE_RP2,       GAMECUBE_CODE_RX_LEFT),
            INPUT_DEFAULT(INPUT_CODE_START,     GAMECUBE_CODE_START),
            INPUT_DEFAULT(INPUT_CODE_MISC3,     GAMECUBE_CODE_RX_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_LS,        GAMECUBE_CODE_RY_DOWN),
            INPUT_DEFAULT(INPUT_CODE_LX_RIGHT,  GAMECUBE_CODE_LX_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_LX_LEFT,   GAMECUBE_CODE_LX_LEFT),
            INPUT_DEFAULT(INPUT_CODE_LY_UP,     GAMECUBE_CODE_LY_UP),
            INPUT_DEFAULT(INPUT_CODE_LY_DOWN,   GAMECUBE_CODE_LY_DOWN),
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
            INPUT_DEFAULT(INPUT_CODE_LT,        XINPUT_CODE_LT_ANALOG),
            INPUT_DEFAULT(INPUT_CODE_RT,        XINPUT_CODE_RT_ANALOG),
            INPUT_DEFAULT(INPUT_CODE_LP1,       XINPUT_CODE_LS),
            INPUT_DEFAULT(INPUT_CODE_LP2,       XINPUT_CODE_RS),
            INPUT_DEFAULT(INPUT_CODE_START,     XINPUT_CODE_START),
            INPUT_DEFAULT(INPUT_CODE_SELECT,    XINPUT_CODE_BACK),
            INPUT_DEFAULT(INPUT_CODE_HOME,      XINPUT_CODE_GUIDE),
            INPUT_DEFAULT(INPUT_CODE_LX_RIGHT,  XINPUT_CODE_LX_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_LX_LEFT,   XINPUT_CODE_LX_LEFT),
            INPUT_DEFAULT(INPUT_CODE_LY_UP,     XINPUT_CODE_LY_UP),
            INPUT_DEFAULT(INPUT_CODE_LY_DOWN,   XINPUT_CODE_LY_DOWN),
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
            INPUT_DEFAULT(INPUT_CODE_LP1,       SINPUT_CODE_LP_1),
            INPUT_DEFAULT(INPUT_CODE_RP1,       SINPUT_CODE_RP_1),
            INPUT_DEFAULT(INPUT_CODE_LP2,       SINPUT_CODE_LP_2),
            INPUT_DEFAULT(INPUT_CODE_RP2,       SINPUT_CODE_RP_2),
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
            INPUT_DEFAULTS_END,
        },
    },

    .rgb_driver = {
        .gpio_pin = 14,
    },

    .rgb = {
        .groups = {
            { .name = "L\u2197", .leds = RGB_GROUP(0) },
            { .name = "R\u2197", .leds = RGB_GROUP(1) },
            { .name = "B",      .leds = RGB_GROUP(2) },
            { .name = "A",      .leds = RGB_GROUP(3) },
            { .name = "X",      .leds = RGB_GROUP(4) },
            { .name = "Y",      .leds = RGB_GROUP(5) },
            { .name = "R\u2198", .leds = RGB_GROUP(6) },
            { .name = "L\u2198", .leds = RGB_GROUP(7) },
            { .name = "L3",     .leds = RGB_GROUP(9) },
            { .name = "R3",     .leds = RGB_GROUP(12) },
            { .name = "RX",     .leds = RGB_GROUP(11) },
            { .name = "RY",     .leds = RGB_GROUP(10) },
            { .name = "NB2",    .leds = RGB_GROUP(8) },
        },

        .notification_group_index = -1,
        .notification_group_size  = 0,
        .player_group_index       = -1,

        .key_mapping_count = 13,
        .key_mappings = {
            { INPUT_CODE_LB,    0 },
            { INPUT_CODE_RB,    1 },
            { INPUT_CODE_EAST,  2 },
            { INPUT_CODE_SOUTH, 3 },
            { INPUT_CODE_WEST,  4 },
            { INPUT_CODE_NORTH, 5 },
            { INPUT_CODE_RT,    6 },
            { INPUT_CODE_LT,    7 },
            { INPUT_CODE_LS,    8 },
            { INPUT_CODE_MISC3, 9 },
            { INPUT_CODE_LP2,   10 },
            { INPUT_CODE_RP2,   11 },
            { INPUT_CODE_RP1,   12 },
        },
    },
};

void _local_setup_btn(uint32_t gpio)
{
    gpio_init(gpio);
    gpio_pull_up(gpio);
    gpio_set_dir(gpio, GPIO_IN);
}

void cb_hoja_init()
{
    stdio_init_all();

    _local_setup_btn(PGPIO_BTN_A);
    _local_setup_btn(PGPIO_BTN_B);
    _local_setup_btn(PGPIO_BTN_X);
    _local_setup_btn(PGPIO_BTN_Y);

    _local_setup_btn(PGPIO_BTN_L3);
    _local_setup_btn(PGPIO_BTN_R3);

    _local_setup_btn(PGPIO_BTN_NB1);
    _local_setup_btn(PGPIO_BTN_NB2);

    _local_setup_btn(PGPIO_BTN_RX);
    _local_setup_btn(PGPIO_BTN_RY);

    _local_setup_btn(PGPIO_BTN_TL);
    _local_setup_btn(PGPIO_BTN_TR);
    _local_setup_btn(PGPIO_BTN_TZL);
    _local_setup_btn(PGPIO_BTN_TZR);

    _local_setup_btn(PGPIO_BTN_BACK);
    _local_setup_btn(PGPIO_BTN_MENU);
    _local_setup_btn(PGPIO_BTN_HOME);
    _local_setup_btn(PGPIO_BTN_CAPTURE);

    _local_setup_btn(PGPIO_BTN_DUP);
    _local_setup_btn(PGPIO_BTN_DDOWN);
    _local_setup_btn(PGPIO_BTN_DLEFT);
    _local_setup_btn(PGPIO_BTN_DRIGHT);

    adc_hal_init(&joystick_driver_lx);
    adc_hal_init(&joystick_driver_ly);
}

void cb_hoja_read_joystick(uint16_t *input)
{
    adc_hal_read(&joystick_driver_lx);
    input[0] = joystick_driver_lx.output;

    adc_hal_read(&joystick_driver_ly);
    input[1] = 0xFFF - joystick_driver_ly.output;

    // No physical right stick; report centered.
    input[2] = 2048;
    input[3] = 2048;
}

void cb_hoja_read_input(mapper_input_s *input)
{
    bool *out = input->presses;

    out[INPUT_CODE_UP]      = !gpio_get(PGPIO_BTN_DUP);
    out[INPUT_CODE_DOWN]    = !gpio_get(PGPIO_BTN_DDOWN);
    out[INPUT_CODE_LEFT]    = !gpio_get(PGPIO_BTN_DLEFT);
    out[INPUT_CODE_RIGHT]   = !gpio_get(PGPIO_BTN_DRIGHT);

    out[INPUT_CODE_SOUTH]   = !gpio_get(PGPIO_BTN_A);
    out[INPUT_CODE_EAST]    = !gpio_get(PGPIO_BTN_B);
    out[INPUT_CODE_WEST]    = !gpio_get(PGPIO_BTN_X);
    out[INPUT_CODE_NORTH]   = !gpio_get(PGPIO_BTN_Y);

    out[INPUT_CODE_START]   = !gpio_get(PGPIO_BTN_MENU);
    out[INPUT_CODE_SELECT]  = !gpio_get(PGPIO_BTN_BACK);
    out[INPUT_CODE_HOME]    = !gpio_get(PGPIO_BTN_HOME);
    out[INPUT_CODE_SHARE]   = !gpio_get(PGPIO_BTN_CAPTURE);

    out[INPUT_CODE_LS]      = !gpio_get(PGPIO_BTN_L3);
    out[INPUT_CODE_MISC3]   = !gpio_get(PGPIO_BTN_R3);

    out[INPUT_CODE_LP2]     = !gpio_get(PGPIO_BTN_RX);
    out[INPUT_CODE_RP2]     = !gpio_get(PGPIO_BTN_RY);

    out[INPUT_CODE_LP1]     = !gpio_get(PGPIO_BTN_NB1);
    out[INPUT_CODE_RP1]     = !gpio_get(PGPIO_BTN_NB2);

    out[INPUT_CODE_LB]      = !gpio_get(PGPIO_BTN_TL);
    out[INPUT_CODE_RB]      = !gpio_get(PGPIO_BTN_TR);
    out[INPUT_CODE_LT]      = !gpio_get(PGPIO_BTN_TZL);
    out[INPUT_CODE_RT]      = !gpio_get(PGPIO_BTN_TZR);
}

int main()
{
    hoja_init(&_hoja_config);
}
