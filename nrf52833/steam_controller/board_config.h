#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "driver_define_helper.h"
#include "rgb_define_helper.h"

// Valve Steam Controller (2026): nRF52833. USB, and Bluetooth LE from the chip's own radio.

#define HOJA_TRANSPORT_USB_DRIVER   USB_DRIVER_HAL

// The radio carries SInput, as a Bluetooth LE HID gamepad, and Switch, as a Switch 2's Pro
// Controller 2 (a build with HOJA_NS2_BLE). A build without the controller (HOJA_NRF52_BLE off) has
// no wireless mode at all.
#if defined(HOJA_NRF52_BLE)
#define HOJA_TRANSPORT_BT_DRIVER    BT_DRIVER_NRF52
#endif

#define HOJA_IMU_DRIVER             IMU_DRIVER_LSM6DSV16X
#define ADC_SMOOTHING_STRENGTH      0

// The two grip motors, behind the I2S amplifier
#define HOJA_HAPTICS_DRIVER         HAPTICS_DRIVER_LRA_HAL

#define HOJA_BATTERY_DRIVER         BATTERY_DRIVER_MP2733
#define HOJA_FUELGAUGE_DRIVER       FUELGAUGE_DRIVER_ADC

// The status LED by the Steam button
#define HOJA_RGB_DRIVER             RGB_DRIVER_HAL
#define RGB_DRIVER_LED_COUNT        1
#define RGB_DRIVER_ORDER            RGB_ORDER_RGB

// Hover (analog) inputs whose reading falls as they are pressed, read by hover.c: both triggers
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

// What the firmware saves (flash_layout.h): 48 KB below the page Valve's bootloader keeps the
// firmware's header in. steam_controller.ld ends the firmware where this starts. The bootloader
// erases it with the firmware on every update.
#define NRF52_FLASH_STORAGE_OFFSET  0x70000u
#define NRF52_FLASH_STORAGE_SECTORS 12u

#endif
