#include "hoja.h"
#include "input/button.h"
#include "board_config.h"
#include "main.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h" 

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
    _local_setup_btn(PGPIO_BUTTON_SELECT);

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
    data->trigger_l       = !gpio_get(PGPIO_PUSH_C);
    data->trigger_r       = !gpio_get(PGPIO_PUSH_B);
    data->button_plus     = !gpio_get(PGPIO_PUSH_A);
    //
    gpio_put(PGPIO_SCAN_C, true);

    data->button_minus =!gpio_get(PGPIO_BUTTON_SELECT);


    if(data->button_plus && data->button_minus)
    {
        data->button_sync   = true;
        data->button_minus  = false;
        data->button_shipping = false;
    }
    else if(data->button_minus)
    {
        data->button_sync   = false;
        data->button_shipping = true;
    }
    else 
    {
        data->button_sync       = false;
        data->button_shipping   = false;
    }

    if(hoja_get_status().gamepad_mode != GAMEPAD_MODE_SNES)
    {
        if(data->button_minus && data->trigger_r)
        {
            data->button_minus  = false;
            data->trigger_r     = false;
            data->button_sync   = false;
            data->button_shipping   = false;
            data->button_home       = true;
        }
        else data->button_home = false;

        if(data->button_minus && data->trigger_l)
        {
            data->button_minus  = false;
            data->trigger_l     = false;
            data->button_sync   = false;
            data->button_shipping   = false;
            data->button_capture    = true;
        }
        else data->button_capture = false;
    }
}

int main()
{
    hoja_init();
}
