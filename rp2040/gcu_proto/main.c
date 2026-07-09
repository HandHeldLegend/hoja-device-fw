#include "hoja.h"
#include "board_config.h"
#include "main.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"

#include "drivers/adc/mcp3002.h"
#include "hal/adc_hal.h"
#include "input/input_config.h"

#define GCU_INPUT_CODE_A    INPUT_CODE_SOUTH
#define GCU_INPUT_CODE_X    INPUT_CODE_EAST
#define GCU_INPUT_CODE_B    INPUT_CODE_WEST
#define GCU_INPUT_CODE_Y    INPUT_CODE_NORTH
#define GCU_INPUT_CODE_PWR  INPUT_CODE_MISC3

adc_mcp3002_driver_s joystick_driver_1 = {
    .cs_gpio = 1,
    .spi_instance = 0,
};

adc_mcp3002_driver_s joystick_driver_2 = {
    .cs_gpio = 9,
    .spi_instance = 0,
};

adc_hal_driver_s trigger_driver_l = {
    .gpio = 29,
};

adc_hal_driver_s trigger_driver_r = {
    .gpio = 27,
};

static const hoja_config_s _hoja_config = {
    .i2c = {
        .instance_0 = {
            .enabled = false,
        },
        .instance_1 = {
            .enabled      = true,
            .sda_gpio     = 22,
            .scl_gpio     = 23,
            .baudrate_khz = 400,
        },
    },

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

    .battery = {
        .i2c_instance   = 1,
        .charge_rate_ma = 100,
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

    .battery_capacity_mah     = 1200,
    .battery_part_code        = "BDT 903035",
    .battery_shutdown_percent = 0,

    .device_name   = "GCU-Proto",
    .device_maker  = "HHL",
    .manifest_url  = "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/gcu_proto/manifest.json",
    .firmware_url  = "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/gcu_proto/gcu_proto.uf2",
    .manual_url    = "https://docs.handheldlegend.com/s/portal/doc/user-guide-UoDtIku68z",
    .fcc_id        = NULL,

    .usb_vid = 0x2E8A,
    .usb_pid = 0x10DD,

    .sinput = {
        .gamepad_type       = SINPUT_SDL_GAMEPAD_TYPE_GAMECUBE,
        .face_buttons_style = SINPUT_SDL_GAMEPAD_FACE_STYLE_AXBY,
        .gamepad_subtype    = 0,
    },

    .sewn_layout = SEWN_LAYOUT_AXBY,
    .tourney_macro_code = INPUT_CODE_UNUSED,

    .shipping_macro_code = { GCU_INPUT_CODE_PWR, INPUT_CODE_UNUSED },

    .sync_on_boot_code = INPUT_CODE_START,
    .sync_macro_code   = { INPUT_CODE_UNUSED, INPUT_CODE_UNUSED },

    .usb_bootloader_code  = { INPUT_CODE_LB, INPUT_CODE_START },
    .baseband_update_code = { INPUT_CODE_RB, INPUT_CODE_START },
    .wlan_force_code      = INPUT_CODE_UNUSED,

    .joybus = {
        .data_pin = 19,
    },

    .nesbus = {
        .data_pin  = 19,
        .clock_pin = 20,
        .latch_pin = 18,
    },

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
            INPUT_DEFAULT(GCU_INPUT_CODE_A,     SNES_CODE_A),
            INPUT_DEFAULT(GCU_INPUT_CODE_X,     SNES_CODE_X),
            INPUT_DEFAULT(GCU_INPUT_CODE_B,     SNES_CODE_B),
            INPUT_DEFAULT(GCU_INPUT_CODE_Y,     SNES_CODE_Y),
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
        .channel_a_pin  = 3,
        .brake_pin      = 0,
        .brake_enable   = false,
        .intensity_max  = 1.0f,
    },

    .rgb_driver = {
        .gpio_pin = 21,
    },

    .rgb = {
        .groups = {
            { .name = "L Stick", .leds = RGB_GROUP(0, 1, 2, 3) },
        },

        .notification_group_index = -1,
        .notification_group_size  = 0,
        .player_group_index       = -1,

        .key_mapping_count = 1,
        .key_mappings = {
            { INPUT_CODE_LX_RIGHT, 0 },
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

void cb_hoja_init()
{
    stdio_init_all();

    _local_setup_btn(PGPIO_BUTTON_MODE);
    _local_setup_scan(PGPIO_SCAN_A);
    _local_setup_scan(PGPIO_SCAN_B);
    _local_setup_scan(PGPIO_SCAN_C);
    _local_setup_scan(PGPIO_SCAN_D);

    _local_setup_push(PGPIO_PUSH_A);
    _local_setup_push(PGPIO_PUSH_B);
    _local_setup_push(PGPIO_PUSH_C);
    _local_setup_push(PGPIO_PUSH_D);

    mcp3002_init(&joystick_driver_1);
    mcp3002_init(&joystick_driver_2);
    adc_hal_init(&trigger_driver_r);
    adc_hal_init(&trigger_driver_l);
}

#define BUTTON_SLEEP_US 15

void cb_hoja_read_input(mapper_input_s *input)
{
    adc_hal_read(&trigger_driver_r);
    adc_hal_read(&trigger_driver_l);

    bool *out = input->presses;

    gpio_put(PGPIO_SCAN_A, false);
    sleep_us(BUTTON_SLEEP_US);
    out[GCU_INPUT_CODE_A] = !gpio_get(PGPIO_PUSH_A);
    out[GCU_INPUT_CODE_X] = !gpio_get(PGPIO_PUSH_B);
    out[GCU_INPUT_CODE_B] = !gpio_get(PGPIO_PUSH_C);
    out[GCU_INPUT_CODE_Y] = !gpio_get(PGPIO_PUSH_D);
    gpio_put(PGPIO_SCAN_A, true);

    gpio_put(PGPIO_SCAN_B, false);
    sleep_us(BUTTON_SLEEP_US);
    out[INPUT_CODE_UP] = !gpio_get(PGPIO_PUSH_A);
    out[INPUT_CODE_LEFT] = !gpio_get(PGPIO_PUSH_B);
    out[INPUT_CODE_DOWN] = !gpio_get(PGPIO_PUSH_C);
    out[INPUT_CODE_RIGHT] = !gpio_get(PGPIO_PUSH_D);
    gpio_put(PGPIO_SCAN_B, true);

    gpio_put(PGPIO_SCAN_C, false);
    sleep_us(BUTTON_SLEEP_US);
    out[INPUT_CODE_LB] = !gpio_get(PGPIO_PUSH_A);
    out[INPUT_CODE_LT] = !gpio_get(PGPIO_PUSH_B);
    out[INPUT_CODE_RB] = !gpio_get(PGPIO_PUSH_C);
    out[INPUT_CODE_RT] = !gpio_get(PGPIO_PUSH_D);
    gpio_put(PGPIO_SCAN_C, true);

    gpio_put(PGPIO_SCAN_D, false);
    sleep_us(BUTTON_SLEEP_US);
    out[INPUT_CODE_LS] = !gpio_get(PGPIO_PUSH_A);
    out[INPUT_CODE_START] = !gpio_get(PGPIO_PUSH_B);
    out[INPUT_CODE_RS] = !gpio_get(PGPIO_PUSH_C);
    gpio_put(PGPIO_SCAN_D, true);

    out[GCU_INPUT_CODE_PWR] = !gpio_get(PGPIO_BUTTON_MODE);

    input->inputs[INPUT_CODE_LT_ANALOG] = 0xFFF - trigger_driver_l.output;
    input->inputs[INPUT_CODE_RT_ANALOG] = 0xFFF - trigger_driver_r.output;
}

void cb_hoja_read_joystick(uint16_t *input)
{
    mcp3002_read(&joystick_driver_1);
    mcp3002_read(&joystick_driver_2);

    input[0] = joystick_driver_1.output_ch_0;
    input[1] = joystick_driver_1.output_ch_1;

    input[2] = joystick_driver_2.output_ch_0;
    input[3] = joystick_driver_2.output_ch_1;
}

int main()
{
    hoja_init(&_hoja_config);
}
