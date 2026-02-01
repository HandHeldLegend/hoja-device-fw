#include "hoja.h"
#include "input_shared_types.h"

#include "board_config.h"
#include "main.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h" 

#include "drivers/adc/mcp3002.h"
#include "hal/adc_hal.h"

adc_hal_driver_s battery_adc = {
    .gpio = 26
};

adc_mcp3002_driver_s joystick_driver_l = {
    .cs_gpio = 16,
    .spi_instance = 0,
};

adc_mcp3002_driver_s joystick_driver_r = {
    .cs_gpio = 22,
    .spi_instance = 0,
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

void cb_hoja_read_joystick(uint16_t *input)
{
    mcp3002_read(&joystick_driver_l);
    mcp3002_read(&joystick_driver_r);

    input[0] = joystick_driver_l.output_ch_0;
    input[1] = joystick_driver_l.output_ch_1;

    input[2] = joystick_driver_r.output_ch_0;
    input[3] = joystick_driver_r.output_ch_1;
}

uint16_t cb_hoja_read_battery()
{
    adc_hal_read(&battery_adc);
    return battery_adc.output;
}

void cb_hoja_init()
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

    mcp3002_init(&joystick_driver_l);
    mcp3002_init(&joystick_driver_r);
    adc_hal_init(&battery_adc);
}

#define SCAN_TIME_US 15

void cb_hoja_read_input(mapper_input_s *input)
{
    bool *out = input->presses;

     // Keypad version
    gpio_put(PGPIO_SCAN_A, false);
    sleep_us(SCAN_TIME_US);
    out[INPUT_CODE_EAST]  = !gpio_get(PGPIO_PUSH_C);
    out[INPUT_CODE_SOUTH] = !gpio_get(PGPIO_PUSH_D);
    out[INPUT_CODE_NORTH] = !gpio_get(PGPIO_PUSH_A);
    out[INPUT_CODE_WEST]  = !gpio_get(PGPIO_PUSH_B);
    gpio_put(PGPIO_SCAN_A, true);

    gpio_put(PGPIO_SCAN_B, false);
    sleep_us(SCAN_TIME_US);
    out[INPUT_CODE_LEFT]    = !gpio_get(PGPIO_PUSH_D);
    out[INPUT_CODE_RIGHT]   = !gpio_get(PGPIO_PUSH_C);
    out[INPUT_CODE_DOWN]    = !gpio_get(PGPIO_PUSH_B);
    out[INPUT_CODE_UP]      = !gpio_get(PGPIO_PUSH_A);
    gpio_put(PGPIO_SCAN_B, true);

    gpio_put(PGPIO_SCAN_C, false);
    sleep_us(SCAN_TIME_US);
    out[INPUT_CODE_START]   = !gpio_get(PGPIO_PUSH_A);
    out[INPUT_CODE_HOME]    = !gpio_get(PGPIO_PUSH_B);
    out[INPUT_CODE_SHARE]   = !gpio_get(PGPIO_PUSH_D);
    out[INPUT_CODE_SELECT]  = !gpio_get(PGPIO_PUSH_C);
    gpio_put(PGPIO_SCAN_C, true);

    gpio_put(PGPIO_SCAN_D, false);
    sleep_us(SCAN_TIME_US);
    out[INPUT_CODE_RB]   = !gpio_get(PGPIO_PUSH_B);
    out[INPUT_CODE_LB]   = !gpio_get(PGPIO_PUSH_D);
    out[INPUT_CODE_LT]   = !gpio_get(PGPIO_PUSH_A);
    out[INPUT_CODE_RT]   = !gpio_get(PGPIO_PUSH_C);
    gpio_put(PGPIO_SCAN_D, true);

    out[INPUT_CODE_RS]  = !gpio_get(PGPIO_BUTTON_RS);
    out[INPUT_CODE_LS]  = !gpio_get(PGPIO_BUTTON_LS);

    input->button_safemode   = !gpio_get(PGPIO_BUTTON_MODE);
    input->button_shipping   = out[INPUT_CODE_RS] && out[INPUT_CODE_LS];
    input->button_sync       = input->button_safemode;
}

int main()
{
    hoja_init();
}
