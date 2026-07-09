#include "hoja.h"
#include "board_config.h"
#include "main.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"

#include "input/input_config.h"

// Minimal test-build wiring: two buttons on GP0/GP1. Holding both at boot
// enters the UF2 bootloader (usb_bootloader_code below).
#define PICOW_BTN_A 0
#define PICOW_BTN_B 1

static const hoja_config_s _hoja_config = {
    .device_name   = "Pico-W",
    .device_maker  = "RPI",
    .manifest_url  = "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/pico_w/manifest.json",
    .firmware_url  = "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/pico_w/pico_w.uf2",
    .manual_url    = "https://docs.handheldlegend.com/s/portal/doc/user-guide-UoDtIku68z",
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

    .usb_bootloader_code = { INPUT_CODE_SOUTH, INPUT_CODE_EAST },
    .wlan_force_code     = INPUT_CODE_UNUSED,

    .inputs = {
        .slots = {
            { .code = INPUT_CODE_SOUTH, .type = INPUT_TYPE_DIGITAL, .name = "A" },
            { .code = INPUT_CODE_EAST,  .type = INPUT_TYPE_DIGITAL, .name = "B" },
        },
    },

    .defaults_switch = {
        .maps = {
            INPUT_DEFAULT(INPUT_CODE_SOUTH, SWITCH_CODE_A),
            INPUT_DEFAULT(INPUT_CODE_EAST,  SWITCH_CODE_B),
            INPUT_DEFAULTS_END,
        },
    },

    .defaults_snes = {
        .maps = {
            INPUT_DEFAULT(INPUT_CODE_SOUTH, SNES_CODE_A),
            INPUT_DEFAULT(INPUT_CODE_EAST,  SNES_CODE_B),
            INPUT_DEFAULTS_END,
        },
    },

    .defaults_n64 = {
        .maps = {
            INPUT_DEFAULT(INPUT_CODE_SOUTH, N64_CODE_A),
            INPUT_DEFAULT(INPUT_CODE_EAST,  N64_CODE_B),
            INPUT_DEFAULTS_END,
        },
    },

    .defaults_gamecube = {
        .maps = {
            INPUT_DEFAULT(INPUT_CODE_SOUTH, GAMECUBE_CODE_A),
            INPUT_DEFAULT(INPUT_CODE_EAST,  GAMECUBE_CODE_B),
            INPUT_DEFAULTS_END,
        },
    },

    .defaults_xinput = {
        .maps = {
            INPUT_DEFAULT(INPUT_CODE_SOUTH, XINPUT_CODE_A),
            INPUT_DEFAULT(INPUT_CODE_EAST,  XINPUT_CODE_B),
            INPUT_DEFAULTS_END,
        },
    },

    .defaults_sinput = {
        .maps = {
            INPUT_DEFAULT(INPUT_CODE_SOUTH, SINPUT_CODE_SOUTH),
            INPUT_DEFAULT(INPUT_CODE_EAST,  SINPUT_CODE_EAST),
            INPUT_DEFAULTS_END,
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

    _local_setup_btn(PICOW_BTN_A);
    _local_setup_btn(PICOW_BTN_B);
}

void cb_hoja_read_joystick(uint16_t *input)
{
    input[0] = 2048;
    input[1] = 2048;
    input[2] = 2048;
    input[3] = 2048;
}

void cb_hoja_read_input(mapper_input_s *input)
{
    bool *out = input->presses;

    out[INPUT_CODE_SOUTH] = !gpio_get(PICOW_BTN_A);
    out[INPUT_CODE_EAST]  = !gpio_get(PICOW_BTN_B);
}

int main()
{
    hoja_init(&_hoja_config);
}
