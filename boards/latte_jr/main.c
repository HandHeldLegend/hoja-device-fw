#include "hoja.h" 
#include "utilities/static_config.h"
#include "hal/sys_hal.h"

#include "input_shared_types.h"

#include "board_config.h" 
#include "main.h" 
#include "hardware/gpio.h" 
#include "pico/stdlib.h" 
#include "pico/bootrom.h" 

#include "drivers/adc/mcp3002.h"
#include "hal/adc_hal.h"

adc_hal_driver_s input_driver_1 = {
    .gpio = 26
};

adc_hal_driver_s input_driver_2 = {
    .gpio = 27
};

adc_hal_driver_s input_driver_3 = {
    .gpio = 28
};

adc_hal_driver_s input_driver_4 = {
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
    gpio_set_dir(gpio, GPIO_OUT);
    gpio_put(gpio, true);
}

void cb_hoja_init()
{
    stdio_init_all();
    
    _local_setup_push(PGPIO_EN_DPAD);
    _local_setup_push(PGPIO_EN_TRIGGER_L);
    _local_setup_push(PGPIO_EN_SEWN);
    _local_setup_push(PGPIO_EN_TRIGGER_R);
    // Set up GPIO for input buttons
    _local_setup_btn(PGPIO_BTN_POWER);

    _local_setup_btn(PGPIO_BTN_GL);
    _local_setup_btn(PGPIO_BTN_MINUS);
    _local_setup_btn(PGPIO_BTN_CAPTURE);
    _local_setup_btn(PGPIO_BTN_HOME);
    _local_setup_btn(PGPIO_BTN_PLUS);
    _local_setup_btn(PGPIO_BTN_GR);

    adc_hal_init(&input_driver_1);
    adc_hal_init(&input_driver_2);
    adc_hal_init(&input_driver_3);
    adc_hal_init(&input_driver_4);
}

#define BUTTON_SLEEP_US 64

void cb_hoja_read_input(mapper_input_s *input)
{
    static uint8_t read_idx = 0;
    static uint16_t a_vals[36] = {0};

    bool *out = input->presses;
    uint16_t *aout = input->inputs;

    out[INPUT_CODE_LP1] = !gpio_get(PGPIO_BTN_GL);
    out[INPUT_CODE_SELECT] = !gpio_get(PGPIO_BTN_MINUS);
    out[INPUT_CODE_SHARE] = !gpio_get(PGPIO_BTN_CAPTURE);
    out[INPUT_CODE_HOME] = !gpio_get(PGPIO_BTN_HOME);
   
    out[INPUT_CODE_START] = !gpio_get(PGPIO_BTN_PLUS);
    if(!gpio_get(PGPIO_BTN_POWER)) sys_hal_bootloader();

    out[INPUT_CODE_RP1] = !gpio_get(PGPIO_BTN_GR);

    
    switch(read_idx)
    {
        case 1:
        // DPAD
        gpio_put(PGPIO_EN_DPAD, false);
        sleep_us(BUTTON_SLEEP_US);
        adc_hal_read(&input_driver_1);
        adc_hal_read(&input_driver_2);
        adc_hal_read(&input_driver_3);
        adc_hal_read(&input_driver_4);

        a_vals[INPUT_CODE_DOWN] = input_driver_1.output; // Down
        a_vals[INPUT_CODE_RIGHT] = input_driver_2.output; // Right
        a_vals[INPUT_CODE_LEFT] = input_driver_3.output; 
        a_vals[INPUT_CODE_UP] = input_driver_4.output; // Up
        gpio_put(PGPIO_EN_DPAD, true);
        break;


        case 2:
        // TRIGGERS R
        gpio_put(PGPIO_EN_TRIGGER_R, false);
        sleep_us(BUTTON_SLEEP_US);
        adc_hal_read(&input_driver_2);
        adc_hal_read(&input_driver_3);
        adc_hal_read(&input_driver_4);
        a_vals[INPUT_CODE_LB] = input_driver_4.output;
        a_vals[INPUT_CODE_RB] = input_driver_2.output;
        a_vals[INPUT_CODE_RT_ANALOG] = input_driver_3.output;
        gpio_put(PGPIO_EN_TRIGGER_R, true);
        break;

        case 3:
        // TRIGGER L
        gpio_put(PGPIO_EN_TRIGGER_L, false);
        sleep_us(BUTTON_SLEEP_US);
        adc_hal_read(&input_driver_2);
        a_vals[INPUT_CODE_LT_ANALOG] = input_driver_2.output;
        gpio_put(PGPIO_EN_TRIGGER_L, true);
        break;

        default:
        // SEWNs
        gpio_put(PGPIO_EN_SEWN, false);
        sleep_us(BUTTON_SLEEP_US);
        adc_hal_read(&input_driver_1);
        adc_hal_read(&input_driver_2);
        adc_hal_read(&input_driver_3);
        adc_hal_read(&input_driver_4);

        a_vals[INPUT_CODE_EAST] = input_driver_1.output; // East/2
        a_vals[INPUT_CODE_SOUTH] = input_driver_2.output; // South/1
        a_vals[INPUT_CODE_WEST] = input_driver_3.output; // West/4
        a_vals[INPUT_CODE_NORTH] = input_driver_4.output; // North/5
        gpio_put(PGPIO_EN_SEWN, true);
        break;
    }

    input->button_shipping = !gpio_get(PGPIO_BTN_POWER);
    out[INPUT_CODE_MISC3] = input->button_shipping;
    input->button_sync = (out[INPUT_CODE_START] > 0);

    for(int i = 0; i < 36; i++)
    {
        input->inputs[i] = a_vals[i];
    }

    read_idx++;
    if(read_idx > 3) read_idx = 0;
}

int main()
{
    hoja_init();
}
