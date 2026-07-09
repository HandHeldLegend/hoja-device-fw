#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "driver_define_helper.h"
#include "rgb_define_helper.h"

#define HOJA_BT_LOGGING_DEBUG 0

#define HOJA_TASK_BENCHMARKING 1

#define HOJA_WEBUSB_URL     "handheldlegend.github.io/hoja2"

// ---------------------------------
// ---------------------------------

// HOJA Transport driver gates.
#define HOJA_TRANSPORT_BT_DRIVER        BT_DRIVER_ESP32HOJA
#define HOJA_TRANSPORT_USB_DRIVER       USB_DRIVER_HAL
#define HOJA_TRANSPORT_JOYBUS64_DRIVER  JOYBUS_N64_DRIVER_HAL
#define HOJA_TRANSPORT_JOYBUSGC_DRIVER  JOYBUS_GC_DRIVER_HAL
#define HOJA_TRANSPORT_NESBUS_DRIVER    NESBUS_DRIVER_HAL

// ESP32 baseband (I2C instance must match hoja_config_s.i2c wiring).
#define BLUETOOTH_DRIVER_I2C_INSTANCE      1
#define BLUETOOTH_DRIVER_ENABLE_PIN        26
#define BLUETOOTH_DRIVER_BATMON_ENABLE     1
#define BLUETOOTH_DRIVER_BATMON_ADC_GPIO   36

// USB mux (required for ESP32 baseband).
#define HOJA_USB_MUX_DRIVER         USB_MUX_DRIVER_PI3USB4000A
#define USB_MUX_DRIVER_ENABLE_PIN   24
#define USB_MUX_DRIVER_SELECT_PIN   25

// IMU Driver Setup
// Gate: selects WHICH IMU driver compiles + shapes hoja_config_s.imu.
#define HOJA_IMU_DRIVER             IMU_DRIVER_LSM6DSR

// Battery PMIC
#define HOJA_BATTERY_DRIVER         BATTERY_DRIVER_BQ25180

// Disable smoothing
#define ADC_SMOOTHING_STRENGTH      0
#define HOJA_ANALOG_DEFAULT_DISTANCE 1400.0f

// Haptic Driver
#define HOJA_HAPTICS_DRIVER         HAPTICS_DRIVER_LRA_HAL

// ---------------------------------
// ---------------------------------

// RGB Setup (gate + compile-time sizing only).
// Group layout lives in hoja_config_s.rgb / .rgb_driver (see main.c).
#define HOJA_RGB_DRIVER         RGB_DRIVER_HAL
#define RGB_DRIVER_LED_COUNT    12
#define RGB_DRIVER_ORDER        RGB_ORDER_GRB
#define RGB_DRIVER_REFRESHRATE  90

#define HOJA_RGB_GROUP_DEFAULTS { \
    (rgb_s) {.r = 0xB4, .g = 0xB4, .b = 0xB4},  /* L Stick */ \
    (rgb_s) {.r = 0x06, .g = 0xFE, .b = 0x23},  /* A */ \
    (rgb_s) {.r = 0xF2, .g = 0x60, .b = 0x50},  /* B */ \
    (rgb_s) {.r = 0x0E, .g = 0x86, .b = 0xE1},  /* X */ \
    (rgb_s) {.r = 0xE1, .g = 0xFF, .b = 0x00},  /* Y */ \
}

// ---------------------------------
// ---------------------------------

#endif
