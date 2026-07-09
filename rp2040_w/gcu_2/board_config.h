#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "driver_define_helper.h" 
#include "rgb_define_helper.h" 

#define HOJA_BT_LOGGING_DEBUG 0 

#define HOJA_TASK_BENCHMARKING 1

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
// map all live in hoja_config_s.rgb / .rgb_driver (see main.c).
// PIO block + state machine are allocated dynamically by the RGB driver.
#define HOJA_RGB_DRIVER         RGB_DRIVER_HAL
// LED count sizes the animation buffers; color order shapes rgb_s. Both are
// optional (library defaults: 32 LEDs, GRB order).
#define RGB_DRIVER_LED_COUNT    32
#define RGB_DRIVER_ORDER        RGB_ORDER_GRB
// ---------------------------------
// ---------------------------------

#endif
