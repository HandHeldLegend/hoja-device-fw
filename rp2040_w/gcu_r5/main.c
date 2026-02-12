#include "hoja.h" 
#include "board_config.h" 
#include "main.h" 
#include "hardware/gpio.h" 
#include "pico/stdlib.h" 
#include "pico/bootrom.h"
#include "hal/sys_hal.h" 
#include "hal/adc_hal.h"
#include "drivers/mux/tmux1204.h"

#include "utilities/settings.h"

mux_tmux1204_driver_s mux_driver = {
    .a0_gpio=4, .a1_gpio=5\
};

adc_hal_driver_s joystick_driver = {
    .gpio = 29
};

adc_hal_driver_s trigger_driver_l = {
    .gpio = 27
};

adc_hal_driver_s trigger_driver_r = {
    .gpio = 26
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

    tmux1204_init(&mux_driver);
    adc_hal_init(&trigger_driver_l);
    adc_hal_init(&trigger_driver_r);
    adc_hal_init(&joystick_driver);
}

#define BUTTON_SLEEP_US 15

void cb_hoja_read_input(mapper_input_s *input)
{
    adc_hal_read(&trigger_driver_l);
    adc_hal_read(&trigger_driver_r);

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

    input->inputs[INPUT_CODE_LT_ANALOG] = 0xFFF - trigger_driver_l.output;
    input->inputs[INPUT_CODE_RT_ANALOG] = 0xFFF - trigger_driver_r.output;

    const uint16_t threshold = 500;

    if(input->inputs[INPUT_CODE_LT_ANALOG] > (hover_config->config[INPUT_CODE_LT_ANALOG].max + threshold))
    {
        out[INPUT_CODE_LT] |= 1;
    }
    else out[INPUT_CODE_LT] |= 0;

    if(input->inputs[INPUT_CODE_RT_ANALOG] > (hover_config->config[INPUT_CODE_RT_ANALOG].max + threshold))
    {
        out[INPUT_CODE_RT] |= 1;
    }
    else out[INPUT_CODE_RT] |= 0;

    // DEBUG
    //if(out[INPUT_CODE_SHARE]) sys_hal_bootloader();
}

bool cb_hoja_boot(boot_input_s *boot)
{
    mapper_input_s input;
    cb_hoja_read_input(&input);
    bool *out = input.presses;

    // Set default transport mode
    boot->gamepad_transport = GAMEPAD_TRANSPORT_AUTO;

    uint8_t a = out[INPUT_CODE_SOUTH] << 7;
    uint8_t b = out[INPUT_CODE_WEST] << 6;
    uint8_t x = out[INPUT_CODE_EAST] << 5;
    uint8_t y = out[INPUT_CODE_NORTH] << 4;
    uint8_t u = out[INPUT_CODE_UP] << 3;
    uint8_t d = out[INPUT_CODE_DOWN] << 2;
    uint8_t l = out[INPUT_CODE_LEFT] << 1;
    uint8_t r = out[INPUT_CODE_RIGHT];

    uint8_t mask = (a|b|x|y|u|d|l|r);

    switch(mask)
    {
        // SwPro
        default:
        boot->gamepad_mode = GAMEPAD_MODE_LOAD;
        break;

        // A
        case 0b10000000:
        boot->gamepad_mode = GAMEPAD_MODE_SWPRO;
        break;

        // B
        case 0b1000000:
        boot->gamepad_mode = GAMEPAD_MODE_SINPUT;
        break;

        // X
        case 0b100000:
        boot->gamepad_mode = GAMEPAD_MODE_XINPUT;
        break;

        // Y
        case 0b10000:
        boot->gamepad_mode = GAMEPAD_MODE_GCUSB;
        break;

        // U
        case 0b1000:
        boot->gamepad_mode = GAMEPAD_MODE_LOAD;
        break;

        // D
        case 0b100:
        boot->gamepad_mode = GAMEPAD_MODE_N64;
        break;

        // L
        case 0b10:
        boot->gamepad_mode = GAMEPAD_MODE_SNES;
        break;

        // R
        case 0b1:
        boot->gamepad_mode = GAMEPAD_MODE_GAMECUBE;
        break;
    }

    bool lt = out[INPUT_CODE_LB];
    bool rt = out[INPUT_CODE_RB];
    bool plus = out[INPUT_CODE_START];

    if(plus&&lt)
    {
        boot->bootloader = true;
        return true;
    }

    if(plus)
    {
        boot->pairing_mode = true;
    }

    if(rt)
    {
        switch(boot->gamepad_mode)
        {
            case GAMEPAD_MODE_GCUSB:
            case GAMEPAD_MODE_GAMECUBE:
            case GAMEPAD_MODE_SINPUT:
            case GAMEPAD_MODE_N64:
            boot->gamepad_transport = GAMEPAD_TRANSPORT_WLAN;
            break;

            default:
            boot->gamepad_transport = GAMEPAD_TRANSPORT_AUTO;
        }
    }

    return true;
}

void cb_hoja_read_joystick(uint16_t *input)
{
    tmux1204_switch_channel(&mux_driver, 2);
    adc_hal_read(&joystick_driver);
    input[0] = joystick_driver.output;
    
    tmux1204_switch_channel(&mux_driver, 0);
    adc_hal_read(&joystick_driver);
    input[1] = joystick_driver.output;

    tmux1204_switch_channel(&mux_driver, 3);
    adc_hal_read(&joystick_driver);
    input[2] = joystick_driver.output;

    tmux1204_switch_channel(&mux_driver, 1);
    adc_hal_read(&joystick_driver);
    input[3] = joystick_driver.output;
}

int main()
{
    hoja_init();
}