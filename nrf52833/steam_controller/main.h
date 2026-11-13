#ifndef STEAM_CONTROLLER_MAIN_H
#define STEAM_CONTROLLER_MAIN_H

#include "hoja_bsp.h"

// Buttons wired to the chip: a press pulls the pin low
#define PGPIO_BTN_A             NRF52_PIN(0, 23)
#define PGPIO_BTN_B             NRF52_PIN(0, 10)
#define PGPIO_BTN_VIEW          NRF52_PIN(0, 12) // Left of centre
#define PGPIO_BTN_MENU          NRF52_PIN(1, 4)  // Right of centre
#define PGPIO_BTN_STEAM         NRF52_PIN(1, 6)

// The other buttons come through a GreenPAK (SLG4L48185) on the second I2C bus: a 16-bit word, a
// bit per button, low while pressed
#define PGPIO_BUTTONS_INT       NRF52_PIN(1, 7)
#define BUTTONS_I2C_INSTANCE    1
#define BUTTONS_I2C_ADDRESS     0x10
#define BUTTONS_READ_COMMAND    0xF0

#define BUTTONS_BIT_DPAD_UP     1
#define BUTTONS_BIT_DPAD_DOWN   2
#define BUTTONS_BIT_DPAD_LEFT   3
#define BUTTONS_BIT_DPAD_RIGHT  4
#define BUTTONS_BIT_QUICK       5
#define BUTTONS_BIT_L4          6
#define BUTTONS_BIT_L5          7
#define BUTTONS_BIT_LB          8
#define BUTTONS_BIT_Y           9
#define BUTTONS_BIT_LS          10
#define BUTTONS_BIT_X           11
#define BUTTONS_BIT_R5          12
#define BUTTONS_BIT_R4          13
#define BUTTONS_BIT_RB          14
#define BUTTONS_BIT_RS          15

// Sticks and triggers: analog inputs, their sensors powered by one enable pin
#define PGPIO_ANALOG_ENABLE     NRF52_PIN(0, 19)
#define AIN_STICK_LX            4
#define AIN_STICK_LY            1
#define AIN_STICK_RX            7
#define AIN_STICK_RY            5
#define AIN_TRIGGER_L           6
#define AIN_TRIGGER_R           0

// Battery voltage through a divider, switched in by its own pin
#define PGPIO_BATTERY_SENSE_EN  NRF52_PIN(1, 1)
#define AIN_BATTERY             3

// I2C bus 0: motion sensor (0x6A) and charger (0x4B)
#define PGPIO_I2C0_SDA          NRF52_PIN(0, 20)
#define PGPIO_I2C0_SCL          NRF52_PIN(0, 17)
// I2C bus 1: buttons (0x10) and the trackpad controller (0x2C)
#define PGPIO_I2C1_SDA          NRF52_PIN(0, 14)
#define PGPIO_I2C1_SCL          NRF52_PIN(0, 26)

// Trackpad controller ("Olympus", trackpad.h): low holds it in reset, and it pulls its ready pin low
// while a frame waits to be read
#define PGPIO_TRACKPAD_RESET    NRF52_PIN(0, 15)
#define PGPIO_TRACKPAD_READY    NRF52_PIN(0, 6)
#define TRACKPAD_RESET_MS       2
#define TRACKPAD_I2C_INSTANCE   1
#define TRACKPAD_I2C_ADDRESS    0x2C

// Trackpad haptics (PWM driven): held off, not used yet
#define PGPIO_PAD_HAPTIC_ENABLE NRF52_PIN(0, 9)
#define PGPIO_PAD_HAPTIC_LEFT   NRF52_PIN(0, 8)
#define PGPIO_PAD_HAPTIC_RIGHT  NRF52_PIN(0, 27)

// Grip haptics: I2S amplifier
#define PGPIO_I2S_SCK           NRF52_PIN(0, 1)
#define PGPIO_I2S_LRCK          NRF52_PIN(0, 16)
#define PGPIO_I2S_SDOUT         NRF52_PIN(0, 11)
#define PGPIO_HAPTIC_AMP_ENABLE NRF52_PIN(1, 5)

// Status LED, each colour lit by a low pin
#define PGPIO_LED_RED           NRF52_PIN(1, 9)
#define PGPIO_LED_GREEN         NRF52_PIN(0, 22)
#define PGPIO_LED_BLUE          NRF52_PIN(1, 0)
#define PGPIO_LED_WHITE         NRF52_PIN(1, 8)

#endif
