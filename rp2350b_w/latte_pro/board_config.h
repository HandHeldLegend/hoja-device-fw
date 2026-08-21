#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "driver_define_helper.h"
#include "rgb_define_helper.h"

#define HOJA_BT_LOGGING_DEBUG 0

// ---------------------------------
// ---------------------------------

// HOJA Transport driver gates.
// Each define selects WHICH platform HAL gets compiled/linked for that transport
// and simultaneously marks the transport as supported: transport_init() infers
// availability directly from the presence of these defines (no separate list).
#define HOJA_TRANSPORT_BT_DRIVER        BT_DRIVER_HAL
#define HOJA_TRANSPORT_USB_DRIVER       USB_DRIVER_HAL
#define HOJA_TRANSPORT_JOYBUS64_DRIVER  JOYBUS_N64_DRIVER_HAL
#define HOJA_TRANSPORT_JOYBUSGC_DRIVER  JOYBUS_GC_DRIVER_HAL
#define HOJA_TRANSPORT_WLAN_DRIVER      WLAN_DRIVER_HAL
#define HOJA_TRANSPORT_NESBUS_DRIVER    NESBUS_DRIVER_HAL

// IMU Driver Setup
// Gate: selects WHICH IMU driver compiles + shapes hoja_config_s.imu.
#define HOJA_IMU_DRIVER             IMU_DRIVER_LSM6DSR

// Disable smoothing
#define ADC_SMOOTHING_STRENGTH      0

// Haptic Driver
#define HOJA_HAPTICS_DRIVER         HAPTICS_DRIVER_LRA_HAL

// Battery Driver
#define HOJA_BATTERY_DRIVER         BATTERY_DRIVER_BQ25180

// Fuel Gauge Driver
#define HOJA_FUELGAUGE_DRIVER       FUELGAUGE_DRIVER_BQ27621G1

// ---------------------------------
// ---------------------------------

// RGB Setup (gate + compile-time sizing only).
// Group names, LED membership, notification/player indicators and key_mappings
// all live in hoja_config_s.rgb / .rgb_driver (see main.c).
#define HOJA_RGB_DRIVER         RGB_DRIVER_HAL
#define RGB_DRIVER_LED_COUNT    24
#define RGB_DRIVER_ORDER        RGB_ORDER_GRB

// Hover (analog face/trigger/dpad) polarity. Still read from board_config.h
// by hover.c; it did not move into hoja_config_s.
#define HOJA_HOVER_INVERTS {\
    /*South*/   0,\
    /*East*/    0,\
    /*West*/    0,\
    /*North*/   0,\
    /*Up*/      0,\
    /*Down*/    0,\
    /*Left*/    0,\
    /*Right*/   0,\
    /*LB*/      1,\
    /*RB*/      1,\
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

// ---------------------------------
// ---------------------------------

#endif
