#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "driver_define_helper.h"
#include "rgb_define_helper.h"

#define HOJA_BT_LOGGING_DEBUG 0

#define HOJA_TASK_BENCHMARKING 1

// ---------------------------------
// ---------------------------------

// HOJA Transport driver gates.
// GCU-S1 is USB + wired retro buses only (no BT / WLAN).
#define HOJA_TRANSPORT_USB_DRIVER       USB_DRIVER_HAL
#define HOJA_TRANSPORT_JOYBUS64_DRIVER  JOYBUS_N64_DRIVER_HAL
#define HOJA_TRANSPORT_JOYBUSGC_DRIVER  JOYBUS_GC_DRIVER_HAL
#define HOJA_TRANSPORT_NESBUS_DRIVER    NESBUS_DRIVER_HAL

// IMU Driver Setup
// Gate: selects WHICH IMU driver compiles + shapes hoja_config_s.imu.
#define HOJA_IMU_DRIVER             IMU_DRIVER_LSM6DSR

// Disable smoothing
#define ADC_SMOOTHING_STRENGTH      0

// Haptic Driver
#define HOJA_HAPTICS_DRIVER         HAPTICS_DRIVER_LRA_HAL

// ---------------------------------
// ---------------------------------

// RGB Setup (gate + compile-time sizing only).
// Group layout lives in hoja_config_s.rgb / .rgb_driver (see main.c).
#define HOJA_RGB_DRIVER         RGB_DRIVER_HAL
//#define RGB_DRIVER_LED_COUNT    32
#define RGB_DRIVER_ORDER        RGB_ORDER_GRB

// ---------------------------------
// ---------------------------------

#endif
