#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "driver_define_helper.h"
#include "rgb_define_helper.h"

#define HOJA_BT_LOGGING_DEBUG 0

// Device stuff
#define HOJA_PRODUCT        "SuperGamepad+"

// URL that will display to open a config tool
#define HOJA_WEBUSB_URL     "handheldlegend.github.io/hoja2"
#define HOJA_MANUFACTURER   "HHL"

#define HOJA_INPUT_ENABLE_SEWN 1
#define HOJA_INPUT_ENABLE_DPAD 1
#define HOJA_INPUT_ENABLE_STARTSELECT 1
#define HOJA_INPUT_ENABLE_HOME 0
#define HOJA_INPUT_ENABLE_CAPTURE 0
#define HOJA_INPUT_ENABLE_BUMPERS 1
#define HOJA_INPUT_ENABLE_TRIGGERS 0
#define HOJA_INPUT_ENABLE_UPPERGRIPS 0
#define HOJA_INPUT_ENABLE_LOWERGRIPS 0
#define HOJA_INPUT_ENABLE_POWER 0

#define HOJA_SINPUT_GAMEPAD_TYPE 0

#define HOJA_SINPUT_GAMEPAD_SUBTYPE   0 // All Exposed Type 
#define HOJA_SINPUT_GAMEPAD_FACESTYLE 3 // Nintendo Switch face style

#define HOJA_DISABLE_TOURNEY_MACRO 1

#define HOJA_SEWN_TYPE  SEWN_LAYOUT_BAYX

// ---------------------------------
// ---------------------------------

// SPI HAL Setup
//#define HOJA_SPI_0_ENABLE     1
//#define HOJA_SPI_0_GPIO_CLK   18
//#define HOJA_SPI_0_GPIO_MOSI  19
//#define HOJA_SPI_0_GPIO_MISO  20
//#define SPI_INSTANCE_0        0

// I2C HAL Setup
#define HOJA_I2C_1_ENABLE       1
#define HOJA_I2C_1_GPIO_SDA     26
#define HOJA_I2C_1_GPIO_SCL     27
#define I2C_INSTANCE_1          1

// IMU Driver Setup
// #define HOJA_IMU_CHAN_A_DRIVER          IMU_DRIVER_LSM6DSR
// #define HOJA_IMU_CHAN_B_DRIVER          IMU_DRIVER_LSM6DSR

// ---------------------------------
// ---------------------------------

// Irrelevant for SuperGamepad+
//#define ADC_SMOOTHING_STRENGTH      0

//#define HOJA_ADC_LX_DRIVER          ADC_DRIVER_MCP3002
//#define HOJA_ADC_LX_CHANNEL         0
//#define HOJA_ADC_LX_SPI_INSTANCE    0
//#define HOJA_ADC_LX_CS_PIN          16 

//#define HOJA_ADC_LY_DRIVER          ADC_DRIVER_MCP3002
//#define HOJA_ADC_LY_CHANNEL         1
//#define HOJA_ADC_LY_SPI_INSTANCE    0
//#define HOJA_ADC_LY_CS_PIN          16 

//#define HOJA_ADC_RX_DRIVER          ADC_DRIVER_MCP3002
//#define HOJA_ADC_RX_CHANNEL         0
//#define HOJA_ADC_RX_SPI_INSTANCE    0
//#define HOJA_ADC_RX_CS_PIN          22 

//#define HOJA_ADC_RY_DRIVER          ADC_DRIVER_MCP3002
//#define HOJA_ADC_RY_CHANNEL         1
//#define HOJA_ADC_RY_SPI_INSTANCE    0
//#define HOJA_ADC_RY_CS_PIN          22 
// ---------------------------------
// ---------------------------------

// Haptic Driver Setup
// No helper driver used
//#define HOJA_HD_HAPTICS_DRIVER      HD_HAPTICS_DRIVER_HAL
//#define HOJA_HDRUMBLE_CHAN_A_PIN    24
//#define HOJA_HDRUMBLE_CHAN_B_PIN 24

//#define BOARD_SAFE_PCM_MAX      100
//#define BOARD_LO_FREQUENCY_MIN  0.9f
//#define BOARD_HI_FREQUENCY_MIN  0.95f
// ---------------------------------
// ---------------------------------

// Bluetooth Driver Setup
#define HOJA_BLUETOOTH_DRIVER           BLUETOOTH_DRIVER_ESP32HOJA
#define BLUETOOTH_DRIVER_I2C_INSTANCE   1
#define BLUETOOTH_DRIVER_ENABLE_PIN     4
// ---------------------------------
// ---------------------------------

// USB Mux Driver Setup
#define HOJA_USB_MUX_DRIVER         USB_MUX_DRIVER_PI3USB4000A
#define USB_MUX_DRIVER_ENABLE_PIN   6
#define USB_MUX_DRIVER_SELECT_PIN   5
// ---------------------------------
// ---------------------------------

// Battery Driver Setup
#define HOJA_BATTERY_DRIVER         BATTERY_DRIVER_BQ25180
#define HOJA_BATTERY_I2C_INSTANCE   1
#define HOJA_BATTERY_PART_CODE      "HAC-006"

extern adc_driver_cfg_s battery_adc_1;
#define HOJA_BATTERY_ADC_CFG (adc_channel_cfg_s) {.ch_local = 3, .driver_cfg = &battery_adc_1} 

// ---------------------------------
// ---------------------------------

// Device Information Setup 
#define HOJA_DEVICE_NAME            HOJA_PRODUCT
#define HOJA_DEVICE_MAKER           HOJA_MANUFACTURER
#define HOJA_DEVICE_MANIFEST_URL    "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/super_gamepad/manifest.json"
#define HOJA_DEVICE_FIRMWARE_URL    "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/super_gamepad/super_gamepad.uf2"
#define HOJA_DEVICE_MANUAL_URL      "https://docs.handheldlegend.com/s/portal/doc/user-guide-ZCJzKM9emw"
#define HOJA_DEVICE_FCC_ELABEL      ""
#define HOJA_DEVICE_SNES_SUPPORTED      1
#define HOJA_DEVICE_JOYBUS_SUPPORTED    1
// ---------------------------------
// ---------------------------------

// Flash Memory Setup
// TO DO
#define HOJA_FLASH_BYTES 1
// ---------------------------------
// ---------------------------------

// Static Buttons Memory Setup
// Button order follows bit order of buttons_all input_shared_types. 
// LSB is dpad_up and we go in that order towards MSB
#define HOJA_BUTTONS_SUPPORTED_MAIN     0b0011010111111111 // ALL buttons supported
#define HOJA_BUTTONS_SUPPORTED_SYSTEM   0b000 // Home, Capture, Sync
// ---------------------------------
// ---------------------------------

// NESBUS Setup
#define HOJA_NESBUS_DRIVER          NESBUS_DRIVER_HAL
#define NESBUS_DRIVER_PIO_INSTANCE  1
#define NESBUS_DRIVER_DATA_PIN      7
#define NESBUS_DRIVER_CLOCK_PIN     8
#define NESBUS_DRIVER_LATCH_PIN     9

// ---------------------------------
// ---------------------------------

// JOYBUS Setup
#define HOJA_JOYBUS_N64_DRIVER          JOYBUS_N64_DRIVER_HAL
#define JOYBUS_N64_DRIVER_PIO_INSTANCE  1
#define JOYBUS_N64_DRIVER_DATA_PIN      7

#define HOJA_JOYBUS_GC_DRIVER           JOYBUS_GC_DRIVER_HAL
#define JOYBUS_GC_DRIVER_PIO_INSTANCE  1
#define JOYBUS_GC_DRIVER_DATA_PIN      7

// ---------------------------------
// ---------------------------------

// RGB Setup
#define HOJA_RGB_DRIVER         RGB_DRIVER_HAL
#define RGB_DRIVER_OUTPUT_PIN   15
#define RGB_DRIVER_LED_COUNT    32
#define RGB_DRIVER_ORDER        RGB_ORDER_GRB
#define RGB_DRIVER_PIO_INSTANCE 0

#define HOJA_RGB_GROUPS_NUM 7
#define HOJA_RGB_GROUP_NAMES { \
    {"A"}, {"B"}, {"X"}, {"Y"}, \
    {"D-Pad"},\
    {"Start"}, {"Select"} \
}

// If we don't have a specific player group
// we can use another group for notifications
#define HOJA_RGB_NOTIF_GROUP_IDX   6
#define HOJA_RGB_NOTIF_GROUP_SIZE  1

// U D L R is the order
// 0,  2,  3,  1
// Corresponds to the group names in order
// Filled out with any LED index corresponding to a group
#define HOJA_RGB_GROUPINGS { \
    RGB_GROUP(9), RGB_GROUP(7), RGB_GROUP(8), RGB_GROUP(6), \
    RGB_GROUP(1,2,0,3),\
    RGB_GROUP(5), RGB_GROUP(4) \
}

// This will relate an input button
// to a group index found in HOJA_RGB_GROUPINGS
#define RGB_REACT_GROUP_ASSIGNMENT {\
    1, 0, 3, 2, /* S, E, W, N */\
    4, -1, -1,  /* D-Pad, L Stick, R Stick */\
    -1, -1, -1, -1, /* L, R, ZL, ZR */\
    -1, -1, /* Home, Capture */\
    5, 6, /* Plus, Minus */\
}
// ---------------------------------
// ---------------------------------

#endif
