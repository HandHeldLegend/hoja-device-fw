#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "driver_define_helper.h" 
#include "rgb_define_helper.h" 

#define HOJA_BT_LOGGING_DEBUG 0 

#define HOJA_PRODUCT        "Padbox GS" 

// URL that will display to open a config tool
#define HOJA_WEBUSB_URL     "handheldlegend.github.io/hoja2" 
#define HOJA_MANUFACTURER   "arkodd.com" 

#define HOJA_DISABLE_TOURNEY_MACRO 1

// ---------------------------------
// ---------------------------------

// SPI HAL Setup
//#define HOJA_SPI_0_ENABLE     1 
//#define HOJA_SPI_0_GPIO_CLK   6 
//#define HOJA_SPI_0_GPIO_MOSI  7 
//#define HOJA_SPI_0_GPIO_MISO  4 
//#define SPI_INSTANCE_0        0 

// I2C HAL Setup
//#define HOJA_I2C_1_ENABLE       1 
//#define HOJA_I2C_1_GPIO_SDA     22 
//#define HOJA_I2C_1_GPIO_SCL     23 
//#define I2C_INSTANCE_1          1 

// IMU Driver Setup
//#define HOJA_IMU_CHAN_A_DRIVER          IMU_DRIVER_LSM6DSR_SPI 
//#define HOJA_IMU_CHAN_B_DRIVER          IMU_DRIVER_LSM6DSR_SPI 

//#define HOJA_IMU_CHAN_A_CS_PIN          0 
//#define HOJA_IMU_CHAN_A_SPI_INSTANCE    0 
//#define HOJA_IMU_CHAN_A_INVERT_FLAGS    0b000010 
//#define HOJA_IMU_CHAN_B_CS_PIN          2 
//#define HOJA_IMU_CHAN_B_SPI_INSTANCE    0 
//#define HOJA_IMU_CHAN_B_INVERT_FLAGS    0b000010 
// ---------------------------------
// ---------------------------------

// Disable smoothing
#define ADC_SMOOTHING_STRENGTH      0

// HAL ADC (1 instance) 
extern adc_driver_cfg_s user_adc_hal; 

#define HOJA_ADC_LX_CFG (adc_channel_cfg_s) {.ch_local = 3, .driver_cfg = &user_adc_hal} 
#define HOJA_ADC_LY_CFG (adc_channel_cfg_s) {.ch_local = 2, .driver_cfg = &user_adc_hal, .ch_invert = true} 

#define HOJA_ADC_RX_CFG (adc_channel_cfg_s) {.ch_local = 1, .driver_cfg = &user_adc_hal} 
#define HOJA_ADC_RY_CFG (adc_channel_cfg_s) {.ch_local = 0, .driver_cfg = &user_adc_hal, .ch_invert = true} 

// ---------------------------------
// ---------------------------------

// Haptic Driver Setup
//#define HOJA_HAPTICS_DRIVER         HAPTICS_DRIVER_LRA_HAL
//#define HOJA_HAPTICS_CHAN_A_PIN     8
//#define HOJA_HAPTICS_CHAN_B_PIN     3

//#define HOJA_HAPTICS_MAX        0.625f 

//#define HOJA_HAPTICS_MIN_LO     0.12f
//#define HOJA_HAPTICS_MIN_HI     0.10f

//#define HOJA_HAPTICS_DEBUG 0
// ---------------------------------
// ---------------------------------

// Bluetooth Driver Setup
// #define HOJA_BLUETOOTH_DRIVER           BLUETOOTH_DRIVER_ESP32HOJA
// #define BLUETOOTH_DRIVER_I2C_INSTANCE   1
// #define BLUETOOTH_DRIVER_ENABLE_PIN     26
// ---------------------------------
// ---------------------------------

// USB Mux Driver Setup
// #define HOJA_USB_MUX_DRIVER         USB_MUX_DRIVER_PI3USB4000A
// #define USB_MUX_DRIVER_ENABLE_PIN   24
// #define USB_MUX_DRIVER_SELECT_PIN   25
// ---------------------------------
// ---------------------------------

// Battery Driver Setup
// #define HOJA_BATTERY_DRIVER         BATTERY_DRIVER_BQ25180
// #define HOJA_BATTERY_I2C_INSTANCE   1
// #define HOJA_BATTERY_CAPACITY_MAH   1200
// #define HOJA_BATTERY_PART_CODE      "BDT 903035"
// #define HOJA_BATTERY_CONSUME_RATE   225 // mA
// ---------------------------------
// ---------------------------------

// Device Information Setup 
#define HOJA_DEVICE_NAME            HOJA_PRODUCT
#define HOJA_DEVICE_MAKER           HOJA_MANUFACTURER
#define HOJA_DEVICE_MANIFEST_URL    "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/padbox_gs/manifest.json"
#define HOJA_DEVICE_FIRMWARE_URL    "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/padbox_gs/padbox_gs.uf2"
#define HOJA_DEVICE_MANUAL_URL      "https://arkodd.com/the-padbox-gs-detailed-description/"
#define HOJA_DEVICE_FCC_ELABEL      ""
#define HOJA_DEVICE_SNES_SUPPORTED      0
#define HOJA_DEVICE_JOYBUS_SUPPORTED    0
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
// #define HOJA_NESBUS_DRIVER          NESBUS_DRIVER_HAL
// #define NESBUS_DRIVER_PIO_INSTANCE  1
// #define NESBUS_DRIVER_DATA_PIN      20
// #define NESBUS_DRIVER_CLOCK_PIN     21
// #define NESBUS_DRIVER_LATCH_PIN     19

// ---------------------------------
// ---------------------------------

// JOYBUS Setup
//#define HOJA_JOYBUS_N64_DRIVER          JOYBUS_N64_DRIVER_HAL
//#define JOYBUS_N64_DRIVER_PIO_INSTANCE  1
//#define JOYBUS_N64_DRIVER_DATA_PIN      20

//#define HOJA_JOYBUS_GC_DRIVER           JOYBUS_GC_DRIVER_HAL
//#define JOYBUS_GC_DRIVER_PIO_INSTANCE  1
//#define JOYBUS_GC_DRIVER_DATA_PIN      20

// ---------------------------------
// ---------------------------------

// RGB Setup
#define HOJA_RGB_DRIVER         RGB_DRIVER_HAL
#define RGB_DRIVER_OUTPUT_PIN   14
#define RGB_DRIVER_LED_COUNT    32
#define RGB_DRIVER_ORDER        RGB_ORDER_GRB
#define RGB_DRIVER_PIO_INSTANCE 0
#define RGB_DRIVER_REFRESHRATE  120 // HZ

#define HOJA_RGB_GROUPS_NUM 8
#define HOJA_RGB_GROUP_NAMES { \
    {"L"}, \
    {"R"}, \
    {"B"}, \
    {"A"}, \
    {"X"}, \
    {"Y"}, \
    {"ZR"}, \
    {"ZL"} \
}

// #define HOJA_RGB_NOTIF_GROUP_IDX   5
// #define HOJA_RGB_NOTIF_GROUP_SIZE  1

// Corresponds to the group names in order
// Filled out with any LED index corresponding to a group
#define HOJA_RGB_GROUPINGS { \
    RGB_GROUP(0), \
    RGB_GROUP(1), \
    RGB_GROUP(2), \
    RGB_GROUP(3), \
    RGB_GROUP(4), \
    RGB_GROUP(5), \
    RGB_GROUP(6), \
    RGB_GROUP(7) \
}

// This will relate an input button
// to a group index found in HOJA_RGB_GROUPINGS
#define RGB_REACT_GROUP_ASSIGNMENT {\
    3, 2, 4, 5, /* A, B, X, Y */\
    -1, -1, -1, /* D-Pad, L Stick, R Stick */\
    0, 1, 7, 6, /* L, R, ZL, ZR */\
    -1, -1, /* Home, Capture */\
    -1, -1, /* Plus, Minus */\
}
// ---------------------------------
// ---------------------------------

#endif
