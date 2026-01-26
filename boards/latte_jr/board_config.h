//#ifndef BOARD_CONFIG_H
//#define BOARD_CONFIG_H

#include "driver_define_helper.h" 
#include "rgb_define_helper.h" 

#define HOJA_BT_LOGGING_DEBUG 0

// Device stuff
#define HOJA_PRODUCT        "Latte Jr 1" 

//#define HOJA_USB_VID        0x2E8A // Raspberry Pi
//#define HOJA_USB_PID        0x10DD // GC Ultimate PID

#define HOJA_SEWN_TYPE  SEWN_LAYOUT_ABXY

#define HOJA_SINPUT_GAMEPAD_SUBTYPE    0  // Default
#define HOJA_SINPUT_GAMEPAD_TYPE       0  // Default
#define HOJA_SINPUT_GAMEPAD_FACESTYLE  3 // Nintendo

// URL that will display to open a config tool
#define HOJA_WEBUSB_URL     "handheldlegend.github.io/hoja2" 
#define HOJA_MANUFACTURER   "HHL" 

#define HOJA_DISABLE_TOURNEY_MACRO 1

// ---------------------------------
// ---------------------------------

// I2C HAL Setup
#define HOJA_I2C_0_ENABLE       1 
#define HOJA_I2C_0_GPIO_SDA     12
#define HOJA_I2C_0_GPIO_SCL     13 
#define I2C_INSTANCE_0          0 

// Disable smoothing
#define ADC_SMOOTHING_STRENGTH      0

// ---------------------------------
// ---------------------------------

// Haptic Driver Setup
// No helper driver used
//#define HOJA_HAPTICS_DRIVER         HAPTICS_DRIVER_LRA_HAL
//#define HOJA_HAPTICS_CHAN_A_PIN     12
//#define HOJA_HAPTICS_CHAN_B_PIN     25
//
//#define HOJA_HAPTICS_MAX        0.5f
//
//#define HOJA_HAPTICS_MIN_LO     0.085f
//#define HOJA_HAPTICS_MIN_HI     0.085f
//
//#define HOJA_HAPTICS_DEBUG 0
// ---------------------------------
// ---------------------------------

// Bluetooth Driver Setup
// Bluetooth Driver Setup
#define HOJA_BLUETOOTH_DRIVER           BLUETOOTH_DRIVER_HAL
//#define BLUETOOTH_DRIVER_PIO_INSTANCE   1 
// ---------------------------------
// ---------------------------------

#define HOJA_BATTERY_DRIVER         BATTERY_DRIVER_BQ25180
#define HOJA_BATTERY_I2C_INSTANCE   0
#define HOJA_BATTERY_PART_CODE      "JD 603040"
#define HOJA_BATTERY_CAPACITY_MAH   800
#define HOJA_BATTERY_PMIC_PART_NUMBER "BQ25180"
#define HOJA_FUELGAUGE_DRIVER       FUELGAUGE_DRIVER_BQ27621G1
#define HOJA_FUELGAUGE_I2C_INSTANCE 0
#define HOJA_BATTERY_FUELGAUGE_PART_NUMBER "BQ27621G1"
// ---------------------------------
// ---------------------------------

// Device Information Setup 
#define HOJA_DEVICE_NAME            HOJA_PRODUCT
#define HOJA_DEVICE_MAKER           HOJA_MANUFACTURER
#define HOJA_DEVICE_MANIFEST_URL    "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw-2350/main/builds/latte_1/manifest.json"
#define HOJA_DEVICE_FIRMWARE_URL    "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw-2350/main/builds/latte_1/latte_1.uf2"
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
#define NESBUS_DRIVER_DATA_PIN      0
#define NESBUS_DRIVER_CLOCK_PIN     2
#define NESBUS_DRIVER_LATCH_PIN     1

// ---------------------------------
// ---------------------------------

// JOYBUS Setup
#define HOJA_JOYBUS_N64_DRIVER          JOYBUS_N64_DRIVER_HAL
#define JOYBUS_N64_DRIVER_PIO_INSTANCE  1
#define JOYBUS_N64_DRIVER_DATA_PIN      0

#define HOJA_JOYBUS_GC_DRIVER           JOYBUS_GC_DRIVER_HAL
#define JOYBUS_GC_DRIVER_PIO_INSTANCE  1
#define JOYBUS_GC_DRIVER_DATA_PIN      0

// ---------------------------------
// ---------------------------------

// INPUT Setup
// Follows convention of mapper_input_code_t
#define HOJA_INPUT_SLOTS { \
    (inputInfoSlot_s) {/*South*/.input_name="1", .input_type=INPUT_TYPE_HOVER, .rgb_group=5}, \
    (inputInfoSlot_s) {/*East*/.input_name="2", .input_type=INPUT_TYPE_HOVER, .rgb_group=6}, \
    (inputInfoSlot_s) {/*West*/.input_name="4", .input_type=INPUT_TYPE_HOVER, .rgb_group=8}, \
    (inputInfoSlot_s) {/*North*/.input_name="5", .input_type=INPUT_TYPE_HOVER, .rgb_group=9}, \
    (inputInfoSlot_s) {/*Up*/.input_name="D Up", .input_type=INPUT_TYPE_HOVER, .rgb_group=1}, \
    (inputInfoSlot_s) {/*Down*/.input_name="D Down", .input_type=INPUT_TYPE_HOVER, .rgb_group=3}, \
    (inputInfoSlot_s) {/*Left*/.input_name="D Left", .input_type=INPUT_TYPE_HOVER, .rgb_group=2}, \
    (inputInfoSlot_s) {/*Right*/.input_name="D Right", .input_type=INPUT_TYPE_HOVER, .rgb_group=4}, \
    (inputInfoSlot_s) {/*LB*/.input_name="6", .input_type=INPUT_TYPE_HOVER, .rgb_group=10}, \
    (inputInfoSlot_s) {/*RB*/.input_name="3", .input_type=INPUT_TYPE_HOVER, .rgb_group=7}, \
    (inputInfoSlot_s) {/*LT*/0}, \
    (inputInfoSlot_s) {/*LTANALOG*/.input_name="L", .input_type=INPUT_TYPE_HOVER}, \
    (inputInfoSlot_s) {/*RT*/0}, \
    (inputInfoSlot_s) {/*RTANALOG*/.input_name="R", .input_type=INPUT_TYPE_HOVER}, \
    (inputInfoSlot_s) {/*LP1*/.input_name="GL", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*RP1*/.input_name="GR", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*LP2*/0}, \
    (inputInfoSlot_s) {/*RP2*/0}, \
    (inputInfoSlot_s) {/*Start*/.input_name="Plus", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*Select*/.input_name="Minus", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*Home*/.input_name="Home", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*Share*/.input_name="Capture", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*MISC3*/.input_name="Power", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*MISC4*/0}, \
    (inputInfoSlot_s) {/*TP1*/0}, \
    (inputInfoSlot_s) {/*TP2*/0}, \
    (inputInfoSlot_s) {/*LS*/0}, \
    (inputInfoSlot_s) {/*LX_RIGHT*/ 0}, \
    (inputInfoSlot_s) {/*LX_LEFT*/  0}, \
    (inputInfoSlot_s) {/*LY_UP*/    0}, \
    (inputInfoSlot_s) {/*LY_DOWN*/  0}, \
    (inputInfoSlot_s) {/*RS*/       0}, \
    (inputInfoSlot_s) {/*RX_RIGHT*/ 0}, \
    (inputInfoSlot_s) {/*RX_LEFT*/  0}, \
    (inputInfoSlot_s) {/*RY_UP*/    0}, \
    (inputInfoSlot_s) {/*RY_DOWN*/  0} \
}

#define HOJA_HOVER_INVERTS {\
    /*South*/   0,\
    /*East*/    0,\
    /*West*/    0,\
    /*North*/   0,\
    /*Up*/      0,\
    /*Down*/    0,\
    /*Left*/    0,\
    /*Right*/   0,\
    /*LB*/      0,\
    /*RB*/      0,\
    /*LT*/      0,\
    /*LTANALOG*/1,\
    /*RT*/      0,\
    /*RTANALOG*/1,\
    /*LP1*/     0,\
    /*RP1*/     0,\
    /*LP2*/     0,\
    /*RP2*/     0,\
    /*Start*/   0,\
    /*Select*/  0,\
    /*Home*/    0,\
    /*Share*/   0,\
    /*MISC3*/   0,\
    /*MISC4*/   0,\
    /*TP1*/     0,\
    /*TP2*/     0,\
    /*LS*/      0,\
    /*LX_RIGHT*/0,\
    /*LX_LEFT*/ 0,\
    /*LY_UP*/   0,\
    /*LY_DOWN*/ 0,\
    /*RS*/      0,\
    /*RX_RIGHT*/0,\
    /*RX_LEFT*/ 0,\
    /*RY_UP*/   0,\
    /*RY_DOWN*/ 0\
}

#define HOJA_INPUT_DEFAULTS_SWITCH { \
    /*South*/       SWITCH_CODE_B, \
    /*East*/        SWITCH_CODE_A, \
    /*West*/        SWITCH_CODE_Y, \
    /*North*/       SWITCH_CODE_X, \
    /*Up*/          SWITCH_CODE_LY_UP, \
    /*Down*/        SWITCH_CODE_LY_DOWN, \
    /*Left*/        SWITCH_CODE_LX_LEFT, \
    /*Right*/       SWITCH_CODE_LX_RIGHT, \
    /*LB*/          SWITCH_CODE_ZL, \
    /*RB*/          SWITCH_CODE_ZR, \
    /*LT*/          SWITCH_CODE_UNUSED, \
    /*LTANALOG*/    SWITCH_CODE_L, \
    /*RT*/          SWITCH_CODE_UNUSED, \
    /*RTANALOG*/    SWITCH_CODE_R, \
    /*LP1*/         SWITCH_CODE_UNUSED, \
    /*RP1*/         SWITCH_CODE_UNUSED, \
    /*LP2*/         SWITCH_CODE_UNUSED, \
    /*RP2*/         SWITCH_CODE_UNUSED, \
    /*Start*/       SWITCH_CODE_PLUS, \
    /*Select*/      SWITCH_CODE_MINUS, \
    /*Home*/        SWITCH_CODE_HOME, \
    /*Share*/       SWITCH_CODE_CAPTURE, \
    /*MISC3*/       SWITCH_CODE_UNUSED, \
    /*MISC4*/       SWITCH_CODE_UNUSED, \
    /*TP1*/         SWITCH_CODE_UNUSED, \
    /*TP2*/         SWITCH_CODE_UNUSED, \
    /*LS*/          SWITCH_CODE_UNUSED, \
    /*LX_RIGHT*/    SWITCH_CODE_UNUSED, \
    /*LX_LEFT*/     SWITCH_CODE_UNUSED, \
    /*LY_UP*/       SWITCH_CODE_UNUSED, \
    /*LY_DOWN*/     SWITCH_CODE_UNUSED, \
    /*RS*/          SWITCH_CODE_UNUSED, \
    /*RX_RIGHT*/    SWITCH_CODE_UNUSED, \
    /*RX_LEFT*/     SWITCH_CODE_UNUSED, \
    /*RY_UP*/       SWITCH_CODE_UNUSED, \
    /*RY_DOWN*/     SWITCH_CODE_UNUSED  \
}

#define HOJA_INPUT_DEFAULTS_SNES { \
    /*South*/       SNES_CODE_B, \
    /*East*/        SNES_CODE_A, \
    /*West*/        SNES_CODE_Y, \
    /*North*/       SNES_CODE_X, \
    /*Up*/          SNES_CODE_UP, \
    /*Down*/        SNES_CODE_DOWN, \
    /*Left*/        SNES_CODE_LEFT, \
    /*Right*/       SNES_CODE_RIGHT, \
    /*LB*/          SNES_CODE_Y, \
    /*RB*/          SNES_CODE_B, \
    /*LT*/          SNES_CODE_UNUSED, \
    /*LTANALOG*/    SNES_CODE_L, \
    /*RT*/          SNES_CODE_UNUSED, \
    /*RTANALOG*/    SNES_CODE_R, \
    /*LP1*/         SNES_CODE_UNUSED, \
    /*RP1*/         SNES_CODE_UNUSED, \
    /*LP2*/         SNES_CODE_UNUSED, \
    /*RP2*/         SNES_CODE_UNUSED, \
    /*Start*/       SNES_CODE_START, \
    /*Select*/      SNES_CODE_SELECT, \
    /*Home*/        SNES_CODE_UNUSED, \
    /*Share*/       SNES_CODE_UNUSED, \
    /*MISC3*/       SNES_CODE_UNUSED, \
    /*MISC4*/       SNES_CODE_UNUSED, \
    /*TP1*/         SNES_CODE_UNUSED, \
    /*TP2*/         SNES_CODE_UNUSED, \
    /*LS*/          SNES_CODE_UNUSED, \
    /*LX_RIGHT*/    SNES_CODE_UNUSED, \
    /*LX_LEFT*/     SNES_CODE_UNUSED, \
    /*LY_UP*/       SNES_CODE_UNUSED, \
    /*LY_DOWN*/     SNES_CODE_UNUSED, \
    /*RS*/          SNES_CODE_UNUSED, \
    /*RX_RIGHT*/    SNES_CODE_UNUSED, \
    /*RX_LEFT*/     SNES_CODE_UNUSED, \
    /*RY_UP*/       SNES_CODE_UNUSED, \
    /*RY_DOWN*/     SNES_CODE_UNUSED  \
}

#define HOJA_INPUT_DEFAULTS_N64 { \
    /*South*/       N64_CODE_A, \
    /*East*/        N64_CODE_CDOWN, \
    /*West*/        N64_CODE_B, \
    /*North*/       N64_CODE_CLEFT, \
    /*Up*/          N64_CODE_LY_UP, \
    /*Down*/        N64_CODE_LY_DOWN, \
    /*Left*/        N64_CODE_LX_LEFT, \
    /*Right*/       N64_CODE_LX_RIGHT, \
    /*LB*/          N64_CODE_CUP, \
    /*RB*/          N64_CODE_CRIGHT, \
    /*LT*/          N64_CODE_UNUSED, \
    /*LTANALOG*/    N64_CODE_Z, \
    /*RT*/          N64_CODE_UNUSED, \
    /*RTANALOG*/    N64_CODE_R, \
    /*LP1*/         N64_CODE_UNUSED, \
    /*RP1*/         N64_CODE_UNUSED, \
    /*LP2*/         N64_CODE_UNUSED, \
    /*RP2*/         N64_CODE_UNUSED, \
    /*Start*/       N64_CODE_START, \
    /*Select*/      N64_CODE_L, \
    /*Home*/        N64_CODE_UNUSED, \
    /*Share*/       N64_CODE_UNUSED, \
    /*MISC3*/       N64_CODE_UNUSED, \
    /*MISC4*/       N64_CODE_UNUSED, \
    /*TP1*/         N64_CODE_UNUSED, \
    /*TP2*/         N64_CODE_UNUSED, \
    /*LS*/          N64_CODE_UNUSED, \
    /*LX_RIGHT*/    N64_CODE_UNUSED, \
    /*LX_LEFT*/     N64_CODE_UNUSED, \
    /*LY_UP*/       N64_CODE_UNUSED, \
    /*LY_DOWN*/     N64_CODE_UNUSED, \
    /*RS*/          N64_CODE_UNUSED, \
    /*RX_RIGHT*/    N64_CODE_UNUSED, \
    /*RX_LEFT*/     N64_CODE_UNUSED, \
    /*RY_UP*/       N64_CODE_UNUSED, \
    /*RY_DOWN*/     N64_CODE_UNUSED  \
}

#define HOJA_INPUT_DEFAULTS_GAMECUBE { \
    /*South*/       GAMECUBE_CODE_B, \
    /*East*/        GAMECUBE_CODE_A, \
    /*West*/        GAMECUBE_CODE_Y, \
    /*North*/       GAMECUBE_CODE_X, \
    /*Up*/          GAMECUBE_CODE_LY_UP, \
    /*Down*/        GAMECUBE_CODE_LY_DOWN, \
    /*Left*/        GAMECUBE_CODE_LX_LEFT, \
    /*Right*/       GAMECUBE_CODE_LX_RIGHT, \
    /*LB*/          GAMECUBE_CODE_L_ANALOG, \
    /*RB*/          GAMECUBE_CODE_R_ANALOG, \
    /*LT*/          GAMECUBE_CODE_UNUSED, \
    /*LTANALOG*/    GAMECUBE_CODE_L, \
    /*RT*/          GAMECUBE_CODE_UNUSED, \
    /*RTANALOG*/    GAMECUBE_CODE_R, \
    /*LP1*/         GAMECUBE_CODE_UNUSED, \
    /*RP1*/         GAMECUBE_CODE_UNUSED, \
    /*LP2*/         GAMECUBE_CODE_UNUSED, \
    /*RP2*/         GAMECUBE_CODE_UNUSED, \
    /*Start*/       GAMECUBE_CODE_START, \
    /*Select*/      GAMECUBE_CODE_Z, \
    /*Home*/        GAMECUBE_CODE_UNUSED, \
    /*Share*/       GAMECUBE_CODE_UNUSED, \
    /*MISC3*/       GAMECUBE_CODE_UNUSED, \
    /*MISC4*/       GAMECUBE_CODE_UNUSED, \
    /*TP1*/         GAMECUBE_CODE_UNUSED, \
    /*TP2*/         GAMECUBE_CODE_UNUSED, \
    /*LS*/          GAMECUBE_CODE_UNUSED, \
    /*LX_RIGHT*/    GAMECUBE_CODE_UNUSED, \
    /*LX_LEFT*/     GAMECUBE_CODE_UNUSED, \
    /*LY_UP*/       GAMECUBE_CODE_UNUSED, \
    /*LY_DOWN*/     GAMECUBE_CODE_UNUSED, \
    /*RS*/          GAMECUBE_CODE_UNUSED, \
    /*RX_RIGHT*/    GAMECUBE_CODE_UNUSED, \
    /*RX_LEFT*/     GAMECUBE_CODE_UNUSED, \
    /*RY_UP*/       GAMECUBE_CODE_UNUSED, \
    /*RY_DOWN*/     GAMECUBE_CODE_UNUSED  \
}

#define HOJA_INPUT_DEFAULTS_XINPUT { \
    /*South*/       XINPUT_CODE_B, \
    /*East*/        XINPUT_CODE_A, \
    /*West*/        XINPUT_CODE_Y, \
    /*North*/       XINPUT_CODE_X, \
    /*Up*/          XINPUT_CODE_LY_UP, \
    /*Down*/        XINPUT_CODE_LY_DOWN, \
    /*Left*/        XINPUT_CODE_LX_LEFT, \
    /*Right*/       XINPUT_CODE_LX_RIGHT, \
    /*LB*/          XINPUT_CODE_LB, \
    /*RB*/          XINPUT_CODE_RB, \
    /*LT*/          XINPUT_CODE_UNUSED, \
    /*LTANALOG*/    XINPUT_CODE_LT_ANALOG, \
    /*RT*/          XINPUT_CODE_UNUSED, \
    /*RTANALOG*/    XINPUT_CODE_RT_ANALOG, \
    /*LP1*/         XINPUT_CODE_UNUSED, \
    /*RP1*/         XINPUT_CODE_UNUSED, \
    /*LP2*/         XINPUT_CODE_UNUSED, \
    /*RP2*/         XINPUT_CODE_UNUSED, \
    /*Start*/       XINPUT_CODE_START, \
    /*Select*/      XINPUT_CODE_BACK, \
    /*Home*/        XINPUT_CODE_GUIDE, \
    /*Share*/       XINPUT_CODE_UNUSED, \
    /*MISC3*/       XINPUT_CODE_UNUSED, \
    /*MISC4*/       XINPUT_CODE_UNUSED, \
    /*TP1*/         XINPUT_CODE_UNUSED, \
    /*TP2*/         XINPUT_CODE_UNUSED, \
    /*LS*/          XINPUT_CODE_UNUSED, \
    /*LX_RIGHT*/    XINPUT_CODE_UNUSED, \
    /*LX_LEFT*/     XINPUT_CODE_UNUSED, \
    /*LY_UP*/       XINPUT_CODE_UNUSED, \
    /*LY_DOWN*/     XINPUT_CODE_UNUSED, \
    /*RS*/          XINPUT_CODE_UNUSED, \
    /*RX_RIGHT*/    XINPUT_CODE_UNUSED, \
    /*RX_LEFT*/     XINPUT_CODE_UNUSED, \
    /*RY_UP*/       XINPUT_CODE_UNUSED, \
    /*RY_DOWN*/     XINPUT_CODE_UNUSED  \
}

#define HOJA_INPUT_DEFAULTS_SINPUT { \
    /*South*/       SINPUT_CODE_SOUTH, \
    /*East*/        SINPUT_CODE_EAST, \
    /*West*/        SINPUT_CODE_WEST, \
    /*North*/       SINPUT_CODE_NORTH, \
    /*Up*/          SINPUT_CODE_UP, \
    /*Down*/        SINPUT_CODE_DOWN, \
    /*Left*/        SINPUT_CODE_LEFT, \
    /*Right*/       SINPUT_CODE_RIGHT, \
    /*LB*/          SINPUT_CODE_LB, \
    /*RB*/          SINPUT_CODE_RB, \
    /*LT*/          SINPUT_CODE_UNUSED, \
    /*LTANALOG*/    SINPUT_CODE_LT_ANALOG, \
    /*RT*/          SINPUT_CODE_UNUSED, \
    /*RTANALOG*/    SINPUT_CODE_RT_ANALOG, \
    /*LP1*/         SINPUT_CODE_LP_1, \
    /*RP1*/         SINPUT_CODE_RP_1, \
    /*LP2*/         SINPUT_CODE_UNUSED, \
    /*RP2*/         SINPUT_CODE_UNUSED, \
    /*Start*/       SINPUT_CODE_START, \
    /*Select*/      SINPUT_CODE_SELECT, \
    /*Home*/        SINPUT_CODE_GUIDE, \
    /*Share*/       SINPUT_CODE_SHARE, \
    /*MISC3*/       SINPUT_CODE_MISC_3, \
    /*MISC4*/       SINPUT_CODE_UNUSED, \
    /*TP1*/         SINPUT_CODE_UNUSED, \
    /*TP2*/         SINPUT_CODE_UNUSED, \
    /*LS*/          SINPUT_CODE_UNUSED, \
    /*LX_RIGHT*/    SINPUT_CODE_UNUSED, \
    /*LX_LEFT*/     SINPUT_CODE_UNUSED, \
    /*LY_UP*/       SINPUT_CODE_UNUSED, \
    /*LY_DOWN*/     SINPUT_CODE_UNUSED, \
    /*RS*/          SINPUT_CODE_UNUSED, \
    /*RX_RIGHT*/    SINPUT_CODE_UNUSED, \
    /*RX_LEFT*/     SINPUT_CODE_UNUSED, \
    /*RY_UP*/       SINPUT_CODE_UNUSED, \
    /*RY_DOWN*/     SINPUT_CODE_UNUSED  \
}

// ---------------------------------
// ---------------------------------

// RGB Setup
#define HOJA_RGB_DRIVER         RGB_DRIVER_HAL
#define RGB_DRIVER_OUTPUT_PIN   11
#define RGB_DRIVER_LED_COUNT    24
#define RGB_DRIVER_ORDER        RGB_ORDER_GRB
#define RGB_DRIVER_PIO_INSTANCE 0

#define HOJA_RGB_GROUPS_NUM 11
#define HOJA_RGB_GROUP_NAMES { \
    {"D Up"}, \
    {"D Left"}, \
    {"D Down"}, \
    {"D Right"}, \
    {"1"}, /*g5*/ \
    {"2"}, \
    {"3"}, \
    {"4"}, \
    {"5"}, \
    {"6"}, \
    {"Player"},\
}

#define HOJA_RGB_PLAYER_GROUP_IDX   10
#define HOJA_RGB_PLAYER_GROUP_SIZE  4

#define HOJA_RGB_NOTIF_GROUP_IDX   7
#define HOJA_RGB_NOTIF_GROUP_SIZE  1

#define HOJA_RGB_GROUPINGS { \
    RGB_GROUP(0), /*Up*/ \
    RGB_GROUP(1), /*Left*/ \
    RGB_GROUP(2), /*Down*/ \
    RGB_GROUP(3), /*Right*/ \
    RGB_GROUP(9), /*1*/ \
    RGB_GROUP(8), /*2*/ \
    RGB_GROUP(7), /*3*/ \
    RGB_GROUP(4), /*4*/ \
    RGB_GROUP(5), /*5*/ \
    RGB_GROUP(6), /*6*/ \
    RGB_GROUP(10, 11, 12, 13) \
}

// ---------------------------------
// ---------------------------------

//#endif
