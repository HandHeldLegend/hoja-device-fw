#include "driver_define_helper.h" 
#include "rgb_define_helper.h" 

#define HOJA_BT_LOGGING_DEBUG 0 

#define HOJA_PRODUCT        "Padbox GS-C" 

// URL that will display to open a config tool
#define HOJA_WEBUSB_URL     "handheldlegend.github.io/hoja2" 
#define HOJA_MANUFACTURER   "arkodd.com" 

#define HOJA_DISABLE_TOURNEY_MACRO 1

#define HOJA_SINPUT_GAMEPAD_SUBTYPE    0 // Default
#define HOJA_SINPUT_GAMEPAD_TYPE       1 // Gamepad
#define HOJA_SINPUT_GAMEPAD_FACESTYLE  1 // Xbox face style

#define HOJA_SEWN_TYPE SEWN_LAYOUT_ABXY

// ---------------------------------
// ---------------------------------

// Disable smoothing
#define ADC_SMOOTHING_STRENGTH      0
// ---------------------------------
// ---------------------------------

// Device Information Setup 
#define HOJA_DEVICE_NAME            HOJA_PRODUCT
#define HOJA_DEVICE_MAKER           HOJA_MANUFACTURER
#define HOJA_DEVICE_MANIFEST_URL    "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/padbox_gs_c/manifest.json"
#define HOJA_DEVICE_FIRMWARE_URL    "https://raw.githubusercontent.com/HandHeldLegend/hoja-device-fw/main/builds/padbox_gs_c/padbox_gs_c.uf2"
#define HOJA_DEVICE_MANUAL_URL      "https://arkodd.com/the-padbox-gs-detailed-description/"

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
#define NESBUS_DRIVER_DATA_PIN      24
#define NESBUS_DRIVER_CLOCK_PIN     25
#define NESBUS_DRIVER_LATCH_PIN     23

// ---------------------------------
// ---------------------------------

// JOYBUS Setup
#define HOJA_JOYBUS_N64_DRIVER          JOYBUS_N64_DRIVER_HAL
#define JOYBUS_N64_DRIVER_PIO_INSTANCE  1
#define JOYBUS_N64_DRIVER_DATA_PIN      24

#define HOJA_JOYBUS_GC_DRIVER           JOYBUS_GC_DRIVER_HAL
#define JOYBUS_GC_DRIVER_PIO_INSTANCE  1
#define JOYBUS_GC_DRIVER_DATA_PIN      23

// ---------------------------------
// ---------------------------------

// INPUT Setup
// Follows convention of mapper_input_code_t
#define HOJA_INPUT_SLOTS { \
    (inputInfoSlot_s) {/*South*/.input_name="A", .input_type=INPUT_TYPE_DIGITAL, .rgb_group=4}, \
    (inputInfoSlot_s) {/*East*/.input_name="B", .input_type=INPUT_TYPE_DIGITAL, .rgb_group=3}, \
    (inputInfoSlot_s) {/*West*/.input_name="X", .input_type=INPUT_TYPE_DIGITAL, .rgb_group=5}, \
    (inputInfoSlot_s) {/*North*/.input_name="Y", .input_type=INPUT_TYPE_DIGITAL, .rgb_group=6}, \
    (inputInfoSlot_s) {/*Up*/.input_name="D Up", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*Down*/.input_name="D Down", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*Left*/.input_name="D Left", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*Right*/.input_name="D Right", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*LB*/.input_name="L↗", .input_type=INPUT_TYPE_DIGITAL, .rgb_group=1}, \
    (inputInfoSlot_s) {/*RB*/.input_name="R↗", .input_type=INPUT_TYPE_DIGITAL, .rgb_group=2}, \
    (inputInfoSlot_s) {/*LT*/.input_name="L↘", .input_type=INPUT_TYPE_DIGITAL, .rgb_group=8}, \
    (inputInfoSlot_s) {/*LTANALOG*/0}, \
    (inputInfoSlot_s) {/*RT*/.input_name="R↘", .input_type=INPUT_TYPE_DIGITAL, .rgb_group=7}, \
    (inputInfoSlot_s) {/*RTANALOG*/0}, \
    (inputInfoSlot_s) {/*LP1*/.input_name="NB1", .input_type=INPUT_TYPE_DIGITAL,}, \
    (inputInfoSlot_s) {/*RP1*/.input_name="NB2", .input_type=INPUT_TYPE_DIGITAL, .rgb_group=13}, \
    (inputInfoSlot_s) {/*LP2*/.input_name="RX", .input_type=INPUT_TYPE_DIGITAL, .rgb_group=11}, \
    (inputInfoSlot_s) {/*RP2*/.input_name="RY", .input_type=INPUT_TYPE_DIGITAL, .rgb_group=12}, \
    (inputInfoSlot_s) {/*Start*/.input_name="Start", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*Select*/.input_name="Select", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*Home*/.input_name="Home", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*Share*/.input_name="Capture", .input_type=INPUT_TYPE_DIGITAL}, \
    (inputInfoSlot_s) {/*MISC3*/.input_name="R3", .input_type=INPUT_TYPE_DIGITAL, .rgb_group=10}, \
    (inputInfoSlot_s) {/*MISC4*/0}, \
    (inputInfoSlot_s) {/*TP1*/0}, \
    (inputInfoSlot_s) {/*TP2*/0}, \
    (inputInfoSlot_s) {/*LS*/.input_name="L3", .input_type=INPUT_TYPE_DIGITAL, .rgb_group=9}, \
    (inputInfoSlot_s) {/*LX_RIGHT*/.input_name="LX+", .input_type=INPUT_TYPE_JOYSTICK}, \
    (inputInfoSlot_s) {/*LX_LEFT*/.input_name="LX-", .input_type=INPUT_TYPE_JOYSTICK}, \
    (inputInfoSlot_s) {/*LY_UP*/.input_name="LY+", .input_type=INPUT_TYPE_JOYSTICK}, \
    (inputInfoSlot_s) {/*LY_DOWN*/.input_name="LY-", .input_type=INPUT_TYPE_JOYSTICK}, \
    (inputInfoSlot_s) {/*RS*/0}, \
    (inputInfoSlot_s) {/*RX_RIGHT*/0}, \
    (inputInfoSlot_s) {/*RX_LEFT*/0}, \
    (inputInfoSlot_s) {/*RY_UP*/0}, \
    (inputInfoSlot_s) {/*RY_DOWN*/0} \
}

#define HOJA_INPUT_DEFAULTS_SWITCH { \
    /*South*/       SWITCH_CODE_A, \
    /*East*/        SWITCH_CODE_B, \
    /*West*/        SWITCH_CODE_X, \
    /*North*/       SWITCH_CODE_Y, \
    /*Up*/          SWITCH_CODE_UP, \
    /*Down*/        SWITCH_CODE_DOWN, \
    /*Left*/        SWITCH_CODE_LEFT, \
    /*Right*/       SWITCH_CODE_RIGHT, \
    /*LB*/          SWITCH_CODE_L, \
    /*RB*/          SWITCH_CODE_ZR, \
    /*LT*/          SWITCH_CODE_UNUSED, \
    /*LTANALOG*/    SWITCH_CODE_UNUSED, \
    /*RT*/          SWITCH_CODE_UNUSED, \
    /*RTANALOG*/    SWITCH_CODE_UNUSED, \
    /*LP1*/         SWITCH_CODE_ZL, \
    /*RP1*/         SWITCH_CODE_A, \
    /*LP2*/         SWITCH_CODE_RY_UP, \
    /*RP2*/         SWITCH_CODE_RX_LEFT, \
    /*Start*/       SWITCH_CODE_PLUS, \
    /*Select*/      SWITCH_CODE_MINUS, \
    /*Home*/        SWITCH_CODE_HOME, \
    /*Share*/       SWITCH_CODE_CAPTURE, \
    /*MISC3*/       SWITCH_CODE_RX_RIGHT, \
    /*MISC4*/       SWITCH_CODE_UNUSED, \
    /*TP1*/         SWITCH_CODE_UNUSED, \
    /*TP2*/         SWITCH_CODE_UNUSED, \
    /*LS*/          SWITCH_CODE_RY_DOWN, \
    /*LX_RIGHT*/    SWITCH_CODE_LX_RIGHT, \
    /*LX_LEFT*/     SWITCH_CODE_LX_LEFT, \
    /*LY_UP*/       SWITCH_CODE_LY_UP, \
    /*LY_DOWN*/     SWITCH_CODE_LY_DOWN, \
    /*RS*/          SWITCH_CODE_UNUSED, \
    /*RX_RIGHT*/    SWITCH_CODE_UNUSED, \
    /*RX_LEFT*/     SWITCH_CODE_UNUSED, \
    /*RY_UP*/       SWITCH_CODE_UNUSED, \
    /*RY_DOWN*/     SWITCH_CODE_UNUSED  \
}

#define HOJA_INPUT_DEFAULTS_SNES { \
    /*South*/       SNES_CODE_A, \
    /*East*/        SNES_CODE_B, \
    /*West*/        SNES_CODE_X, \
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
    /*East*/        N64_CODE_R, \
    /*West*/        N64_CODE_B, \
    /*North*/       N64_CODE_L, \
    /*Up*/          N64_CODE_UP, \
    /*Down*/        N64_CODE_DOWN, \
    /*Left*/        N64_CODE_LEFT, \
    /*Right*/       N64_CODE_RIGHT, \
    /*LB*/          N64_CODE_UNUSED, \
    /*RB*/          N64_CODE_UNUSED, \
    /*LT*/          N64_CODE_UNUSED, \
    /*LTANALOG*/    N64_CODE_UNUSED, \
    /*RT*/          N64_CODE_UNUSED, \
    /*RTANALOG*/    N64_CODE_UNUSED, \
    /*LP1*/         N64_CODE_Z, \
    /*RP1*/         N64_CODE_UNUSED, \
    /*LP2*/         N64_CODE_CUP, \
    /*RP2*/         N64_CODE_CLEFT, \
    /*Start*/       N64_CODE_START, \
    /*Select*/      N64_CODE_L, \
    /*Home*/        N64_CODE_UNUSED, \
    /*Share*/       N64_CODE_UNUSED, \
    /*MISC3*/       N64_CODE_CRIGHT, \
    /*MISC4*/       N64_CODE_UNUSED, \
    /*TP1*/         N64_CODE_UNUSED, \
    /*TP2*/         N64_CODE_UNUSED, \
    /*LS*/          N64_CODE_CDOWN, \
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
    /*South*/       GAMECUBE_CODE_B, \
    /*East*/        GAMECUBE_CODE_X, \
    /*West*/        GAMECUBE_CODE_R, \
    /*North*/       GAMECUBE_CODE_Y, \
    /*Up*/          GAMECUBE_CODE_UP, \
    /*Down*/        GAMECUBE_CODE_DOWN, \
    /*Left*/        GAMECUBE_CODE_LEFT, \
    /*Right*/       GAMECUBE_CODE_RIGHT, \
    /*LB*/          GAMECUBE_CODE_UNUSED, \
    /*RB*/          GAMECUBE_CODE_Z, \
    /*LT*/          MAPPER_OVERRIDE_SLOT_0, \
    /*LTANALOG*/    GAMECUBE_CODE_UNUSED, \
    /*RT*/          MAPPER_OVERRIDE_SLOT_1, \
    /*RTANALOG*/    GAMECUBE_CODE_UNUSED, \
    /*LP1*/         GAMECUBE_CODE_L, \
    /*RP1*/         GAMECUBE_CODE_A, \
    /*LP2*/         GAMECUBE_CODE_RY_UP, \
    /*RP2*/         GAMECUBE_CODE_RX_LEFT, \
    /*Start*/       GAMECUBE_CODE_START, \
    /*Select*/      GAMECUBE_CODE_UNUSED, \
    /*Home*/        GAMECUBE_CODE_UNUSED, \
    /*Share*/       GAMECUBE_CODE_UNUSED, \
    /*MISC3*/       GAMECUBE_CODE_RX_RIGHT, \
    /*MISC4*/       GAMECUBE_CODE_UNUSED, \
    /*TP1*/         GAMECUBE_CODE_UNUSED, \
    /*TP2*/         GAMECUBE_CODE_UNUSED, \
    /*LS*/          GAMECUBE_CODE_RY_DOWN, \
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

#define HOJA_MAPPER_OVERRIDE_SLOTS {\
    /*slot 0*/ (mapper_override_s){.output_code=GAMECUBE_CODE_R_ANALOG, .static_output=2048, .threshold_delta=1024},\
    /*slot 1*/ (mapper_override_s){.output_code=GAMECUBE_CODE_R_ANALOG, .static_output=410, .threshold_delta=1024},\
}

#define HOJA_INPUT_DEFAULTS_XINPUT { \
    /*South*/       XINPUT_CODE_A, \
    /*East*/        XINPUT_CODE_B, \
    /*West*/        XINPUT_CODE_X, \
    /*North*/       XINPUT_CODE_Y, \
    /*Up*/          XINPUT_CODE_UP, \
    /*Down*/        XINPUT_CODE_DOWN, \
    /*Left*/        XINPUT_CODE_LEFT, \
    /*Right*/       XINPUT_CODE_RIGHT, \
    /*LB*/          XINPUT_CODE_LB, \
    /*RB*/          XINPUT_CODE_RB, \
    /*LT*/          XINPUT_CODE_LT_ANALOG, \
    /*LTANALOG*/    XINPUT_CODE_UNUSED, \
    /*RT*/          XINPUT_CODE_RT_ANALOG, \
    /*RTANALOG*/    XINPUT_CODE_UNUSED, \
    /*LP1*/         XINPUT_CODE_LS, \
    /*RP1*/         XINPUT_CODE_LS, \
    /*LP2*/         XINPUT_CODE_RS, \
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
    /*LX_RIGHT*/    XINPUT_CODE_LX_RIGHT, \
    /*LX_LEFT*/     XINPUT_CODE_LX_LEFT, \
    /*LY_UP*/       XINPUT_CODE_LY_UP, \
    /*LY_DOWN*/     XINPUT_CODE_LY_DOWN, \
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
    /*LT*/          SINPUT_CODE_LT, \
    /*LTANALOG*/    SINPUT_CODE_UNUSED, \
    /*RT*/          SINPUT_CODE_RT, \
    /*RTANALOG*/    SINPUT_CODE_UNUSED, \
    /*LP1*/         SINPUT_CODE_LP_1, \
    /*RP1*/         SINPUT_CODE_RP_1, \
    /*LP2*/         SINPUT_CODE_LP_2, \
    /*RP2*/         SINPUT_CODE_RP_2, \
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
#define RGB_DRIVER_OUTPUT_PIN   14
#define RGB_DRIVER_LED_COUNT    32
#define RGB_DRIVER_ORDER        RGB_ORDER_GRB
#define RGB_DRIVER_PIO_INSTANCE 0

#define HOJA_RGB_GROUPS_NUM 13
#define HOJA_RGB_GROUP_NAMES { \
    {"L↗"}, \
    {"R↗"}, \
    {"B"}, \
    {"A"}, \
    {"X"}, \
    {"Y"}, \
    {"R↘"}, \
    {"L↘"}, \
    {"L3"}, \
    {"R3"}, \
    {"RX"}, \
    {"RY"}, \
    {"NB2"}, \
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
    RGB_GROUP(7), \
    RGB_GROUP(9), \
    RGB_GROUP(12), \
    RGB_GROUP(11), \
    RGB_GROUP(10), \
    RGB_GROUP(8), \
}
// ---------------------------------
// ---------------------------------
