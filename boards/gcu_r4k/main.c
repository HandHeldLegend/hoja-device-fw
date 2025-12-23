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

adc_mcp3002_driver_s joystick_driver_1 = {
    .cs_gpio = 1,
    .spi_instance = 0,
};

adc_mcp3002_driver_s joystick_driver_2 = {
    .cs_gpio = 9,
    .spi_instance = 0,
};

adc_hal_driver_s trigger_driver_1 = {
    .gpio = 28
};

adc_hal_driver_s trigger_driver_2 = {
    .gpio = 29
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

    mcp3002_init(&joystick_driver_1);
    mcp3002_init(&joystick_driver_2);
    adc_hal_init(&trigger_driver_1);
    adc_hal_init(&trigger_driver_2);
}

#define BUTTON_SLEEP_US 15

void cb_hoja_read_input(mapper_input_s *input)
{
    adc_hal_read(&trigger_driver_1);
    adc_hal_read(&trigger_driver_2);

    bool *out = input->presses;

    gpio_put(PGPIO_SCAN_A, false);
    sleep_us(BUTTON_SLEEP_US);
    out[INPUT_CODE_UP] = !gpio_get(PGPIO_PUSH_H);
    out[INPUT_CODE_RS] = !gpio_get(PGPIO_PUSH_G);
    out[INPUT_CODE_START] = !gpio_get(PGPIO_PUSH_I);
    gpio_put(PGPIO_SCAN_A, true);

    gpio_put(PGPIO_SCAN_B, false);
    sleep_us(BUTTON_SLEEP_US);
    out[INPUT_CODE_DOWN] = !gpio_get(PGPIO_PUSH_H);
    out[INPUT_CODE_SOUTH] = !gpio_get(PGPIO_PUSH_G);
    out[INPUT_CODE_SELECT] = !gpio_get(PGPIO_PUSH_I);
    gpio_put(PGPIO_SCAN_B, true);

    gpio_put(PGPIO_SCAN_C, false);
    sleep_us(BUTTON_SLEEP_US);
    out[INPUT_CODE_LEFT]     = !gpio_get(PGPIO_PUSH_H);
    out[INPUT_CODE_EAST]      = !gpio_get(PGPIO_PUSH_G);
    out[INPUT_CODE_HOME]   = !gpio_get(PGPIO_PUSH_I);
    gpio_put(PGPIO_SCAN_C, true);

    gpio_put(PGPIO_SCAN_D, false);
    sleep_us(BUTTON_SLEEP_US);

    out[INPUT_CODE_LB] = !gpio_get(PGPIO_PUSH_H);
    out[INPUT_CODE_NORTH] = !gpio_get(PGPIO_PUSH_G);
    out[INPUT_CODE_SHARE] = !gpio_get(PGPIO_PUSH_I);
    gpio_put(PGPIO_SCAN_D, true);

    gpio_put(PGPIO_SCAN_E, false);
    sleep_us(BUTTON_SLEEP_US);
    out[INPUT_CODE_LS] = !gpio_get(PGPIO_PUSH_H);
    out[INPUT_CODE_RB] = !gpio_get(PGPIO_PUSH_G);
    out[INPUT_CODE_RIGHT] = !gpio_get(PGPIO_PUSH_I);
    gpio_put(PGPIO_SCAN_E, true);

    gpio_put(PGPIO_SCAN_F, false);
    sleep_us(BUTTON_SLEEP_US);
    out[INPUT_CODE_LT]    = !gpio_get(PGPIO_PUSH_H);
    out[INPUT_CODE_RT]    = !gpio_get(PGPIO_PUSH_G);
    out[INPUT_CODE_WEST]      = !gpio_get(PGPIO_PUSH_I);
    gpio_put(PGPIO_SCAN_F, true);

    input->button_shipping = !gpio_get(PGPIO_BUTTON_MODE);
    out[INPUT_CODE_MISC3] = input->button_shipping;
    input->button_sync = (out[INPUT_CODE_START] > 0);

    input->inputs[INPUT_CODE_LT_ANALOG] = trigger_driver_2.output;
    input->inputs[INPUT_CODE_RT_ANALOG] = trigger_driver_1.output;

    // DEBUG
    if(out[INPUT_CODE_SHARE]) sys_hal_bootloader();
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
    hoja_init();
}
