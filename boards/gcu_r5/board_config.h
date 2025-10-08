#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "driver_define_helper.h" 
#include "rgb_define_helper.h" 

#define HOJA_BT_LOGGING_DEBUG 0 

#define HOJA_PRODUCT        "GCU-R5" 

#define HOJA_USB_VID        0x2E8A // Raspberry Pi
#define HOJA_USB_PID        0x10DD // GC Ultimate PID

// URL that will display to open a config tool
#define HOJA_WEBUSB_URL     "handheldlegend.github.io/hoja2" 
#define HOJA_MANUFACTURER   "HHL" 

#define HOJA_SEWN_TYPE  SEWN_LAYOUT_AXBY

#define HOJA_INPUT_ENABLE_SEWN 1
#define HOJA_INPUT_ENABLE_DPAD 1
#define HOJA_INPUT_ENABLE_STARTSELECT 1
#define HOJA_INPUT_ENABLE_HOME 1
#define HOJA_INPUT_ENABLE_CAPTURE 1
#define HOJA_INPUT_ENABLE_BUMPERS 1
#define HOJA_INPUT_ENABLE_TRIGGERS 1
#define HOJA_INPUT_ENABLE_UPPERGRIPS 0
#define HOJA_INPUT_ENABLE_LOWERGRIPS 0
#define HOJA_INPUT_ENABLE_POWER 1

#define HOJA_SINPUT_GAMEPAD_SUBTYPE    0  // GC Ultimate
#define HOJA_SINPUT_GAMEPAD_TYPE       11 // GameCube Type
#define HOJA_SINPUT_GAMEPAD_FACESTYLE  2  // GameCube Face Style

#define HOJA_DISABLE_TOURNEY_MACRO 1

// ---------------------------------
// ---------------------------------

// SPI HAL Setup
// #define HOJA_SPI_0_ENABLE     1 
// #define HOJA_SPI_0_GPIO_CLK   6 
// #define HOJA_SPI_0_GPIO_MOSI  7 
// #define HOJA_SPI_0_GPIO_MISO  4 
// #define SPI_INSTANCE_0        0 

// I2C HAL Setup
#define HOJA_I2C_0_ENABLE       1
#define HOJA_I2C_0_GPIO_SDA     24
#define HOJA_I2C_0_GPIO_SCL     25
#define I2C_INSTANCE_0          0

#define HOJA_I2C_1_ENABLE       1
#define HOJA_I2C_1_GPIO_SDA     22
#define HOJA_I2C_1_GPIO_SCL     23
#define I2C_INSTANCE_1          1

// IMU Driver Setup
#define HOJA_IMU_CHAN_A_DRIVER          IMU_DRIVER_LSM6DSR_I2C 
#define HOJA_IMU_CHAN_B_DRIVER          IMU_DRIVER_LSM6DSR_I2C 

#define HOJA_IMU_CHAN_A_I2C_INSTANCE 0
#define HOJA_IMU_CHAN_B_I2C_INSTANCE 0

#define HOJA_IMU_CHAN_A_SELECT 1
#define HOJA_IMU_CHAN_B_SELECT 0

#define HOJA_IMU_CHAN_A_INVERT_FLAGS    0b010010 
#define HOJA_IMU_CHAN_B_INVERT_FLAGS    0b010010 

// ---------------------------------
// ---------------------------------

// Disable smoothing
#define ADC_SMOOTHING_STRENGTH      0

// TMUX1204 
extern adc_driver_cfg_s user_adc_mux; 

// HAL ADC (1 instance) 
extern adc_driver_cfg_s user_adc_hal; 

#define HOJA_ADC_LX_CFG (adc_channel_cfg_s) {.ch_local = 2, .driver_cfg = &user_adc_mux} 
#define HOJA_ADC_LY_CFG (adc_channel_cfg_s) {.ch_local = 0, .driver_cfg = &user_adc_mux} 

#define HOJA_ADC_RX_CFG (adc_channel_cfg_s) {.ch_local = 3, .driver_cfg = &user_adc_mux} 
#define HOJA_ADC_RY_CFG (adc_channel_cfg_s) {.ch_local = 1, .driver_cfg = &user_adc_mux} 

#define HOJA_ADC_LT_CFG (adc_channel_cfg_s) {.ch_local = 1, .driver_cfg = &user_adc_hal, .ch_invert = 1} 
#define HOJA_ADC_RT_CFG (adc_channel_cfg_s) {.ch_local = 0, .driver_cfg = &user_adc_hal, .ch_invert = 1} 
// ---------------------------------
// ---------------------------------

// Haptic Driver Setup
// No helper driver used
#define HOJA_HAPTICS_DRIVER         HAPTICS_DRIVER_LRA_HAL
#define HOJA_HAPTICS_CHAN_A_PIN     8
#define HOJA_HAPTICS_CHAN_B_PIN     9
#define HOJA_HAPTICS_CHAN_SWAP      1

#define HOJA_HAPTICS_MAX        0.625f

#define HOJA_HAPTICS_MIN_LO     0.1f
#define HOJA_HAPTICS_MIN_HI     0.0975f

#define HOJA_HAPTICS_DEBUG 0
// ---------------------------------
// ---------------------------------

// Bluetooth Driver Setup
#define HOJA_BLUETOOTH_DRIVER           BLUETOOTH_DRIVER_HAL
#define BLUETOOTH_DRIVER_PIO_INSTANCE   1 
#define BLUETOOTH_DRIVER_ENABLE_PIN     0 
#define BLUETOOTH_DRIVER_DATA_OUT       1 
#define BLUETOOTH_DRIVER_DATA_IN        1 
#define BLUETOOTH_DRIVER_WAKE           1 
#define BLUETOOTH_DRIVER_CLOCK          2 
#define BLUETOOTH_DRIVER_CS             3
// ---------------------------------
// ---------------------------------

// Battery Driver Setup
#define HOJA_BATTERY_DRIVER         BATTERY_DRIVER_BQ25180
#define HOJA_BATTERY_I2C_INSTANCE   1
#define HOJA_BATTERY_PART_CODE      "BDT 903035"
#define HOJA_BATTERY_CAPACITY       1200

#define HOJA_FUELGAUGE_DRIVER       FUELGAUGE_DRIVER_BQ27621G1
#define HOJA_FUELGAUGE_I2C_INSTANCE 1

// ---------------------------------
// ---------------------------------

// Device Information Setup 
#define HOJA_DEVICE_NAME            HOJA_PRODUCT
#define HOJA_DEVICE_MAKER           HOJA_MANUFACTURER
#define HOJA_DEVICE_MANIFEST_URL    "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/gcu_r5/manifest.json"
#define HOJA_DEVICE_FIRMWARE_URL    "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/gcu_r5/gcu_r5.uf2"
#define HOJA_DEVICE_MANUAL_URL      "https://docs.handheldlegend.com/s/portal/doc/user-guide-UoDtIku68z"
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

// NESBUS Setup
#define HOJA_NESBUS_DRIVER          NESBUS_DRIVER_HAL
#define NESBUS_DRIVER_PIO_INSTANCE  1
#define NESBUS_DRIVER_DATA_PIN      19
#define NESBUS_DRIVER_CLOCK_PIN     21
#define NESBUS_DRIVER_LATCH_PIN     20

// ---------------------------------
// ---------------------------------

// JOYBUS Setup
#define HOJA_JOYBUS_N64_DRIVER          JOYBUS_N64_DRIVER_HAL
#define JOYBUS_N64_DRIVER_PIO_INSTANCE  1
#define JOYBUS_N64_DRIVER_DATA_PIN      19

#define HOJA_JOYBUS_GC_DRIVER           JOYBUS_GC_DRIVER_HAL
#define JOYBUS_GC_DRIVER_PIO_INSTANCE  1
#define JOYBUS_GC_DRIVER_DATA_PIN      19

// ---------------------------------
// ---------------------------------

// RGB Setup
#define HOJA_RGB_DRIVER         RGB_DRIVER_HAL
#define RGB_DRIVER_OUTPUT_PIN   7
#define RGB_DRIVER_LED_COUNT    32
#define RGB_DRIVER_ORDER        RGB_ORDER_GRB
#define RGB_DRIVER_PIO_INSTANCE 0
#define RGB_DRIVER_REFRESHRATE  90 // HZ

#define HOJA_RGB_GROUPS_NUM 8
#define HOJA_RGB_GROUP_NAMES { \
    {"L Stick"}, \
    {"R Stick"}, \
    {"A"}, \
    {"B"}, \
    {"X"}, \
    {"Y"}, \
    {"D-Pad"}, \
    {"Power"} \
}

#define HOJA_RGB_NOTIF_GROUP_IDX   7
#define HOJA_RGB_NOTIF_GROUP_SIZE  1

// DPad Order must be
// L   D   R   U
// Corresponds to the group names in order
// Filled out with any LED index corresponding to a group
#define HOJA_RGB_GROUPINGS { \
    RGB_GROUP(0, 1, 2, 3), \
    RGB_GROUP(13, 14, 15, 16), \
    RGB_GROUP(11), \
    RGB_GROUP(9), \
    RGB_GROUP(12), \
    RGB_GROUP(10), \
    RGB_GROUP(4, 5, 6, 7), \
    RGB_GROUP(8) \
}

// This will relate an input button
// to a group index found in HOJA_RGB_GROUPINGS
#define RGB_REACT_GROUP_ASSIGNMENT {\
    2, 4, 3, 5, /* S, E, W, N */\
    6, 0, 1, /* D-Pad, L Stick, R Stick */\
    -1, -1, -1, -1, /* L, R, ZL, ZR */\
    -1, -1, /* Home, Capture */\
    -1, -1, /* Plus, Minus */\
}

// ---------------------------------
// ---------------------------------

#endif
