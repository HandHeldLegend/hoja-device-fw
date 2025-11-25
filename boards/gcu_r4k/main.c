#include "hoja.h" 
#include "input/button.h" 
#include "utilities/static_config.h"
#include "board_config.h" 
#include "main.h" 
#include "hardware/gpio.h" 
#include "pico/stdlib.h" 
#include "pico/bootrom.h" 

adc_driver_cfg_s gcuk_adc_1 = {
    .driver_type = ADC_DRIVER_MCP3002,
    .driver_instance  = 0,
    .mcp3002_cfg = {.cs_gpio = 1, .spi_instance = 0}
    };

adc_driver_cfg_s gcuk_adc_2 = {
    .driver_type = ADC_DRIVER_MCP3002,
    .driver_instance  = 1,
    .mcp3002_cfg = {.cs_gpio = 9, .spi_instance = 0}
    };

adc_driver_cfg_s user_adc_hal = {
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
    _local_setup_scan(PGPIO_SCAN_E);
    _local_setup_scan(PGPIO_SCAN_F);

    _local_setup_push(PGPIO_PUSH_G);
    _local_setup_push(PGPIO_PUSH_H);
    _local_setup_push(PGPIO_PUSH_I);

    return true;
}

#define BUTTON_SLEEP_US 15

void cb_hoja_read_inputs()
{

}

void cb_hoja_read_hover()
{

}

void cb_hoja_read_joysticks()
{

}

void cb_hoja_read_buttons(button_data_s *data)
{
    gpio_put(PGPIO_SCAN_A, false);
    sleep_us(BUTTON_SLEEP_US);
    data->dpad_up               = !gpio_get(PGPIO_PUSH_H);
    data->button_stick_right    = !gpio_get(PGPIO_PUSH_G);
    data->button_plus           = !gpio_get(PGPIO_PUSH_I);
    gpio_put(PGPIO_SCAN_A, true);

    gpio_put(PGPIO_SCAN_B, false);
    sleep_us(BUTTON_SLEEP_US);
    data->dpad_down = !gpio_get(PGPIO_PUSH_H);
    data->button_south = !gpio_get(PGPIO_PUSH_G);
    data->button_minus = !gpio_get(PGPIO_PUSH_I);
    gpio_put(PGPIO_SCAN_B, true);

    gpio_put(PGPIO_SCAN_C, false);
    sleep_us(BUTTON_SLEEP_US);
    data->dpad_left     = !gpio_get(PGPIO_PUSH_H);
    data->button_east      = !gpio_get(PGPIO_PUSH_G);
    data->button_home   = !gpio_get(PGPIO_PUSH_I);
    gpio_put(PGPIO_SCAN_C, true);

    gpio_put(PGPIO_SCAN_D, false);
    sleep_us(BUTTON_SLEEP_US);

    bool l_pressed = !gpio_get(PGPIO_PUSH_H);
    static uint8_t l_bounced = 0;
    l_bounced = ((l_bounced << 1) | (l_pressed & 1)) & 0xFF;

    data->trigger_l = (l_bounced > 0) ? true : false;

    data ->button_north     = !gpio_get(PGPIO_PUSH_G);
    data->button_capture = !gpio_get(PGPIO_PUSH_I);
    gpio_put(PGPIO_SCAN_D, true);

    gpio_put(PGPIO_SCAN_E, false);
    sleep_us(BUTTON_SLEEP_US);
    data->button_stick_left = !gpio_get(PGPIO_PUSH_H);

    bool r_pressed = !gpio_get(PGPIO_PUSH_G);
    static uint8_t r_bounced = 0;
    r_bounced = ((r_bounced << 1) | (r_pressed & 1)) & 0xFF;

    data->trigger_r = (r_bounced > 0) ? true : false;

    data->dpad_right        = !gpio_get(PGPIO_PUSH_I);
    gpio_put(PGPIO_SCAN_E, true);

    gpio_put(PGPIO_SCAN_F, false);
    sleep_us(BUTTON_SLEEP_US);
    data->trigger_zl    = !gpio_get(PGPIO_PUSH_H);
    data->trigger_zr    = !gpio_get(PGPIO_PUSH_G);
    data->button_west      = !gpio_get(PGPIO_PUSH_I);
    gpio_put(PGPIO_SCAN_F, true);

    data->button_shipping = !gpio_get(PGPIO_BUTTON_MODE);

    //data->button_sync = data->button_plus;
}

int main()
{
    hoja_init();
}
