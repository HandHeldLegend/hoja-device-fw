#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "driver_define_helper.h"
#include "rgb_define_helper.h"

#define HOJA_BT_LOGGING_DEBUG 0

#define HOJA_PRODUCT        "Phob 2"

// URL that will display to open a config tool
#define HOJA_WEBUSB_URL     "handheldlegend.github.io/hoja2"
#define HOJA_MANUFACTURER   "PhobGCC"

#define HOJA_SINPUT_GAMEPAD_SUBTYPE    0  // GC Ultimate
#define HOJA_SINPUT_GAMEPAD_TYPE       11 // GameCube Type
#define HOJA_SINPUT_GAMEPAD_FACESTYLE  2  // GameCube Face Style

#define HOJA_DISABLE_TOURNEY_MACRO 1

#define HOJA_SEWN_TYPE  SEWN_LAYOUT_AXBY

// ---------------------------------
// ---------------------------------

// SPI HAL Setup
#define HOJA_SPI_0_ENABLE     1
#define HOJA_SPI_0_GPIO_CLK   6
#define HOJA_SPI_0_GPIO_MOSI  7
#define HOJA_SPI_0_GPIO_MISO  4
#define SPI_INSTANCE_0        0

// ---------------------------------
// ---------------------------------

// Disable smoothing
#define ADC_SMOOTHING_STRENGTH      0
#define HOJA_ANALOG_INVERT_ALLOWED  1

// ---------------------------------
// ---------------------------------

// Haptic Driver Setup
// No helper driver used
#define HOJA_HAPTICS_DRIVER         HAPTICS_DRIVER_ERM_HAL
#define HOJA_HAPTICS_CHAN_A_PIN     25

#define HOJA_HAPTICS_MAX        1.0f

#define HOJA_HAPTICS_MIN_LO     0.625f 
#define HOJA_HAPTICS_MIN_HI     0.625f 

#define HOJA_HAPTICS_DEBUG 0
#define HOJA_HAPTICS_DEBUG_STEP 0.025f
// ---------------------------------
// ---------------------------------

// Device Information Setup 
#define HOJA_DEVICE_NAME            HOJA_PRODUCT
#define HOJA_DEVICE_MAKER           HOJA_MANUFACTURER
#define HOJA_DEVICE_MANIFEST_URL    "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/phob_2/manifest.json"
#define HOJA_DEVICE_FIRMWARE_URL    "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/phob_2/phob_2.uf2"
#define HOJA_DEVICE_MANUAL_URL      ""

#define HOJA_DEVICE_SNES_SUPPORTED      0
#define HOJA_DEVICE_JOYBUS_SUPPORTED    1
// ---------------------------------
// ---------------------------------

// Flash Memory Setup
// TO DO
#define HOJA_FLASH_BYTES 1
// ---------------------------------
// ---------------------------------


// JOYBUS Setup
#define HOJA_JOYBUS_N64_DRIVER          JOYBUS_N64_DRIVER_HAL
#define JOYBUS_N64_DRIVER_PIO_INSTANCE  1
#define JOYBUS_N64_DRIVER_DATA_PIN      28

#define HOJA_JOYBUS_GC_DRIVER           JOYBUS_GC_DRIVER_HAL
#define JOYBUS_GC_DRIVER_PIO_INSTANCE  1
#define JOYBUS_GC_DRIVER_DATA_PIN      28
// ---------------------------------
// ---------------------------------

// INPUT Setup
// Follows convention of mapper_input_code_t
#define HOJA_INPUT_SLOTS { \
    (inputInfoSlot_s) {/*South*/.input_name="A", .input_type=INPUT_TYPE_DIGITAL, .rgb_group=3}, \
    (inputInfoSlot_s) {/*East*/.input_name="X", .input_type=INPUT_TYPE_DIGITAL, .rgb_group=5}, \
    (inputInfoSlot_s) {/*West*/.input_name="B", .input_type=INPUT_TYPE_DIGITAL, .rgb_group=4}, \
    (inputInfoSlot_s) {/*North*/.input_name="Y", .input_type=INPUT_TYPE_DIGITAL, .rgb_group=6}, \
    (inputInfoSlot_s) {/*Up*/.input_name="D Up", .input_type=INPUT_TYPE_DIGITAL, .rgb_group=7}, \
    (inputInfoSlot_s) {/*Down*/.input_name="D Down", .input_type=INPUT_TYPE_DIGITAL, .rgb_group=8}, \
    (inputInfoSlot_s) {/*Left*/.input_name="D Left", .input_type=INPUT_TYPE_DIGITAL, .rgb_group=9}, \
    (inputInfoSlot_s) {/*Right*/.input_name="D Right", .input_type=INPUT_TYPE_DIGITAL, .rgb_group=10}, \
    (inputInfoSlot_s) {/*LB*/.input_name="ZL", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*RB*/.input_name="ZR", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*LT*/.input_name="L", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*LTANALOG*/.input_name="LT", .input_type=INPUT_TYPE_HOVER}, \
    (inputInfoSlot_s) {/*RT*/.input_name="R", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*RTANALOG*/.input_name="RT", .input_type=INPUT_TYPE_HOVER}, \
    (inputInfoSlot_s) {/*LP1*/0}, \
    (inputInfoSlot_s) {/*RP1*/0}, \
    (inputInfoSlot_s) {/*LP2*/0}, \
    (inputInfoSlot_s) {/*RP2*/0}, \
    (inputInfoSlot_s) {/*Start*/.input_name="Start", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*Select*/0}, \
    (inputInfoSlot_s) {/*Home*/0}, \
    (inputInfoSlot_s) {/*Share*/0}, \
    (inputInfoSlot_s) {/*MISC3*/0}, \
    (inputInfoSlot_s) {/*MISC4*/0}, \
    (inputInfoSlot_s) {/*TP1*/0}, \
    (inputInfoSlot_s) {/*TP2*/0}, \
    (inputInfoSlot_s) {/*LS*/0}, \
    (inputInfoSlot_s) {/*LX_RIGHT*/.input_name="LX+", .input_type=INPUT_TYPE_JOYSTICK, .rgb_group=1}, \
    (inputInfoSlot_s) {/*LX_LEFT*/.input_name="LX-", .input_type=INPUT_TYPE_JOYSTICK}, \
    (inputInfoSlot_s) {/*LY_UP*/.input_name="LY+", .input_type=INPUT_TYPE_JOYSTICK}, \
    (inputInfoSlot_s) {/*LY_DOWN*/.input_name="LY-", .input_type=INPUT_TYPE_JOYSTICK}, \
    (inputInfoSlot_s) {/*RS*/0}, \
    (inputInfoSlot_s) {/*RX_RIGHT*/.input_name="RX+", .input_type=INPUT_TYPE_JOYSTICK, .rgb_group=2}, \
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
    /*LB*/          SWITCH_CODE_L, \
    /*RB*/          SWITCH_CODE_R, \
    /*LT*/          SWITCH_CODE_ZL, \
    /*LTANALOG*/    SWITCH_CODE_ZL, \
    /*RT*/          SWITCH_CODE_ZR, \
    /*RTANALOG*/    SWITCH_CODE_ZR, \
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
    /*LS*/          SWITCH_CODE_LS, \
    /*LX_RIGHT*/    SWITCH_CODE_LX_RIGHT, \
    /*LX_LEFT*/     SWITCH_CODE_LX_LEFT, \
    /*LY_UP*/       SWITCH_CODE_LY_UP, \
    /*LY_DOWN*/     SWITCH_CODE_LY_DOWN, \
    /*RS*/          SWITCH_CODE_RS, \
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
    /*LB*/          SNES_CODE_L, \
    /*RB*/          SNES_CODE_R, \
    /*LT*/          SNES_CODE_UNUSED, \
    /*LTANALOG*/    SNES_CODE_UNUSED, \
    /*RT*/          SNES_CODE_UNUSED, \
    /*RTANALOG*/    SNES_CODE_UNUSED, \
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
    /*North*/       N64_CODE_CUP, \
    /*Up*/          N64_CODE_UP, \
    /*Down*/        N64_CODE_DOWN, \
    /*Left*/        N64_CODE_LEFT, \
    /*Right*/       N64_CODE_RIGHT, \
    /*LB*/          N64_CODE_CLEFT, \
    /*RB*/          N64_CODE_CRIGHT, \
    /*LT*/          N64_CODE_Z, \
    /*LTANALOG*/    N64_CODE_Z, \
    /*RT*/          N64_CODE_R, \
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
    /*LX_RIGHT*/    N64_CODE_LX_RIGHT, \
    /*LX_LEFT*/     N64_CODE_LX_LEFT, \
    /*LY_UP*/       N64_CODE_LY_UP, \
    /*LY_DOWN*/     N64_CODE_LY_DOWN, \
    /*RS*/          N64_CODE_UNUSED, \
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
    /*LB*/          GAMECUBE_CODE_UNUSED, \
    /*RB*/          GAMECUBE_CODE_Z, \
    /*LT*/          GAMECUBE_CODE_L, \
    /*LTANALOG*/    GAMECUBE_CODE_L_ANALOG, \
    /*RT*/          GAMECUBE_CODE_R, \
    /*RTANALOG*/    GAMECUBE_CODE_R_ANALOG, \
    /*LP1*/         GAMECUBE_CODE_UNUSED, \
    /*RP1*/         GAMECUBE_CODE_UNUSED, \
    /*LP2*/         GAMECUBE_CODE_UNUSED, \
    /*RP2*/         GAMECUBE_CODE_UNUSED, \
    /*Start*/       GAMECUBE_CODE_START, \
    /*Select*/      GAMECUBE_CODE_UNUSED, \
    /*Home*/        GAMECUBE_CODE_UNUSED, \
    /*Share*/       GAMECUBE_CODE_UNUSED, \
    /*MISC3*/       GAMECUBE_CODE_UNUSED, \
    /*MISC4*/       GAMECUBE_CODE_UNUSED, \
    /*TP1*/         GAMECUBE_CODE_UNUSED, \
    /*TP2*/         GAMECUBE_CODE_UNUSED, \
    /*LS*/          GAMECUBE_CODE_UNUSED, \
    /*LX_RIGHT*/    GAMECUBE_CODE_LX_RIGHT, \
    /*LX_LEFT*/     GAMECUBE_CODE_LX_LEFT, \
    /*LY_UP*/       GAMECUBE_CODE_LY_UP, \
    /*LY_DOWN*/     GAMECUBE_CODE_LY_DOWN, \
    /*RS*/          GAMECUBE_CODE_UNUSED, \
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
    /*LS*/          XINPUT_CODE_LS, \
    /*LX_RIGHT*/    XINPUT_CODE_LX_RIGHT, \
    /*LX_LEFT*/     XINPUT_CODE_LX_LEFT, \
    /*LY_UP*/       XINPUT_CODE_LY_UP, \
    /*LY_DOWN*/     XINPUT_CODE_LY_DOWN, \
    /*RS*/          XINPUT_CODE_RS, \
    /*RX_RIGHT*/    XINPUT_CODE_RX_RIGHT, \
    /*RX_LEFT*/     XINPUT_CODE_RX_LEFT, \
    /*RY_UP*/       XINPUT_CODE_RY_UP, \
    /*RY_DOWN*/     XINPUT_CODE_RY_DOWN  \
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
    /*LT*/          SINPUT_CODE_LT, \
    /*LTANALOG*/    SINPUT_CODE_LT_ANALOG, \
    /*RT*/          SINPUT_CODE_RT, \
    /*RTANALOG*/    SINPUT_CODE_RT_ANALOG, \
    /*LP1*/         SINPUT_CODE_UNUSED, \
    /*RP1*/         SINPUT_CODE_UNUSED, \
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
    /*LS*/          SINPUT_CODE_LS, \
    /*LX_RIGHT*/    SINPUT_CODE_LX_RIGHT, \
    /*LX_LEFT*/     SINPUT_CODE_LX_LEFT, \
    /*LY_UP*/       SINPUT_CODE_LY_UP, \
    /*LY_DOWN*/     SINPUT_CODE_LY_DOWN, \
    /*RS*/          SINPUT_CODE_RS, \
    /*RX_RIGHT*/    SINPUT_CODE_RX_RIGHT, \
    /*RX_LEFT*/     SINPUT_CODE_RX_LEFT, \
    /*RY_UP*/       SINPUT_CODE_RY_UP, \
    /*RY_DOWN*/     SINPUT_CODE_RY_DOWN  \
}

// ---------------------------------
// ---------------------------------

#endif
