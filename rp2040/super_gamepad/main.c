#include "hoja.h"
#include "board_config.h"
#include "main.h"
#include "cores/cores.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"

#include "hal/adc_hal.h"
#include "input/input_config.h"

adc_hal_driver_s battery_adc_1 = {
    .gpio = 29
};

static const hoja_config_s _hoja_config = {
    .i2c = {
        .instance_0 = {
            .enabled = false,
        },
        .instance_1 = {
            .enabled      = true,
            .sda_gpio     = 26,
            .scl_gpio     = 27,
            .baudrate_khz = 400,
        },
    },

    .spi = {
        .instance_0 = {
            .enabled = false,
        },
        .instance_1 = {
            .enabled = false,
        },
    },

    .battery = {
        .i2c_instance   = 1,
        .charge_rate_ma = 100,
    },

    .fuelgauge = {
        .voltage_offset = 0.125f,
    },

    .battery_capacity_mah     = 525,
    .battery_part_code        = "HAC-006",
    .battery_shutdown_percent = 0,

    .device_name   = "SuperGamepad+",
    .device_maker  = "HHL",
    .manifest_url  = "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/super_gamepad/manifest.json",
    .firmware_url  = "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/super_gamepad/super_gamepad.uf2",
    .manual_url    = "https://docs.handheldlegend.com/s/portal/doc/user-guide-ZCJzKM9emw",
    .fcc_id        = NULL,

    .usb_vid = 0,
    .usb_pid = 0,

    .sinput = {
        .gamepad_type       = SINPUT_SDL_GAMEPAD_TYPE_STANDARD,
        .face_buttons_style = SINPUT_SDL_GAMEPAD_TYPE_NINTENDO_PRO,
        .gamepad_subtype    = 0,
    },

    .sewn_layout = SEWN_LAYOUT_BAYX,
    .tourney_macro_code = INPUT_CODE_UNUSED,

    .shipping_macro_code = { INPUT_CODE_SELECT, INPUT_CODE_UNUSED },

    .sync_on_boot_code = INPUT_CODE_START,
    .sync_macro_code   = { INPUT_CODE_UNUSED, INPUT_CODE_UNUSED },

    .usb_bootloader_code  = { INPUT_CODE_LB, INPUT_CODE_START },
    .baseband_update_code = { INPUT_CODE_RB, INPUT_CODE_START },
    .wlan_force_code      = INPUT_CODE_UNUSED,

    .joybus = {
        .data_pin = 7,
    },

    .nesbus = {
        .data_pin  = 7,
        .clock_pin = 8,
        .latch_pin = 9,
    },

    .inputs = {
        .slots = {
            { .code = INPUT_CODE_SOUTH,  .type = INPUT_TYPE_DIGITAL, .name = "B" },
            { .code = INPUT_CODE_EAST,   .type = INPUT_TYPE_DIGITAL, .name = "A" },
            { .code = INPUT_CODE_WEST,   .type = INPUT_TYPE_DIGITAL, .name = "Y" },
            { .code = INPUT_CODE_NORTH,  .type = INPUT_TYPE_DIGITAL, .name = "X" },
            { .code = INPUT_CODE_UP,     .type = INPUT_TYPE_DIGITAL, .name = "D Up" },
            { .code = INPUT_CODE_DOWN,   .type = INPUT_TYPE_DIGITAL, .name = "D Down" },
            { .code = INPUT_CODE_LEFT,   .type = INPUT_TYPE_DIGITAL, .name = "D Left" },
            { .code = INPUT_CODE_RIGHT,  .type = INPUT_TYPE_DIGITAL, .name = "D Right" },
            { .code = INPUT_CODE_LB,     .type = INPUT_TYPE_DIGITAL, .name = "L" },
            { .code = INPUT_CODE_RB,     .type = INPUT_TYPE_DIGITAL, .name = "R" },
            { .code = INPUT_CODE_START,  .type = INPUT_TYPE_DIGITAL, .name = "Start" },
            { .code = INPUT_CODE_SELECT, .type = INPUT_TYPE_DIGITAL, .name = "Select" },
            { .code = INPUT_CODE_HOME,   .type = INPUT_TYPE_DIGITAL, .name = "Home" },
            { .code = INPUT_CODE_SHARE,  .type = INPUT_TYPE_DIGITAL, .name = "Capture" },
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
            INPUT_DEFAULT(INPUT_CODE_START,     SWITCH_CODE_PLUS),
            INPUT_DEFAULT(INPUT_CODE_SELECT,    SWITCH_CODE_MINUS),
            INPUT_DEFAULT(INPUT_CODE_HOME,      SWITCH_CODE_HOME),
            INPUT_DEFAULT(INPUT_CODE_SHARE,     SWITCH_CODE_CAPTURE),
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
            INPUT_DEFAULT(INPUT_CODE_SOUTH,     N64_CODE_B),
            INPUT_DEFAULT(INPUT_CODE_EAST,      N64_CODE_A),
            INPUT_DEFAULT(INPUT_CODE_WEST,      N64_CODE_CDOWN),
            INPUT_DEFAULT(INPUT_CODE_NORTH,     N64_CODE_CUP),
            INPUT_DEFAULT(INPUT_CODE_UP,        N64_CODE_LY_UP),
            INPUT_DEFAULT(INPUT_CODE_DOWN,      N64_CODE_LY_DOWN),
            INPUT_DEFAULT(INPUT_CODE_LEFT,      N64_CODE_LX_LEFT),
            INPUT_DEFAULT(INPUT_CODE_RIGHT,     N64_CODE_LX_RIGHT),
            INPUT_DEFAULT(INPUT_CODE_LB,        N64_CODE_Z),
            INPUT_DEFAULT(INPUT_CODE_RB,        N64_CODE_CRIGHT),
            INPUT_DEFAULT(INPUT_CODE_START,     N64_CODE_START),
            INPUT_DEFAULT(INPUT_CODE_SELECT,    N64_CODE_CLEFT),
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
            INPUT_DEFAULT(INPUT_CODE_LB,        GAMECUBE_CODE_L),
            INPUT_DEFAULT(INPUT_CODE_RB,        GAMECUBE_CODE_R),
            INPUT_DEFAULT(INPUT_CODE_START,     GAMECUBE_CODE_START),
            INPUT_DEFAULT(INPUT_CODE_SELECT,    GAMECUBE_CODE_Z),
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
            INPUT_DEFAULT(INPUT_CODE_START,     XINPUT_CODE_START),
            INPUT_DEFAULT(INPUT_CODE_SELECT,    XINPUT_CODE_BACK),
            INPUT_DEFAULT(INPUT_CODE_HOME,      XINPUT_CODE_GUIDE),
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
            INPUT_DEFAULT(INPUT_CODE_START,     SINPUT_CODE_START),
            INPUT_DEFAULT(INPUT_CODE_SELECT,    SINPUT_CODE_SELECT),
            INPUT_DEFAULT(INPUT_CODE_HOME,      SINPUT_CODE_GUIDE),
            INPUT_DEFAULT(INPUT_CODE_SHARE,     SINPUT_CODE_SHARE),
            INPUT_DEFAULTS_END,
        },
    },

    .rgb_driver = {
        .gpio_pin = 15,
    },

    .rgb = {
        .groups = {
            { .name = "A",       .leds = RGB_GROUP(9) },
            { .name = "B",       .leds = RGB_GROUP(7) },
            { .name = "X",       .leds = RGB_GROUP(8) },
            { .name = "Y",       .leds = RGB_GROUP(6) },
            { .name = "D Up",    .leds = RGB_GROUP(1) },
            { .name = "D Down",  .leds = RGB_GROUP(2) },
            { .name = "D Left",  .leds = RGB_GROUP(0) },
            { .name = "D Right", .leds = RGB_GROUP(3) },
            { .name = "Start",   .leds = RGB_GROUP(5) },
            { .name = "Select",  .leds = RGB_GROUP(4) },
        },

        .notification_group_index = 9,
        .notification_group_size  = 1,
        .player_group_index       = -1,
        .key_mapping_count = 10,
        .key_mappings = {
            { INPUT_CODE_EAST,  0 },
            { INPUT_CODE_SOUTH, 1 },
            { INPUT_CODE_NORTH, 2 },
            { INPUT_CODE_WEST,  3 },
            { INPUT_CODE_UP,    4 },
            { INPUT_CODE_DOWN,  5 },
            { INPUT_CODE_LEFT,  6 },
            { INPUT_CODE_RIGHT, 7 },
            { INPUT_CODE_START, 8 },
            { INPUT_CODE_SELECT, 9 },
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

uint16_t cb_hoja_read_battery()
{
    adc_hal_read(&battery_adc_1);
    return battery_adc_1.output;
}

void cb_hoja_init()
{
    stdio_init_all();

    // Set up GPIO for input buttons
    _local_setup_btn(PGPIO_BUTTON_SELECT);

    _local_setup_push(PGPIO_PUSH_A);
    _local_setup_push(PGPIO_PUSH_B);
    _local_setup_push(PGPIO_PUSH_C);
    _local_setup_push(PGPIO_PUSH_D);

    _local_setup_scan(PGPIO_SCAN_A);
    _local_setup_scan(PGPIO_SCAN_B);
    _local_setup_scan(PGPIO_SCAN_C);
    _local_setup_scan(PGPIO_SCAN_D);

    adc_hal_init(&battery_adc_1);
}

#define BUTTON_SLEEP_US 15

void cb_hoja_read_input(mapper_input_s *input)
{
    bool *out = input->presses;

    // Keypad version
    gpio_put(PGPIO_SCAN_A, false);
    sleep_us(BUTTON_SLEEP_US);
    out[INPUT_CODE_EAST] = !gpio_get(PGPIO_PUSH_C);
    out[INPUT_CODE_SOUTH] = !gpio_get(PGPIO_PUSH_D);
    out[INPUT_CODE_NORTH] = !gpio_get(PGPIO_PUSH_A);
    out[INPUT_CODE_WEST] = !gpio_get(PGPIO_PUSH_B);
    gpio_put(PGPIO_SCAN_A, true);

    gpio_put(PGPIO_SCAN_B, false);
    sleep_us(BUTTON_SLEEP_US);
    out[INPUT_CODE_LEFT]  = !gpio_get(PGPIO_PUSH_D);
    out[INPUT_CODE_RIGHT] = !gpio_get(PGPIO_PUSH_C);
    out[INPUT_CODE_DOWN]  = !gpio_get(PGPIO_PUSH_B);
    out[INPUT_CODE_UP]    = !gpio_get(PGPIO_PUSH_A);
    gpio_put(PGPIO_SCAN_B, true);

    gpio_put(PGPIO_SCAN_C, false);
    sleep_us(BUTTON_SLEEP_US);
    out[INPUT_CODE_LB]  = !gpio_get(PGPIO_PUSH_C);
    out[INPUT_CODE_RB]  = !gpio_get(PGPIO_PUSH_B);
    out[INPUT_CODE_START]  = !gpio_get(PGPIO_PUSH_A);
    //
    gpio_put(PGPIO_SCAN_C, true);

    out[INPUT_CODE_SELECT] = !gpio_get(PGPIO_BUTTON_SELECT);

    if(core_current_reportformat() != CORE_REPORTFORMAT_SNES)
    {
        if(out[INPUT_CODE_SELECT] && out[INPUT_CODE_RB])
        {
            out[INPUT_CODE_SELECT] = false;
            out[INPUT_CODE_RB]     = false;
            out[INPUT_CODE_HOME]   = true;
        }
        else out[INPUT_CODE_HOME] = false;

        if(out[INPUT_CODE_SELECT] && out[INPUT_CODE_LB])
        {
            out[INPUT_CODE_SELECT] = false;
            out[INPUT_CODE_LB]     = false;
            out[INPUT_CODE_SHARE]  = true;
        }
        else out[INPUT_CODE_SHARE] = false;
    }
}

int main()
{
    hoja_init(&_hoja_config);
}
