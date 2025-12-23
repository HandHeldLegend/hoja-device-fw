#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "driver_define_helper.h" 
#include "rgb_define_helper.h" 

#define HOJA_BT_LOGGING_DEBUG 0

// Device stuff
#define HOJA_PRODUCT        "GCU-R4K" 

#define HOJA_USB_VID        0x2E8A // Raspberry Pi
#define HOJA_USB_PID        0x10DD // GC Ultimate PID

#define HOJA_SINPUT_ESP32TYPE 1

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

// URL that will display to open a config tool
#define HOJA_WEBUSB_URL     "handheldlegend.github.io/hoja2" 
#define HOJA_MANUFACTURER   "HHL" 

#define HOJA_DISABLE_TOURNEY_MACRO 1

// ---------------------------------
// ---------------------------------

// SPI HAL Setup
#define HOJA_SPI_0_ENABLE     1 
#define HOJA_SPI_0_GPIO_CLK   6 
#define HOJA_SPI_0_GPIO_MOSI  7 
#define HOJA_SPI_0_GPIO_MISO  4 
#define SPI_INSTANCE_0        0 

// I2C HAL Setup
#define HOJA_I2C_1_ENABLE       1 
#define HOJA_I2C_1_GPIO_SDA     22 
#define HOJA_I2C_1_GPIO_SCL     23 
#define I2C_INSTANCE_1          1 

// IMU Driver Setup
#define HOJA_IMU_CHAN_A_DRIVER          IMU_DRIVER_LSM6DSR_SPI 
#define HOJA_IMU_CHAN_B_DRIVER          IMU_DRIVER_LSM6DSR_SPI 

#define HOJA_IMU_CHAN_A_CS_PIN          0 
#define HOJA_IMU_CHAN_A_SPI_INSTANCE    0 
#define HOJA_IMU_CHAN_A_INVERT_FLAGS    0b010010 

#define HOJA_IMU_CHAN_B_CS_PIN          2 
#define HOJA_IMU_CHAN_B_SPI_INSTANCE    0 
#define HOJA_IMU_CHAN_B_INVERT_FLAGS    0b010010 
// ---------------------------------
// ---------------------------------

// Disable smoothing
#define ADC_SMOOTHING_STRENGTH      0

// MCP3002 (2 of them)
extern adc_driver_cfg_s gcuk_adc_1;
extern adc_driver_cfg_s gcuk_adc_2;

// HAL ADC (1 instance)
extern adc_driver_cfg_s user_adc_hal;

#define HOJA_ADC_LX_CFG (adc_channel_cfg_s) {.ch_local = 0, .driver_cfg = &gcuk_adc_1} 
#define HOJA_ADC_LY_CFG (adc_channel_cfg_s) {.ch_local = 1, .driver_cfg = &gcuk_adc_1} 

#define HOJA_ADC_RX_CFG (adc_channel_cfg_s) {.ch_local = 0, .driver_cfg = &gcuk_adc_2} 
#define HOJA_ADC_RY_CFG (adc_channel_cfg_s) {.ch_local = 1, .driver_cfg = &gcuk_adc_2} 

#define HOJA_ADC_LT_CFG (adc_channel_cfg_s) {.ch_local = 3, .driver_cfg = &user_adc_hal, .ch_invert = 1} 
#define HOJA_ADC_RT_CFG (adc_channel_cfg_s) {.ch_local = 2, .driver_cfg = &user_adc_hal, .ch_invert = 1} 

// ---------------------------------
// ---------------------------------

// Haptic Driver Setup
// No helper driver used
#define HOJA_HAPTICS_DRIVER         HAPTICS_DRIVER_LRA_HAL
#define HOJA_HAPTICS_CHAN_A_PIN     8
#define HOJA_HAPTICS_CHAN_B_PIN     3

#define HOJA_HAPTICS_MAX        0.625f

#define HOJA_HAPTICS_MIN_LO     0.135f
#define HOJA_HAPTICS_MIN_HI     0.10f

#define HOJA_HAPTICS_DEBUG 0
// ---------------------------------
// ---------------------------------

// Bluetooth Driver Setup
#define HOJA_BLUETOOTH_DRIVER           BLUETOOTH_DRIVER_ESP32HOJA
#define BLUETOOTH_DRIVER_I2C_INSTANCE   1 
#define BLUETOOTH_DRIVER_ENABLE_PIN     26 

#define BLUETOOTH_DRIVER_BATMON_ENABLE  1 // Enable battery monitoring by the ESP32
#define BLUETOOTH_DRIVER_BATMON_ADC_GPIO 36 // Select the battery monitoring ESP32 ADC channel
// ---------------------------------
// ---------------------------------

// USB Mux Driver Setup
#define HOJA_USB_MUX_DRIVER         USB_MUX_DRIVER_PI3USB4000A
#define USB_MUX_DRIVER_ENABLE_PIN   24
#define USB_MUX_DRIVER_SELECT_PIN   25
// ---------------------------------
// ---------------------------------

// Battery Driver Setup
#define HOJA_BATTERY_DRIVER         BATTERY_DRIVER_BQ25180
#define HOJA_BATTERY_I2C_INSTANCE   1
#define HOJA_BATTERY_CAPACITY_MAH   1200
#define HOJA_BATTERY_PART_CODE      "BDT 903035"
#define HOJA_BATTERY_CONSUME_RATE   225 // mA
// ---------------------------------
// ---------------------------------

// Device Information Setup 
#define HOJA_DEVICE_NAME            HOJA_PRODUCT
#define HOJA_DEVICE_MAKER           HOJA_MANUFACTURER
#define HOJA_DEVICE_MANIFEST_URL    "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/gcu_r4k/manifest.json"
#define HOJA_DEVICE_FIRMWARE_URL    "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/gcu_r4k/gcu_r4k.uf2"
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

// Static Buttons Memory Setup
#define HOJA_BUTTONS_SUPPORTED_MAIN     0b1111111111111111 // ALL buttons supported
#define HOJA_BUTTONS_SUPPORTED_SYSTEM   0b111 // Home, Capture, Sync
// ---------------------------------
// ---------------------------------

// NESBUS Setup
#define HOJA_NESBUS_DRIVER          NESBUS_DRIVER_HAL
#define NESBUS_DRIVER_PIO_INSTANCE  1
#define NESBUS_DRIVER_DATA_PIN      20
#define NESBUS_DRIVER_CLOCK_PIN     21
#define NESBUS_DRIVER_LATCH_PIN     19

// ---------------------------------
// ---------------------------------

// JOYBUS Setup
#define HOJA_JOYBUS_N64_DRIVER          JOYBUS_N64_DRIVER_HAL
#define JOYBUS_N64_DRIVER_PIO_INSTANCE  1
#define JOYBUS_N64_DRIVER_DATA_PIN      20

#define HOJA_JOYBUS_GC_DRIVER           JOYBUS_GC_DRIVER_HAL
#define JOYBUS_GC_DRIVER_PIO_INSTANCE  1
#define JOYBUS_GC_DRIVER_DATA_PIN      20

// ---------------------------------
// ---------------------------------

// INPUT Setup
// Follows convention of mapper_input_code_t
#define HOJA_INPUT_SLOTS { \
    (inputInfoSlot_s) {/*South*/.input_name="A", .input_type=INPUT_TYPE_DIGITAL, .rgb_assignments={0}}, \
    (inputInfoSlot_s) {/*East*/.input_name="X", .input_type=INPUT_TYPE_DIGITAL, .rgb_assignments={1}}, \
    (inputInfoSlot_s) {/*West*/.input_name="B", .input_type=INPUT_TYPE_DIGITAL, .rgb_assignments={2}}, \
    (inputInfoSlot_s) {/*North*/.input_name="Y", .input_type=INPUT_TYPE_DIGITAL, .rgb_assignments={3}}, \
    (inputInfoSlot_s) {/*Up*/.input_name="Up", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*Down*/.input_name="Down", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*Left*/.input_name="Left", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*Right*/.input_name="Right", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*SL*/.input_name="SL", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*SR*/.input_name="SR", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*LB*/.input_name="LZ", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*RB*/.input_name="RZ", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*LT*/.input_name="L", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*RT*/.input_name="R", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*LP1*/0}, \
    (inputInfoSlot_s) {/*RP1*/0}, \
    (inputInfoSlot_s) {/*Start*/.input_name="Plus", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*Select*/.input_name="Minus", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*Home*/.input_name="Home", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*Share*/.input_name="Capture", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*LP2*/0}, \
    (inputInfoSlot_s) {/*RP2*/0}, \
    (inputInfoSlot_s) {/*TP1*/0}, \
    (inputInfoSlot_s) {/*TP2*/0}, \
    (inputInfoSlot_s) {/*MISC3*/.input_name="Power", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*MISC4*/0}, \
    (inputInfoSlot_s) {/*LTANALOG*/.input_name="LT", .input_type=INPUT_TYPE_HOVER}, \
    (inputInfoSlot_s) {/*RTANALOG*/.input_name="RT", .input_type=INPUT_TYPE_HOVER}, \
    (inputInfoSlot_s) {/*LX_RIGHT*/.input_name="LX+", .input_type=INPUT_TYPE_JOYSTICK}, \
    (inputInfoSlot_s) {/*LX_LEFT*/.input_name="LX-", .input_type=INPUT_TYPE_JOYSTICK}, \
    (inputInfoSlot_s) {/*LY_UP*/.input_name="LY+", .input_type=INPUT_TYPE_JOYSTICK}, \
    (inputInfoSlot_s) {/*LY_DOWN*/.input_name="LY-", .input_type=INPUT_TYPE_JOYSTICK}, \
    (inputInfoSlot_s) {/*RX_RIGHT*/.input_name="RX+", .input_type=INPUT_TYPE_JOYSTICK}, \
    (inputInfoSlot_s) {/*RX_LEFT*/.input_name="RX-", .input_type=INPUT_TYPE_JOYSTICK}, \
    (inputInfoSlot_s) {/*RY_UP*/.input_name="RY+", .input_type=INPUT_TYPE_JOYSTICK}, \
    (inputInfoSlot_s) {/*RY_DOWN*/.input_name="RY-", .input_type=INPUT_TYPE_JOYSTICK} \
}

#define HOJA_INPUT_DEFAULTS_SWITCH { \
    /*South*/       SWITCH_CODE_A, \
    /*East*/        SWITCH_CODE_X, \
    /*West*/        SWITCH_CODE_B, \
    /*North*/       SWITCH_CODE_Y, \
    /*Up*/          SWITCH_CODE_UP, \
    /*Down*/        SWITCH_CODE_DOWN, \
    /*Left*/        SWITCH_CODE_LEFT, \
    /*Right*/       SWITCH_CODE_RIGHT, \
    /*SL*/          SWITCH_CODE_LS, \
    /*SR*/          SWITCH_CODE_RS, \
    /*LB*/          SWITCH_CODE_L, \
    /*RB*/          SWITCH_CODE_R, \
    /*LT*/          SWITCH_CODE_LZ, \
    /*RT*/          SWITCH_CODE_RZ, \
    /*LP1*/         SWITCH_CODE_UNUSED, \
    /*RP1*/         SWITCH_CODE_UNUSED, \
    /*Start*/       SWITCH_CODE_PLUS, \
    /*Select*/      SWITCH_CODE_MINUS, \
    /*Home*/        SWITCH_CODE_HOME, \
    /*Share*/       SWITCH_CODE_CAPTURE, \
    /*LP2*/         SWITCH_CODE_UNUSED, \
    /*RP2*/         SWITCH_CODE_UNUSED, \
    /*TP1*/         SWITCH_CODE_UNUSED, \
    /*TP2*/         SWITCH_CODE_UNUSED, \
    /*MISC3*/       SWITCH_CODE_UNUSED, \
    /*MISC4*/       SWITCH_CODE_UNUSED, \
    /*LTANALOG*/    SWITCH_CODE_LZ, \
    /*RTANALOG*/    SWITCH_CODE_RZ, \
    /*LX_RIGHT*/    SWITCH_CODE_LX_RIGHT, \
    /*LX_LEFT*/     SWITCH_CODE_LX_LEFT, \
    /*LY_UP*/       SWITCH_CODE_LY_UP, \
    /*LY_DOWN*/     SWITCH_CODE_LY_DOWN, \
    /*RX_RIGHT*/    SWITCH_CODE_RX_RIGHT, \
    /*RX_LEFT*/     SWITCH_CODE_RX_LEFT, \
    /*RY_UP*/       SWITCH_CODE_RY_UP, \
    /*RY_DOWN*/     SWITCH_CODE_RY_DOWN  \
}

#define HOJA_INPUT_DEFAULTS_SNES { \
    /*South*/       SNES_CODE_A, \
    /*East*/        SNES_CODE_X, \
    /*West*/        SNES_CODE_B, \
    /*North*/       SNES_CODE_Y, \
    /*Up*/          SNES_CODE_UP, \
    /*Down*/        SNES_CODE_DOWN, \
    /*Left*/        SNES_CODE_LEFT, \
    /*Right*/       SNES_CODE_RIGHT, \
    /*SL*/          SNES_CODE_UNUSED, \
    /*SR*/          SNES_CODE_UNUSED, \
    /*LB*/          SNES_CODE_L, \
    /*RB*/          SNES_CODE_R, \
    /*LT*/          SNES_CODE_UNUSED, \
    /*RT*/          SNES_CODE_UNUSED, \
    /*LP1*/         SNES_CODE_UNUSED, \
    /*RP1*/         SNES_CODE_UNUSED, \
    /*Start*/       SNES_CODE_START, \
    /*Select*/      SNES_CODE_SELECT, \
    /*Home*/        SNES_CODE_UNUSED, \
    /*Share*/       SNES_CODE_UNUSED, \
    /*LP2*/         SNES_CODE_UNUSED, \
    /*RP2*/         SNES_CODE_UNUSED, \
    /*TP1*/         SNES_CODE_UNUSED, \
    /*TP2*/         SNES_CODE_UNUSED, \
    /*MISC3*/       SNES_CODE_UNUSED, \
    /*MISC4*/       SNES_CODE_UNUSED, \
    /*LTANALOG*/    SNES_CODE_UNUSED, \
    /*RTANALOG*/    SNES_CODE_UNUSED, \
    /*LX_RIGHT*/    SNES_CODE_UNUSED, \
    /*LX_LEFT*/     SNES_CODE_UNUSED, \
    /*LY_UP*/       SNES_CODE_UNUSED, \
    /*LY_DOWN*/     SNES_CODE_UNUSED, \
    /*RX_RIGHT*/    SNES_CODE_UNUSED, \
    /*RX_LEFT*/     SNES_CODE_UNUSED, \
    /*RY_UP*/       SNES_CODE_UNUSED, \
    /*RY_DOWN*/     SNES_CODE_UNUSED  \
}

#define HOJA_INPUT_DEFAULTS_N64 { \
    /*South*/       N64_CODE_A, \
    /*East*/        N64_CODE_CDOWN, \
    /*West*/        N64_CODE_B, \
    /*North*/       N64_CODE_CUP, \
    /*Up*/          N64_CODE_UP, \
    /*Down*/        N64_CODE_DOWN, \
    /*Left*/        N64_CODE_LEFT, \
    /*Right*/       N64_CODE_RIGHT, \
    /*SL*/          N64_CODE_UNUSED, \
    /*SR*/          N64_CODE_UNUSED, \
    /*LB*/          N64_CODE_CLEFT, \
    /*RB*/          N64_CODE_CRIGHT, \
    /*LT*/          N64_CODE_Z, \
    /*RT*/          N64_CODE_R, \
    /*LP1*/         N64_CODE_UNUSED, \
    /*RP1*/         N64_CODE_UNUSED, \
    /*Start*/       N64_CODE_START, \
    /*Select*/      N64_CODE_L, \
    /*Home*/        N64_CODE_UNUSED, \
    /*Share*/       N64_CODE_UNUSED, \
    /*LP2*/         N64_CODE_UNUSED, \
    /*RP2*/         N64_CODE_UNUSED, \
    /*TP1*/         N64_CODE_UNUSED, \
    /*TP2*/         N64_CODE_UNUSED, \
    /*MISC3*/       N64_CODE_UNUSED, \
    /*MISC4*/       N64_CODE_UNUSED, \
    /*LTANALOG*/    N64_CODE_Z, \
    /*RTANALOG*/    N64_CODE_R, \
    /*LX_RIGHT*/    N64_CODE_LX_RIGHT, \
    /*LX_LEFT*/     N64_CODE_LX_LEFT, \
    /*LY_UP*/       N64_CODE_LY_UP, \
    /*LY_DOWN*/     N64_CODE_LY_DOWN, \
    /*RX_RIGHT*/    N64_CODE_UNUSED, \
    /*RX_LEFT*/     N64_CODE_UNUSED, \
    /*RY_UP*/       N64_CODE_UNUSED, \
    /*RY_DOWN*/     N64_CODE_UNUSED  \
}

#define HOJA_INPUT_DEFAULTS_GAMECUBE { \
    /*South*/       GAMECUBE_CODE_A, \
    /*East*/        GAMECUBE_CODE_X, \
    /*West*/        GAMECUBE_CODE_B, \
    /*North*/       GAMECUBE_CODE_Y, \
    /*Up*/          GAMECUBE_CODE_UP, \
    /*Down*/        GAMECUBE_CODE_DOWN, \
    /*Left*/        GAMECUBE_CODE_LEFT, \
    /*Right*/       GAMECUBE_CODE_RIGHT, \
    /*SL*/          GAMECUBE_CODE_UNUSED, \
    /*SR*/          GAMECUBE_CODE_UNUSED, \
    /*LB*/          GAMECUBE_CODE_UNUSED, \
    /*RB*/          GAMECUBE_CODE_Z, \
    /*LT*/          GAMECUBE_CODE_L, \
    /*RT*/          GAMECUBE_CODE_R, \
    /*LP1*/         GAMECUBE_CODE_UNUSED, \
    /*RP1*/         GAMECUBE_CODE_UNUSED, \
    /*Start*/       GAMECUBE_CODE_START, \
    /*Select*/      GAMECUBE_CODE_UNUSED, \
    /*Home*/        GAMECUBE_CODE_UNUSED, \
    /*Share*/       GAMECUBE_CODE_UNUSED, \
    /*LP2*/         GAMECUBE_CODE_UNUSED, \
    /*RP2*/         GAMECUBE_CODE_UNUSED, \
    /*TP1*/         GAMECUBE_CODE_UNUSED, \
    /*TP2*/         GAMECUBE_CODE_UNUSED, \
    /*MISC3*/       GAMECUBE_CODE_UNUSED, \
    /*MISC4*/       GAMECUBE_CODE_UNUSED, \
    /*LTANALOG*/    GAMECUBE_CODE_L_ANALOG, \
    /*RTANALOG*/    GAMECUBE_CODE_R_ANALOG, \
    /*LX_RIGHT*/    GAMECUBE_CODE_LX_RIGHT, \
    /*LX_LEFT*/     GAMECUBE_CODE_LX_LEFT, \
    /*LY_UP*/       GAMECUBE_CODE_LY_UP, \
    /*LY_DOWN*/     GAMECUBE_CODE_LY_DOWN, \
    /*RX_RIGHT*/    GAMECUBE_CODE_RX_RIGHT, \
    /*RX_LEFT*/     GAMECUBE_CODE_RX_LEFT, \
    /*RY_UP*/       GAMECUBE_CODE_RY_UP, \
    /*RY_DOWN*/     GAMECUBE_CODE_RY_DOWN  \
}

#define HOJA_INPUT_DEFAULTS_XINPUT { \
    /*South*/       XINPUT_CODE_A, \
    /*East*/        XINPUT_CODE_X, \
    /*West*/        XINPUT_CODE_B, \
    /*North*/       XINPUT_CODE_Y, \
    /*Up*/          XINPUT_CODE_UP, \
    /*Down*/        XINPUT_CODE_DOWN, \
    /*Left*/        XINPUT_CODE_LEFT, \
    /*Right*/       XINPUT_CODE_RIGHT, \
    /*SL*/          XINPUT_CODE_LS, \
    /*SR*/          XINPUT_CODE_RS, \
    /*LB*/          XINPUT_CODE_LB, \
    /*RB*/          XINPUT_CODE_RB, \
    /*LT*/          XINPUT_CODE_UNUSED, \
    /*RT*/          XINPUT_CODE_UNUSED, \
    /*LP1*/         XINPUT_CODE_UNUSED, \
    /*RP1*/         XINPUT_CODE_UNUSED, \
    /*Start*/       XINPUT_CODE_START, \
    /*Select*/      XINPUT_CODE_BACK, \
    /*Home*/        XINPUT_CODE_GUIDE, \
    /*Share*/       XINPUT_CODE_UNUSED, \
    /*LP2*/         XINPUT_CODE_UNUSED, \
    /*RP2*/         XINPUT_CODE_UNUSED, \
    /*TP1*/         XINPUT_CODE_UNUSED, \
    /*TP2*/         XINPUT_CODE_UNUSED, \
    /*MISC3*/       XINPUT_CODE_UNUSED, \
    /*MISC4*/       XINPUT_CODE_UNUSED, \
    /*LTANALOG*/    XINPUT_CODE_LT_ANALOG, \
    /*RTANALOG*/    XINPUT_CODE_RT_ANALOG, \
    /*LX_RIGHT*/    XINPUT_CODE_LX_RIGHT, \
    /*LX_LEFT*/     XINPUT_CODE_LX_LEFT, \
    /*LY_UP*/       XINPUT_CODE_LY_UP, \
    /*LY_DOWN*/     XINPUT_CODE_LY_DOWN, \
    /*RX_RIGHT*/    XINPUT_CODE_RX_RIGHT, \
    /*RX_LEFT*/     XINPUT_CODE_RX_LEFT, \
    /*RY_UP*/       XINPUT_CODE_RY_UP, \
    /*RY_DOWN*/     XINPUT_CODE_RY_DOWN  \
}

// ---------------------------------
// ---------------------------------

// RGB Setup
#define HOJA_RGB_DRIVER         RGB_DRIVER_HAL
#define RGB_DRIVER_OUTPUT_PIN   27
#define RGB_DRIVER_LED_COUNT    12
#define RGB_DRIVER_ORDER        RGB_ORDER_GRB
#define RGB_DRIVER_PIO_INSTANCE 0

#define HOJA_RGB_GROUPS_NUM 6
#define HOJA_RGB_GROUP_NAMES { \
    {"L Stick"}, \
    {"A"}, \
    {"B"}, \
    {"X"}, \
    {"Y"}, \
    {"Power"} \
}

#define HOJA_RGB_GROUP_DEFAULTS { \
    (rgb_s) {.r = 0xB4, .g = 0xB4, .b = 0xB4},  /* L Stick */ \
    (rgb_s) {.r = 0x06, .g = 0xFE, .b = 0x23},  /* A */ \
    (rgb_s) {.r = 0xF2, .g = 0x60, .b = 0x50},  /* B */ \
    (rgb_s) {.r = 0x0E, .g = 0x86, .b = 0xE1},  /* X */ \
    (rgb_s) {.r = 0xE1, .g = 0xFF, .b = 0x00},  /* Y */ \
    (rgb_s) {.r = 0xB4, .g = 0xB4, .b = 0xB4}   /* Power */ \
}

#define HOJA_RGB_NOTIF_GROUP_IDX   5
#define HOJA_RGB_NOTIF_GROUP_SIZE  1

// L   D   R   U
// 12, 13, 14, 15
// Corresponds to the group names in order
// Filled out with any LED index corresponding to a group
#define HOJA_RGB_GROUPINGS { \
    RGB_GROUP(0, 1, 2, 3), \
    RGB_GROUP(7), \
    RGB_GROUP(5), \
    RGB_GROUP(8), \
    RGB_GROUP(6), \
    RGB_GROUP(4) \
}

// This will relate an input button
// to a group index found in HOJA_RGB_GROUPINGS



#define RGB_REACT_GROUP_ASSIGNMENT {\
    1, 3, 2, 4, /* S, E, W, N */\
    -1, 0, -1, /* D-Pad, L Stick, R Stick */\
    -1, -1, -1, -1, /* L, R, ZL, ZR */\
    -1, -1, /* Home, Capture */\
    -1, -1, /* Plus, Minus */\
}
// ---------------------------------
// ---------------------------------

#endif
