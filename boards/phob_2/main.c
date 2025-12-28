#include "hoja.h"
#include "input_shared_types.h"
#include "board_config.h"
#include "main.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h" 

#include "drivers/adc/mcp3002.h"
#include "hal/adc_hal.h"

adc_hal_driver_s trigger_driver_r = {
    .gpio = 26
};

adc_hal_driver_s trigger_driver_l = {
    .gpio = 27
};

adc_mcp3002_driver_s joystick_driver_l = {
    .cs_gpio = 24,
    .spi_instance = 0,
};

adc_mcp3002_driver_s joystick_driver_r = {
    .cs_gpio = 23,
    .spi_instance = 0,
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

void cb_hoja_init()
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

    mcp3002_init(&joystick_driver_l);
    mcp3002_init(&joystick_driver_r);
    adc_hal_init(&trigger_driver_r);
    adc_hal_init(&trigger_driver_l);
}

void cb_hoja_read_input(mapper_input_s *input)
{
    adc_hal_read(&trigger_driver_r);
    adc_hal_read(&trigger_driver_l);

    bool *out = input->presses;
    
    static bool first = false;

    out[INPUT_CODE_SOUTH] = !gpio_get(PGPIO_BTN_A);
    out[INPUT_CODE_WEST]  = !gpio_get(PGPIO_BTN_B);
    out[INPUT_CODE_EAST]  = !gpio_get(PGPIO_BTN_X);
    out[INPUT_CODE_NORTH] = !gpio_get(PGPIO_BTN_Y);

    out[INPUT_CODE_LEFT]  = !gpio_get(PGPIO_BTN_DLEFT);
    out[INPUT_CODE_RIGHT] = !gpio_get(PGPIO_BTN_DRIGHT);
    out[INPUT_CODE_DOWN]  = !gpio_get(PGPIO_BTN_DDOWN);
    out[INPUT_CODE_UP]    = !gpio_get(PGPIO_BTN_DUP);

    out[INPUT_CODE_START]   = !gpio_get(PGPIO_BTN_START);

    out[INPUT_CODE_RB]   = !gpio_get(PGPIO_BTN_ZR);
    out[INPUT_CODE_LB]   = !gpio_get(PGPIO_BTN_ZL);
    out[INPUT_CODE_LT]   = !gpio_get(PGPIO_BTN_L);
    out[INPUT_CODE_RT]   = !gpio_get(PGPIO_BTN_R);

    if(!first)
    {
        if(out[INPUT_CODE_START])
        {
            out[INPUT_CODE_LB] = true;
            first = true;
        }
    }

    input->inputs[INPUT_CODE_LT_ANALOG] = trigger_driver_l.output;
    input->inputs[INPUT_CODE_RT_ANALOG] = trigger_driver_r.output;
}

void cb_hoja_read_joystick(uint16_t *input)
{
    mcp3002_read(&joystick_driver_l);
    mcp3002_read(&joystick_driver_r);

    input[0] = joystick_driver_l.output_ch_0;
    input[1] = joystick_driver_l.output_ch_1;

    input[2] = joystick_driver_r.output_ch_0;
    input[3] = joystick_driver_r.output_ch_1;
}

int main()
{
    hoja_init();
}
