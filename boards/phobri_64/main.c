#include "hoja.h"
#include "input/button.h"
#include "board_config.h"
#include "main.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h" 

adc_driver_cfg_s phobri_adc_1 = {
    .driver_type = ADC_DRIVER_ADS7142,
    .driver_instance  = 0,
    .ads7142_cfg = {.i2c_instance = 0}
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
    
    // Set up GPIO for input buttons
    _local_setup_btn(PGPIO_CLEFT);
    _local_setup_btn(PGPIO_CLEFT );
    _local_setup_btn(PGPIO_CUP   );
    _local_setup_btn(PGPIO_CRIGHT);
    _local_setup_btn(PGPIO_R     );
    _local_setup_btn(PGPIO_ZR    );
    _local_setup_btn(PGPIO_CDOWN );
    _local_setup_btn(PGPIO_A     );
    _local_setup_btn(PGPIO_B     );
    _local_setup_btn(PGPIO_DRIGHT);
    _local_setup_btn(PGPIO_DUP   );
    _local_setup_btn(PGPIO_DDOWN );
    _local_setup_btn(PGPIO_DLEFT );
    _local_setup_btn(PGPIO_L     );
    _local_setup_btn(PGPIO_ZL    );
    _local_setup_btn(PGPIO_START );

    return true;
}

void cb_hoja_read_buttons(button_data_s *data)
{
    data->button_south  = !gpio_get(PGPIO_A);
    data->button_east  = !gpio_get(PGPIO_B);

    data->button_north      = !gpio_get(PGPIO_CUP);
    data->button_west      = !gpio_get(PGPIO_CDOWN);
    data->trigger_l     = !gpio_get(PGPIO_CLEFT);
    data->trigger_r     = !gpio_get(PGPIO_CRIGHT);

    data->dpad_left     = !gpio_get(PGPIO_DLEFT);
    data->dpad_right    = !gpio_get(PGPIO_DRIGHT);
    data->dpad_down     = !gpio_get(PGPIO_DDOWN);
    data->dpad_up       = !gpio_get(PGPIO_DUP);

    data->button_plus   = !gpio_get(PGPIO_START);
    
    data->trigger_zr    = !gpio_get(PGPIO_R);
    data->button_minus  = !gpio_get(PGPIO_L);

    data->trigger_zl         = !gpio_get(PGPIO_ZL);
    data->button_stick_right = !gpio_get(PGPIO_ZR);
}

int main()
{
    hoja_init();
}
