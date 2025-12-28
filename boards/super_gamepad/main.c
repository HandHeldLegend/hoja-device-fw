#include "hoja.h"

#include "input_shared_types.h"
#include "board_config.h"
#include "main.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h" 

#include "hal/adc_hal.h"

//adc_driver_cfg_s battery_adc_1 = {
//    .driver_type = ADC_DRIVER_HAL,
//    .driver_instance  = 0,
//    };

adc_hal_driver_s battery_adc_1 = {
    .gpio = 29
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

    out[INPUT_CODE_SELECT] =!gpio_get(PGPIO_BUTTON_SELECT);

    if(out[INPUT_CODE_START] && out[INPUT_CODE_SELECT])
    {
        input->button_sync   = true;
        out[INPUT_CODE_SELECT]  = false;
        input->button_shipping = false;
    }
    else if(out[INPUT_CODE_SELECT])
    {
        input->button_sync   = false;
        input->button_shipping = true;
    }
    else 
    {
        input->button_sync       = false;
        input->button_shipping   = false;
    }

    if(hoja_get_status().gamepad_mode != GAMEPAD_MODE_SNES)
    {
        if(out[INPUT_CODE_SELECT] && out[INPUT_CODE_RB])
        {
            out[INPUT_CODE_SELECT]  = false;
            out[INPUT_CODE_RB]     = false;
            input->button_sync   = false;
            input->button_shipping   = false;
            out[INPUT_CODE_HOME]       = true;
        }
        else out[INPUT_CODE_HOME] = false;

        if(out[INPUT_CODE_SELECT] && out[INPUT_CODE_LB])
        {
            out[INPUT_CODE_SELECT]  = false;
            out[INPUT_CODE_LB]     = false;
            input->button_sync   = false;
            input->button_shipping   = false;
            out[INPUT_CODE_SHARE]    = true;
        }
        else out[INPUT_CODE_SHARE] = false;
    }
}

int main()
{
    hoja_init();
}
