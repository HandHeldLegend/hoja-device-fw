#include "hoja.h"
#include "input/button.h"

#include "board_config.h"
#include "main.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h"

adc_driver_cfg_s user_adc_1 = {
    .driver_type = ADC_DRIVER_MCP3002,
    .driver_instance  = 0,
    .mcp3002_cfg = {.cs_gpio = 16, .spi_instance = 0}
    };

adc_driver_cfg_s user_adc_2 = {
    .driver_type = ADC_DRIVER_MCP3002,
    .driver_instance  = 1,
    .mcp3002_cfg = {.cs_gpio = 22, .spi_instance = 0}
    };

adc_driver_cfg_s battery_adc_1 = {
    .driver_type = ADC_DRIVER_HAL,
    .driver_instance  = 0,
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

bool cb_hoja_buttons_init()
{
    stdio_init_all();
    
    // Set up GPIO for input buttons
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

    return true;
}

void cb_hoja_read_buttons(button_data_s *data)
{
    // Keypad version
    gpio_put(PGPIO_SCAN_A, false);
    sleep_us(5);
    data->button_a  = !gpio_get(PGPIO_PUSH_C);
    data->button_b  = !gpio_get(PGPIO_PUSH_D);
    data->button_x  = !gpio_get(PGPIO_PUSH_A);
    data->button_y  = !gpio_get(PGPIO_PUSH_B);
    gpio_put(PGPIO_SCAN_A, true);

    gpio_put(PGPIO_SCAN_B, false);
    sleep_us(5);
    data->dpad_left     = !gpio_get(PGPIO_PUSH_D);
    data->dpad_right    = !gpio_get(PGPIO_PUSH_C);
    data->dpad_down     = !gpio_get(PGPIO_PUSH_B);
    data->dpad_up       = !gpio_get(PGPIO_PUSH_A);
    gpio_put(PGPIO_SCAN_B, true);

    gpio_put(PGPIO_SCAN_C, false);
    sleep_us(5);
    data->button_plus       = !gpio_get(PGPIO_PUSH_A);
    data->button_home       = !gpio_get(PGPIO_PUSH_B);
    data->button_capture    = !gpio_get(PGPIO_PUSH_D);
    data->button_minus      = !gpio_get(PGPIO_PUSH_C);
    gpio_put(PGPIO_SCAN_C, true);

    gpio_put(PGPIO_SCAN_D, false);
    sleep_us(5);
    data->trigger_r     = !gpio_get(PGPIO_PUSH_B);
    data->trigger_l     = !gpio_get(PGPIO_PUSH_D);
    data->trigger_zl    = !gpio_get(PGPIO_PUSH_A);
    data->trigger_zr    = !gpio_get(PGPIO_PUSH_C);
    gpio_put(PGPIO_SCAN_D, true);

    data->button_stick_right    = !gpio_get(PGPIO_BUTTON_RS);
    data->button_stick_left     = !gpio_get(PGPIO_BUTTON_LS);

    data->button_safemode   = !gpio_get(PGPIO_BUTTON_MODE);
    data->button_shipping   = data->button_stick_right && data->button_stick_left;
    data->button_sync       = data->button_safemode;
}

int main()
{
    hoja_init();
}
