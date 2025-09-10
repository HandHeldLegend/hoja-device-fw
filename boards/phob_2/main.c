#include "hoja.h"
#include "input/button.h"
#include "board_config.h"
#include "main.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h" 

adc_driver_cfg_s gcp_adc_1 = {
    .driver_type = ADC_DRIVER_MCP3002,
    .driver_instance  = 0,
    .mcp3002_cfg = {.cs_gpio = 24, .spi_instance = 0}
    };

adc_driver_cfg_s gcp_adc_2 = {
    .driver_type = ADC_DRIVER_MCP3002,
    .driver_instance  = 1,
    .mcp3002_cfg = {.cs_gpio = 23, .spi_instance = 0}
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
    // Set up GPIO for input buttons
    _local_setup_btn(PGPIO_BTN_A);
    _local_setup_btn(PGPIO_BTN_B);
    _local_setup_btn(PGPIO_BTN_X);
    _local_setup_btn(PGPIO_BTN_Y);

    _local_setup_btn(PGPIO_BTN_START);
    _local_setup_btn(PGPIO_BTN_L);
    _local_setup_btn(PGPIO_BTN_R);
    _local_setup_btn(PGPIO_BTN_ZL);
    _local_setup_btn(PGPIO_BTN_ZR);

    _local_setup_btn(PGPIO_BTN_DUP);
    _local_setup_btn(PGPIO_BTN_DDOWN);
    _local_setup_btn(PGPIO_BTN_DLEFT);
    _local_setup_btn(PGPIO_BTN_DRIGHT);

    return true;
}

void cb_hoja_read_buttons(button_data_s *data)
{
    static bool first = false;

    data->button_south  = !gpio_get(PGPIO_BTN_A);
    data->button_west  = !gpio_get(PGPIO_BTN_B);
    data->button_east  = !gpio_get(PGPIO_BTN_X);
    data->button_north  = !gpio_get(PGPIO_BTN_Y);

    data->dpad_left     = !gpio_get(PGPIO_BTN_DLEFT);
    data->dpad_right    = !gpio_get(PGPIO_BTN_DRIGHT);
    data->dpad_down     = !gpio_get(PGPIO_BTN_DDOWN);
    data->dpad_up       = !gpio_get(PGPIO_BTN_DUP);

    data->button_plus   = !gpio_get(PGPIO_BTN_START);

    data->trigger_r     = !gpio_get(PGPIO_BTN_ZR);
    data->trigger_l     = !gpio_get(PGPIO_BTN_ZL);
    data->trigger_zl    = !gpio_get(PGPIO_BTN_L);
    data->trigger_zr    = !gpio_get(PGPIO_BTN_R);

    if(!first)
    {
        if(data->button_plus)
        {
            data->trigger_l = true;
            first = true;
        }
    }
}

int main()
{
    hoja_init();
}
