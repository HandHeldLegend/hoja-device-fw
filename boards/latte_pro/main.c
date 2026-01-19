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

adc_hal_driver_s joystick_driver_1 = {
    .gpio = 44
};

adc_hal_driver_s joystick_driver_2 = {
    .gpio = 45
};

adc_hal_driver_s joystick_driver_3 = {
    .gpio = 46
};

adc_hal_driver_s joystick_driver_4 = {
    .gpio = 47
};

adc_hal_driver_s input_driver_1 = {
    .gpio = 40
};

adc_hal_driver_s input_driver_2 = {
    .gpio = 41
};

adc_hal_driver_s input_driver_3 = {
    .gpio = 42
};

adc_hal_driver_s input_driver_4 = {
    .gpio = 43
};

uint64_t counter = 0;

void _local_setup_btn(uint32_t gpio)
{
    gpio_init(gpio);
    gpio_pull_up(gpio);
}

void _local_setup_push(uint32_t gpio)
{
    gpio_init(gpio);              // Reset the pad to GPIO function
    gpio_put(gpio, true);         // Set the latch to HIGH first
    gpio_set_dir(gpio, GPIO_OUT); // THEN enable output
    gpio_disable_pulls(gpio);
    gpio_set_drive_strength(gpio, GPIO_DRIVE_STRENGTH_8MA);
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
    _local_setup_btn(PGPIO_BTN_POWER2);
    _local_setup_btn(PGPIO_BTN_LS);
    _local_setup_btn(PGPIO_BTN_GL);
    _local_setup_btn(PGPIO_BTN_MINUS);
    _local_setup_btn(PGPIO_BTN_CAPTURE);
    _local_setup_btn(PGPIO_BTN_HOME);
    _local_setup_btn(PGPIO_BTN_PLUS);
    _local_setup_btn(PGPIO_BTN_GR);
    _local_setup_btn(PGPIO_BTN_RS);

    adc_hal_init(&joystick_driver_1);
    adc_hal_init(&joystick_driver_2);
    adc_hal_init(&joystick_driver_3);
    adc_hal_init(&joystick_driver_4);
    adc_hal_init(&input_driver_1);
    adc_hal_init(&input_driver_2);
    adc_hal_init(&input_driver_3);
    adc_hal_init(&input_driver_4);
}

#define BUTTON_SLEEP_US 16

bool cb_hoja_boot(boot_input_s *boot)
{
    uint16_t vals[4];
    int8_t mode_idx = -1;
    const uint16_t diff = 300;
    uint16_t baseline = 0;
    uint32_t avg = 0;
    uint8_t mode_mask = 0;

    // USB gamepad modes
    gpio_put(PGPIO_EN_SEWN, false);
    sleep_us(BUTTON_SLEEP_US);
    adc_hal_read(&input_driver_1);
    adc_hal_read(&input_driver_2);
    adc_hal_read(&input_driver_3);
    adc_hal_read(&input_driver_4);

    vals[0] = input_driver_1.output; // NORTH
    vals[1] = input_driver_2.output; // WEST
    vals[2] = input_driver_3.output; // EAST
    vals[3] = input_driver_4.output; // SOUTH
    gpio_put(PGPIO_EN_SEWN, true);

    avg = (vals[0] + vals[1] + vals[2] + vals[3])>>2;

    for(int i = 0; i < 4; i++)
    {
        if(vals[i] > (avg+diff))
        {
            mode_mask |= (1<<i);
        }
    }

    // Retro gamepad modes
    gpio_put(PGPIO_EN_DPAD, false);
    sleep_us(BUTTON_SLEEP_US);
    adc_hal_read(&input_driver_1);
    adc_hal_read(&input_driver_2);
    adc_hal_read(&input_driver_3);
    adc_hal_read(&input_driver_4);

    vals[0] = input_driver_1.output;  // Up
    vals[1] = input_driver_2.output;  // Right
    vals[2] = input_driver_3.output;  // Left
    vals[3] = input_driver_4.output;  // down
    gpio_put(PGPIO_EN_DPAD, true);

    avg = (vals[0] + vals[1] + vals[2] + vals[3])>>2;

    for(int i = 0; i < 4; i++)
    {
        if(vals[i] > (avg+diff))
        {
            mode_mask |= (1<<(i+4));
        }
    }

    switch(mode_mask)
    {
        case 0b1:
        mode_idx = GAMEPAD_MODE_XINPUT;
        break;

        case 0b10:
        mode_idx = GAMEPAD_MODE_GCUSB;
        break;

        case 0b100:
        mode_idx = GAMEPAD_MODE_SWPRO;
        break;

        case 0b1000:
        mode_idx = GAMEPAD_MODE_SINPUT;
        break;

        case 0b100000:
        mode_idx = GAMEPAD_MODE_GAMECUBE;
        break;

        case 0b1000000:
        mode_idx = GAMEPAD_MODE_SNES;
        break;

        case 0b10000000:
        mode_idx = GAMEPAD_MODE_N64;
        break;

        case 0b10000:
        default:
        break;
    }

    // Bootloader setting
    gpio_put(PGPIO_EN_TRIGGER_L, false);
    gpio_put(PGPIO_EN_TRIGGER_R, false);
    sleep_us(BUTTON_SLEEP_US);
    adc_hal_read(&input_driver_1);
    adc_hal_read(&input_driver_3);

    vals[0] = 0xFFF - input_driver_1.output; // RB
    vals[1] = 0xFFF - input_driver_3.output; // LB
    gpio_put(PGPIO_EN_TRIGGER_L, true);
    gpio_put(PGPIO_EN_TRIGGER_R, true);

    bool l_only = false;

    if(vals[1] > vals[0])
    {
        l_only = (vals[1]-vals[0]) > 2000;
    }

    bool start = !gpio_get(PGPIO_BTN_PLUS);

    boot->bootloader = (start && l_only);
    boot->pairing_mode = start;
    boot->gamepad_mode = mode_idx;
}

void cb_hoja_read_input(mapper_input_s *input)
{

    static uint8_t read_idx = 0;
    static uint16_t a_vals[36] = {0};

    bool *out = input->presses;
    uint16_t *aout = input->inputs;
    out[INPUT_CODE_LS] = !gpio_get(PGPIO_BTN_LS);
    out[INPUT_CODE_LP1] = !gpio_get(PGPIO_BTN_GL);
    out[INPUT_CODE_SELECT] = !gpio_get(PGPIO_BTN_MINUS);
    out[INPUT_CODE_SHARE] = !gpio_get(PGPIO_BTN_CAPTURE);
    out[INPUT_CODE_HOME] = !gpio_get(PGPIO_BTN_HOME);
   
    out[INPUT_CODE_START] = !gpio_get(PGPIO_BTN_PLUS);
    out[INPUT_CODE_RP1] = !gpio_get(PGPIO_BTN_GR);
    out[INPUT_CODE_RS] = !gpio_get(PGPIO_BTN_RS);

    switch(read_idx)
    {
        default:
        // SEWNs
        gpio_put(PGPIO_EN_SEWN, false);
        sleep_us(BUTTON_SLEEP_US);
        adc_hal_read(&input_driver_1);
        adc_hal_read(&input_driver_2);
        adc_hal_read(&input_driver_3);
        adc_hal_read(&input_driver_4);

        a_vals[INPUT_CODE_NORTH] = input_driver_1.output; // NORTH
        a_vals[INPUT_CODE_WEST] = input_driver_2.output; // WEST
        a_vals[INPUT_CODE_EAST] = input_driver_3.output; //EAST
        a_vals[INPUT_CODE_SOUTH] = input_driver_4.output; // SOUTH
        gpio_put(PGPIO_EN_SEWN, true);
        break;

        case 1:
        // TRIGGERS
        gpio_put(PGPIO_EN_TRIGGER_L, false);
        gpio_put(PGPIO_EN_TRIGGER_R, false);
        sleep_us(BUTTON_SLEEP_US);
        adc_hal_read(&input_driver_1);
        adc_hal_read(&input_driver_2);
        adc_hal_read(&input_driver_3);
        adc_hal_read(&input_driver_4);

        a_vals[INPUT_CODE_RB] = input_driver_1.output;
        a_vals[INPUT_CODE_RT_ANALOG] = input_driver_2.output;
        a_vals[INPUT_CODE_LB] = input_driver_3.output;
        a_vals[INPUT_CODE_LT_ANALOG] = input_driver_4.output;
        gpio_put(PGPIO_EN_TRIGGER_L, true);
        gpio_put(PGPIO_EN_TRIGGER_R, true);
        break;

        case 2:
        // DPAD
        gpio_put(PGPIO_EN_DPAD, false);
        sleep_us(BUTTON_SLEEP_US);
        adc_hal_read(&input_driver_1);
        adc_hal_read(&input_driver_2);
        adc_hal_read(&input_driver_3);
        adc_hal_read(&input_driver_4);

        a_vals[INPUT_CODE_UP] = input_driver_1.output;
        a_vals[INPUT_CODE_RIGHT] = input_driver_2.output; // right
        a_vals[INPUT_CODE_LEFT] = input_driver_3.output;
        a_vals[INPUT_CODE_DOWN] = input_driver_4.output; // down
        gpio_put(PGPIO_EN_DPAD, true);
        break;
    }

    for(int i = 0; i < 36; i++)
    {
        input->inputs[i] = a_vals[i];
    }

    read_idx++;
    if(read_idx > 2) read_idx = 0;

    input->button_shipping = !gpio_get(PGPIO_BTN_POWER);
    out[INPUT_CODE_MISC3] = input->button_shipping;
    input->button_sync = !gpio_get(PGPIO_BTN_PLUS);
}

void cb_hoja_read_joystick(uint16_t *input)
{
    adc_hal_read(&joystick_driver_1);
    adc_hal_read(&joystick_driver_2);
    adc_hal_read(&joystick_driver_3);
    adc_hal_read(&joystick_driver_4);

    input[2] = 0xFFF - joystick_driver_1.output;
    input[3] = joystick_driver_2.output;
    input[0] = 0xFFF - joystick_driver_3.output;
    input[1] = 0xFFF - joystick_driver_4.output;
}

int main()
{
    hoja_init();
}
