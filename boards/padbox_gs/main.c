#include "hoja.h" 
#include "input/button.h" 
#include "board_config.h" 
#include "main.h" 
#include "hardware/gpio.h" 
#include "pico/stdlib.h" 
#include "pico/bootrom.h" 

adc_driver_cfg_s user_adc_hal = {
    .driver_type = ADC_DRIVER_HAL,
    .driver_instance = 0,
    };

void _local_setup_btn(uint32_t gpio)
{
    gpio_init(gpio);
    gpio_pull_up(gpio);
    gpio_set_dir(gpio, GPIO_IN);
}

bool cb_hoja_buttons_init()
{
    stdio_init_all();
    
    _local_setup_btn(PGPIO_BTN_A);
    _local_setup_btn(PGPIO_BTN_B);
    _local_setup_btn(PGPIO_BTN_X);
    _local_setup_btn(PGPIO_BTN_Y);

    _local_setup_btn(PGPIO_BTN_SL);
    _local_setup_btn(PGPIO_BTN_SR);

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

    return true;
}

void cb_hoja_read_buttons(button_data_s *data)
{

    data->dpad_up       = !gpio_get(PGPIO_BTN_DUP);
    data->dpad_down     = !gpio_get(PGPIO_BTN_DDOWN);
    data->dpad_left     = !gpio_get(PGPIO_BTN_DLEFT);
    data->dpad_right    = !gpio_get(PGPIO_BTN_DRIGHT);

    data->button_a      = !gpio_get(PGPIO_BTN_A);
    data->button_b      = !gpio_get(PGPIO_BTN_B);
    data->button_x      = !gpio_get(PGPIO_BTN_X);
    data ->button_y     = !gpio_get(PGPIO_BTN_Y);

    data->button_plus   = !gpio_get(PGPIO_BTN_MENU);
    data->button_minus  = !gpio_get(PGPIO_BTN_BACK);
    data->button_home   = !gpio_get(PGPIO_BTN_HOME);
    data->button_capture = !gpio_get(PGPIO_BTN_CAPTURE);

    data->button_stick_left     = !gpio_get(PGPIO_BTN_SL);
    data->button_stick_right    = !gpio_get(PGPIO_BTN_SR);
    
    data->trigger_l     = !gpio_get(PGPIO_BTN_TL);
    data->trigger_r     = !gpio_get(PGPIO_BTN_TR);
    data->trigger_zl    = !gpio_get(PGPIO_BTN_TZL);
    data->trigger_zr    = !gpio_get(PGPIO_BTN_TZR);
    

}

int main()
{
    hoja_init();
}
