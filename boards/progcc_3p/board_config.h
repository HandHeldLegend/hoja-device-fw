#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "driver_define_helper.h"
#include "rgb_define_helper.h"

#define HOJA_BT_LOGGING_DEBUG 0

#define HOJA_PRODUCT        "ProGCC 3+"

// URL that will display to open a config tool
#define HOJA_WEBUSB_URL     "handheldlegend.github.io/hoja2"
#define HOJA_MANUFACTURER   "HHL"

#define HOJA_USB_VID        0x2E8A // Raspberry Pi
#define HOJA_USB_PID        0x10DF // ProGCC PID

#define HOJA_INPUT_ENABLE_SEWN 1
#define HOJA_INPUT_ENABLE_DPAD 1
#define HOJA_INPUT_ENABLE_STARTSELECT 1
#define HOJA_INPUT_ENABLE_HOME 1
#define HOJA_INPUT_ENABLE_CAPTURE 1
#define HOJA_INPUT_ENABLE_BUMPERS 1
#define HOJA_INPUT_ENABLE_TRIGGERS 1
#define HOJA_INPUT_ENABLE_UPPERGRIPS 0
#define HOJA_INPUT_ENABLE_LOWERGRIPS 0
#define HOJA_INPUT_ENABLE_POWER 0

#define HOJA_SINPUT_GAMEPAD_SUBTYPE    0 // Base ProGCC
#define HOJA_SINPUT_GAMEPAD_TYPE       7 // ProCon Type
#define HOJA_SINPUT_GAMEPAD_FACESTYLE  3 // ProCon Face Style

#define HOJA_SEWN_TYPE  SEWN_LAYOUT_BAYX

// ---------------------------------
// ---------------------------------

// SPI HAL Setup
#define HOJA_SPI_0_ENABLE     1
#define HOJA_SPI_0_GPIO_CLK   18
#define HOJA_SPI_0_GPIO_MOSI  19
#define HOJA_SPI_0_GPIO_MISO  20
#define SPI_INSTANCE_0        0

// I2C HAL Setup
#define HOJA_I2C_0_ENABLE       1
#define HOJA_I2C_0_GPIO_SDA     28
#define HOJA_I2C_0_GPIO_SCL     29
#define I2C_INSTANCE_0          0

// IMU Driver Setup
#define HOJA_IMU_CHAN_A_DRIVER          IMU_DRIVER_LSM6DSR_SPI
#define HOJA_IMU_CHAN_B_DRIVER          IMU_DRIVER_LSM6DSR_SPI

#define HOJA_IMU_CHAN_A_CS_PIN          17
#define HOJA_IMU_CHAN_A_SPI_INSTANCE    0
#define HOJA_IMU_CHAN_A_INVERT_FLAGS    0b100100

#define HOJA_IMU_CHAN_B_CS_PIN          21    // ok
#define HOJA_IMU_CHAN_B_SPI_INSTANCE    0
#define HOJA_IMU_CHAN_B_INVERT_FLAGS    0b010010
// ---------------------------------
// ---------------------------------

// Disable smoothing
#define ADC_SMOOTHING_STRENGTH      0

// MCP3002 (2 of them)
extern adc_driver_cfg_s user_adc_1;
extern adc_driver_cfg_s user_adc_2;

#define HOJA_ADC_LX_CFG (adc_channel_cfg_s) {.ch_local = 0, .driver_cfg = &user_adc_1} 
#define HOJA_ADC_LY_CFG (adc_channel_cfg_s) {.ch_local = 1, .driver_cfg = &user_adc_1} 
#define HOJA_ADC_RX_CFG (adc_channel_cfg_s) {.ch_local = 0, .driver_cfg = &user_adc_2} 
#define HOJA_ADC_RY_CFG (adc_channel_cfg_s) {.ch_local = 1, .driver_cfg = &user_adc_2} 

// ---------------------------------
// ---------------------------------

// Haptic Driver Setup
// No helper driver used
#define HOJA_HAPTICS_DRIVER        HAPTICS_DRIVER_ERM_HAL
#define HOJA_HAPTICS_CHAN_A_PIN    24
#define HOJA_HAPTICS_BRAKE_PIN     25

#define HOJA_HAPTICS_MAX        1.0f

#define HOJA_HAPTICS_MIN_LO     0.5f 
#define HOJA_HAPTICS_MIN_HI     0.5f 

#define HOJA_HAPTICS_DEBUG 0
#define HOJA_HAPTICS_DEBUG_STEP 0.025f
// ---------------------------------
// ---------------------------------

// Bluetooth Driver Setup
#define HOJA_BLUETOOTH_DRIVER           BLUETOOTH_DRIVER_ESP32HOJA
#define BLUETOOTH_DRIVER_I2C_INSTANCE   0
#define BLUETOOTH_DRIVER_ENABLE_PIN     14
// ---------------------------------
// ---------------------------------

// USB Mux Driver Setup
#define HOJA_USB_MUX_DRIVER         USB_MUX_DRIVER_PI3USB4000A
#define USB_MUX_DRIVER_ENABLE_PIN   27
#define USB_MUX_DRIVER_SELECT_PIN   1
// ---------------------------------
// ---------------------------------

// Battery Driver Setup
#define HOJA_BATTERY_DRIVER         BATTERY_DRIVER_BQ25180
#define HOJA_BATTERY_I2C_INSTANCE   0
#define HOJA_BATTERY_PART_CODE      "CTR-003"

extern adc_driver_cfg_s battery_adc_1;
#define HOJA_BATTERY_ADC_CFG (adc_channel_cfg_s) {.ch_local = 0, .driver_cfg = &battery_adc_1} 

// ---------------------------------
// ---------------------------------

// Device Information Setup 
#define HOJA_DEVICE_NAME            HOJA_PRODUCT
#define HOJA_DEVICE_MAKER           HOJA_MANUFACTURER
#define HOJA_DEVICE_MANIFEST_URL    "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/progcc_3p/manifest.json"
#define HOJA_DEVICE_FIRMWARE_URL    "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/progcc_3p/progcc_3p.uf2"
#define HOJA_DEVICE_MANUAL_URL      "https://docs.handheldlegend.com/s/portal/doc/user-guide-CGDu7Js71s"
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
#define HOJA_BUTTONS_SUPPORTED_MAIN     0b1111111111111111 // ALL buttons supported
#define HOJA_BUTTONS_SUPPORTED_SYSTEM   0b111 // Home, Capture, Sync
// ---------------------------------
// ---------------------------------

// NESBUS Setup
#define HOJA_NESBUS_DRIVER          NESBUS_DRIVER_HAL
#define NESBUS_DRIVER_PIO_INSTANCE  1
#define NESBUS_DRIVER_DATA_PIN      3
#define NESBUS_DRIVER_CLOCK_PIN     4
#define NESBUS_DRIVER_LATCH_PIN     5

// ---------------------------------
// ---------------------------------

// JOYBUS Setup
#define HOJA_JOYBUS_N64_DRIVER          JOYBUS_N64_DRIVER_HAL
#define JOYBUS_N64_DRIVER_PIO_INSTANCE  1
#define JOYBUS_N64_DRIVER_DATA_PIN      3

#define HOJA_JOYBUS_GC_DRIVER           JOYBUS_GC_DRIVER_HAL
#define JOYBUS_GC_DRIVER_PIO_INSTANCE  1
#define JOYBUS_GC_DRIVER_DATA_PIN      3

// ---------------------------------
// ---------------------------------

// RGB Setup
#define HOJA_RGB_DRIVER         RGB_DRIVER_HAL
#define RGB_DRIVER_OUTPUT_PIN   23
#define RGB_DRIVER_LED_COUNT    32
#define RGB_DRIVER_ORDER        RGB_ORDER_GRB
#define RGB_DRIVER_PIO_INSTANCE 0

#define HOJA_RGB_GROUPS_NUM 15
#define HOJA_RGB_GROUP_NAMES { \
    {"A"}, {"B"}, {"X"}, {"Y"}, \
    {"D-Pad"}, {"L Stick"}, {"R Stick"}, \
    {"L"}, {"R"}, {"ZL"}, {"ZR"}, \
    {"Home"}, {"Capture"}, \
    {"Plus"}, {"Minus"}\
}
#define HOJA_RGB_NOTIF_GROUP_IDX   11
#define HOJA_RGB_NOTIF_GROUP_SIZE  1
#define HOJA_RGB_PLAYER_TYPE        1 // Numerical player type. Type 2 is for color based player type

// L   D   R   U
// 12, 13, 14, 15
#define HOJA_RGB_GROUPINGS { \
    RGB_GROUP(18), \
    RGB_GROUP(19), \
    RGB_GROUP(17), \
    RGB_GROUP(16), \
    RGB_GROUP(11, 9, 8, 10), \
    RGB_GROUP(4, 5, 6, 7), \
    RGB_GROUP(0, 1, 2, 3), \
    RGB_GROUP(20), \
    RGB_GROUP(22), \
    RGB_GROUP(21), \
    RGB_GROUP(23), \
    RGB_GROUP(14), \
    RGB_GROUP(13), \
    RGB_GROUP(15), \
    RGB_GROUP(12)  \
}

// This will relate an input button
// to a group index found in HOJA_RGB_GROUPINGS
#define RGB_REACT_GROUP_ASSIGNMENT {\
    1, 0, 3, 2, /* S, E, W, N */\
    4, 5, 6, /* D-Pad, L Stick, R Stick */\
    7, 8, 9, 10, /* L, R, ZL, ZR */\
    11, 12, /* Home, Capture */\
    13, 14, /* Plus, Minus */\
}
// ---------------------------------
// ---------------------------------

#endif
