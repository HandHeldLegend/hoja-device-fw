#include "hoja.h"
#include "input/mapper.h"
#include "board_config.h"
#include "hardware/gpio.h"
#include "pico/stdlib.h"
#include "pico/bootrom.h" 

#include "hal/adc_hal.h"

#define PGPIO_SCAN_G1   18u
#define PGPIO_SCAN_G2   7u
#define PGPIO_SCAN_G3   17u
#define PGPIO_SCAN_G4   6u
#define PGPIO_SCAN_G5   37u

#define PGPIO_BTN_SELECT 25u
#define PGPIO_BTN_SHARE  24u
#define PGPIO_BTN_START  4u
#define PGPIO_BTN_HOME   5u
#define PGPIO_BTN_MISC   0u

// G3 Group
#define A1_OUT 3
#define A2_OUT 2
#define A3_OUT 1
#define A4_OUT 0

// G2 Group
#define A5_OUT 0
#define A6_OUT 1
#define A7_OUT 2
#define A8_OUT 3

// G1 Group
#define B1_OUT 3
#define B2_OUT 2
#define B3_OUT 1
#define B4_OUT 0

// G2 Group
#define B5_OUT 7
#define B6_OUT 6
#define B7_OUT 5
#define B8_OUT 4

// G1 Group
#define C1_OUT 4
#define C2_OUT 5
#define C3_OUT 6
#define C4_OUT 7

// G4 Group
#define C5_OUT 7
#define C6_OUT 6
#define C7_OUT 5
#define C8_OUT 4

// G5 Group
#define D1_OUT 7
#define D2_OUT 6

adc_hal_driver_s hall_driver_0 = {
    .gpio = 40};
adc_hal_driver_s hall_driver_1 = {
    .gpio = 41};
adc_hal_driver_s hall_driver_2 = {
    .gpio = 42};
adc_hal_driver_s hall_driver_3 = {
    .gpio = 43};
adc_hal_driver_s hall_driver_4 = {
    .gpio = 44};
adc_hal_driver_s hall_driver_5 = {
    .gpio = 45};
adc_hal_driver_s hall_driver_6 = {
    .gpio = 46};
adc_hal_driver_s hall_driver_7 = {
    .gpio = 47};

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

void cb_hoja_read_joystick(uint16_t *input)
{
    // Unused
}

void cb_hoja_init()
{
    stdio_init_all();
    
    // Set up GPIO for input buttons
    _local_setup_btn(PGPIO_BTN_SELECT);
    _local_setup_btn(PGPIO_BTN_SHARE);
    _local_setup_btn(PGPIO_BTN_START);
    _local_setup_btn(PGPIO_BTN_HOME);
    _local_setup_btn(PGPIO_BTN_MISC);

    _local_setup_push(PGPIO_SCAN_G1);
    _local_setup_push(PGPIO_SCAN_G2);
    _local_setup_push(PGPIO_SCAN_G3);
    _local_setup_push(PGPIO_SCAN_G4);
    _local_setup_push(PGPIO_SCAN_G5);

    adc_hal_init(&hall_driver_0);
    adc_hal_init(&hall_driver_1);
    adc_hal_init(&hall_driver_2);
    adc_hal_init(&hall_driver_3);
    adc_hal_init(&hall_driver_4);
    adc_hal_init(&hall_driver_5);
    adc_hal_init(&hall_driver_6);
    adc_hal_init(&hall_driver_7);
}

#define SCAN_TIME_US 25

void cb_hoja_read_input(mapper_input_s *input)
{
    bool *out = input->presses;
    

    sleep_us(SCAN_TIME_US);
    out[INPUT_CODE_START]  = !gpio_get(PGPIO_BTN_START);
    out[INPUT_CODE_SELECT]  = !gpio_get(PGPIO_BTN_SELECT);
    out[INPUT_CODE_HOME]  = !gpio_get(PGPIO_BTN_HOME);
    out[INPUT_CODE_SHARE]  = !gpio_get(PGPIO_BTN_SHARE);
    out[INPUT_CODE_POWER]  = !gpio_get(PGPIO_BTN_MISC);

    gpio_put(PGPIO_SCAN_G1, false);
    sleep_us(SCAN_TIME_US);
    adc_hal_read(&hall_driver_0);
    adc_hal_read(&hall_driver_1);
    adc_hal_read(&hall_driver_2);
    adc_hal_read(&hall_driver_3);
    adc_hal_read(&hall_driver_4);
    adc_hal_read(&hall_driver_5);
    adc_hal_read(&hall_driver_6);
    adc_hal_read(&hall_driver_7);

    input->inputs[8]  = hall_driver_3.output; // B1
    input->inputs[9]  = hall_driver_2.output; // B2
    input->inputs[10] = hall_driver_1.output; // B3
    input->inputs[12] = hall_driver_0.output; // B4

    input->inputs[26] = hall_driver_4.output; // C1
    input->inputs[27] = hall_driver_5.output; // C2
    input->inputs[28] = hall_driver_6.output; // C3
    input->inputs[29] = hall_driver_7.output; // C4
    gpio_put(PGPIO_SCAN_G1, true);


    gpio_put(PGPIO_SCAN_G2, false);
    sleep_us(SCAN_TIME_US);
    adc_hal_read(&hall_driver_0);
    adc_hal_read(&hall_driver_1);
    adc_hal_read(&hall_driver_2);
    adc_hal_read(&hall_driver_3);
    adc_hal_read(&hall_driver_4);
    adc_hal_read(&hall_driver_5);
    adc_hal_read(&hall_driver_6);
    adc_hal_read(&hall_driver_7);

    input->inputs[4] = hall_driver_0.output; // A5
    input->inputs[5] = hall_driver_1.output; // A6
    input->inputs[6] = hall_driver_2.output; // A7
    input->inputs[7] = hall_driver_3.output; // A8

    input->inputs[14] = hall_driver_7.output; // B5
    input->inputs[15] = hall_driver_6.output; // B6
    input->inputs[16] = hall_driver_5.output; // B7
    input->inputs[17] = hall_driver_4.output; // B8
    gpio_put(PGPIO_SCAN_G2, true);

    // G3/G4 grouped together
    gpio_put(PGPIO_SCAN_G3, false);
    gpio_put(PGPIO_SCAN_G4, false);
    sleep_us(SCAN_TIME_US);

    adc_hal_read(&hall_driver_0);
    adc_hal_read(&hall_driver_1);
    adc_hal_read(&hall_driver_2);
    adc_hal_read(&hall_driver_3);
    adc_hal_read(&hall_driver_4);
    adc_hal_read(&hall_driver_5);
    adc_hal_read(&hall_driver_6);
    adc_hal_read(&hall_driver_7);

    input->inputs[30] = hall_driver_7.output; // C5
    input->inputs[31] = hall_driver_6.output; // C6
    input->inputs[32] = hall_driver_5.output; // C7
    input->inputs[33] = hall_driver_4.output; // C8

    input->inputs[0] = hall_driver_3.output; // A1
    input->inputs[1] = hall_driver_2.output; // A2
    input->inputs[2] = hall_driver_1.output; // A3
    input->inputs[3] = hall_driver_0.output; // A4
    
    gpio_put(PGPIO_SCAN_G3, true);
    gpio_put(PGPIO_SCAN_G4, true);

    gpio_put(PGPIO_SCAN_G5, false);
    sleep_us(SCAN_TIME_US);
    adc_hal_read(&hall_driver_6);
    adc_hal_read(&hall_driver_7);

    input->inputs[34] = hall_driver_7.output; // D1
    input->inputs[35] = hall_driver_6.output; // D2
    gpio_put(PGPIO_SCAN_G5, true);

}

int main()
{
    hoja_init();
}
