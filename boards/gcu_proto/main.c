#include "hoja.h"
#include "input/button.h"
#include "board_config.h"
#include "main.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h" 

/*
button_remap_s user_map = {
    .dpad_up = MAPCODE_DUP,
    .dpad_down = MAPCODE_DDOWN,
    .dpad_left = MAPCODE_DLEFT,
    .dpad_right = MAPCODE_DRIGHT,

    .button_a = MAPCODE_B_A,
    .button_b = MAPCODE_B_B,
    .button_x = MAPCODE_B_X,
    .button_y = MAPCODE_B_Y,

    .trigger_l = MAPCODE_T_ZL,
    .trigger_r = MAPCODE_T_ZR,
    .trigger_zl = MAPCODE_T_L,
    .trigger_zr = MAPCODE_T_R,

    .button_plus = MAPCODE_B_PLUS,
    .button_minus = MAPCODE_B_MINUS,
    .button_stick_left = MAPCODE_B_STICKL,
    .button_stick_right = MAPCODE_B_STICKR,
};
*/ 

adc_driver_cfg_s gcp_adc_1 = {
    .driver_type = ADC_DRIVER_MCP3002,
    .driver_instance  = 0,
    .mcp3002_cfg = {.cs_gpio = 1, .spi_instance = 0}
    };

adc_driver_cfg_s gcp_adc_2 = {
    .driver_type = ADC_DRIVER_MCP3002,
    .driver_instance  = 1,
    .mcp3002_cfg = {.cs_gpio = 9, .spi_instance = 0}
    };

adc_driver_cfg_s gcp_adc_hal = {
    .driver_type = ADC_DRIVER_HAL,
    .driver_instance  = 0,
    };

void _local_setup_btn(uint32_t gpio)
{
    gpio_init(gpio);
    gpio_pull_up(gpio);
    gpio_set_dir(gpio, GPIO_IN);
}

// old SCAN
void _local_setup_push(uint32_t gpio)
{
    gpio_init(gpio);
    gpio_pull_up(gpio);
    gpio_set_dir(gpio, GPIO_IN);
}

// old PULL
void _local_setup_scan(uint32_t gpio)
{
    gpio_init(gpio);
    gpio_pull_up(gpio);
    gpio_set_dir(gpio, GPIO_OUT);
    gpio_put(gpio, true);
}

bool cb_hoja_buttons_init()
{
    stdio_init_all();
    _local_setup_btn(PGPIO_BUTTON_MODE);

    // Set up GPIO for input buttons
    _local_setup_scan(PGPIO_SCAN_A);
    _local_setup_scan(PGPIO_SCAN_B);
    _local_setup_scan(PGPIO_SCAN_C);
    _local_setup_scan(PGPIO_SCAN_D);

    _local_setup_push(PGPIO_PUSH_A);
    _local_setup_push(PGPIO_PUSH_B);
    _local_setup_push(PGPIO_PUSH_C);
    _local_setup_push(PGPIO_PUSH_D);

    return true;
}

void cb_hoja_read_buttons(button_data_s *data)
{
    gpio_put(PGPIO_SCAN_A, false);
    sleep_us(5);
    data->button_a  = !gpio_get(PGPIO_PUSH_A);
    data->button_b  = !gpio_get(PGPIO_PUSH_B);
    data->button_x  = !gpio_get(PGPIO_PUSH_C);
    data->button_y  = !gpio_get(PGPIO_PUSH_D);
    gpio_put(PGPIO_SCAN_A, true);

    gpio_put(PGPIO_SCAN_B, false);
    sleep_us(5);
    data->dpad_up       = !gpio_get(PGPIO_PUSH_A);
    data->dpad_left     = !gpio_get(PGPIO_PUSH_B);
    data->dpad_down     = !gpio_get(PGPIO_PUSH_C);
    data->dpad_right    = !gpio_get(PGPIO_PUSH_D);
    gpio_put(PGPIO_SCAN_B, true);

    gpio_put(PGPIO_SCAN_C, false);
    sleep_us(5);
    data->trigger_l     = !gpio_get(PGPIO_PUSH_A);
    data->trigger_zl    = !gpio_get(PGPIO_PUSH_B);
    data->trigger_r     = !gpio_get(PGPIO_PUSH_C);
    data->trigger_zr    = !gpio_get(PGPIO_PUSH_D);
    gpio_put(PGPIO_SCAN_C, true);

    gpio_put(PGPIO_SCAN_D, false);
    sleep_us(5);
    data->button_stick_left     = !gpio_get(PGPIO_PUSH_A);
    data->button_plus           = !gpio_get(PGPIO_PUSH_B);
    data->button_stick_right    = !gpio_get(PGPIO_PUSH_C);
    gpio_put(PGPIO_SCAN_D, true);

    data->button_shipping = !gpio_get(PGPIO_BUTTON_MODE);
    data->button_home = data->button_shipping;
    data->button_sync = data->button_plus;
}

int main()
{
    hoja_init();
}
