#include "hoja.h" 
#include "input_shared_types.h"

#include "board_config.h" 
#include "main.h" 
#include "hardware/gpio.h" 
#include "pico/stdlib.h" 
#include "pico/bootrom.h" 

#include "hal/adc_hal.h"

adc_hal_driver_s joystick_driver_lx = {
    .gpio = 29
};

adc_hal_driver_s joystick_driver_ly = {
    .gpio = 28
};

void _local_setup_btn(uint32_t gpio)
{
    gpio_init(gpio);
    gpio_pull_up(gpio);
    gpio_set_dir(gpio, GPIO_IN);
}

void cb_hoja_init()
{
    stdio_init_all();
    
    _local_setup_btn(PGPIO_BTN_A);
    _local_setup_btn(PGPIO_BTN_B);
    _local_setup_btn(PGPIO_BTN_X);
    _local_setup_btn(PGPIO_BTN_Y);

    _local_setup_btn(PGPIO_BTN_L3);
    _local_setup_btn(PGPIO_BTN_R3);

    _local_setup_btn(PGPIO_BTN_NB1);
    _local_setup_btn(PGPIO_BTN_NB2);

    _local_setup_btn(PGPIO_BTN_RX);
    _local_setup_btn(PGPIO_BTN_RY);

    _local_setup_btn(PGPIO_BTN_TL);
    _local_setup_btn(PGPIO_BTN_TR);
    _local_setup_btn(PGPIO_BTN_TZL);
    _local_setup_btn(PGPIO_BTN_TZR);

    _local_setup_btn(PGPIO_BTN_BACK);
    _local_setup_btn(PGPIO_BTN_MENU);
    _local_setup_btn(PGPIO_BTN_HOME);
    _local_setup_btn(PGPIO_BTN_CAPTURE);

    _local_setup_btn(PGPIO_BTN_DUP);
    _local_setup_btn(PGPIO_BTN_DDOWN);
    _local_setup_btn(PGPIO_BTN_DLEFT);
    _local_setup_btn(PGPIO_BTN_DRIGHT);

    adc_hal_init(&joystick_driver_lx);
    adc_hal_init(&joystick_driver_ly);
}

void cb_hoja_read_joystick(uint16_t *input)
{
    adc_hal_read(&joystick_driver_lx);
    input[0] = joystick_driver_lx.output;
    
    adc_hal_read(&joystick_driver_ly);
    input[1] = 0xFFF - joystick_driver_ly.output;

    input[2] = 2048;
    input[3] = 2048;
}

void cb_hoja_read_input(mapper_input_s *input)
{
    bool *out = input->presses;

    out[INPUT_CODE_UP]      = !gpio_get(PGPIO_BTN_DUP);
    out[INPUT_CODE_DOWN]    = !gpio_get(PGPIO_BTN_DDOWN);
    out[INPUT_CODE_LEFT]    = !gpio_get(PGPIO_BTN_DLEFT);
    out[INPUT_CODE_RIGHT]   = !gpio_get(PGPIO_BTN_DRIGHT);

    out[INPUT_CODE_SOUTH]   = !gpio_get(PGPIO_BTN_A);
    out[INPUT_CODE_EAST]    = !gpio_get(PGPIO_BTN_B);
    out[INPUT_CODE_WEST]    = !gpio_get(PGPIO_BTN_X);
    out[INPUT_CODE_NORTH]   = !gpio_get(PGPIO_BTN_Y);

    out[INPUT_CODE_START]   = !gpio_get(PGPIO_BTN_MENU);
    out[INPUT_CODE_SELECT]  = !gpio_get(PGPIO_BTN_BACK);
    out[INPUT_CODE_HOME]    = !gpio_get(PGPIO_BTN_HOME);
    out[INPUT_CODE_SHARE]   = !gpio_get(PGPIO_BTN_CAPTURE);

    out[INPUT_CODE_LS]   = !gpio_get(PGPIO_BTN_L3);
    out[INPUT_CODE_MISC3] = !gpio_get(PGPIO_BTN_R3);

    out[INPUT_CODE_LP2]   = !gpio_get(PGPIO_BTN_RY);
    out[INPUT_CODE_RP2]   = !gpio_get(PGPIO_BTN_RX);

    out[INPUT_CODE_LP1] = !gpio_get(PGPIO_BTN_NB1);
    out[INPUT_CODE_RP1] = !gpio_get(PGPIO_BTN_NB2);
    
    out[INPUT_CODE_LB]   = !gpio_get(PGPIO_BTN_TL);
    out[INPUT_CODE_RB]   = !gpio_get(PGPIO_BTN_TR);
    out[INPUT_CODE_LT]   = !gpio_get(PGPIO_BTN_TZL);
    out[INPUT_CODE_RT]   = !gpio_get(PGPIO_BTN_TZR);
}

int main()
{
    hoja_init();
}
