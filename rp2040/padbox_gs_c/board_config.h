#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "driver_define_helper.h"
#include "rgb_define_helper.h"

#define HOJA_BT_LOGGING_DEBUG 0
#define HOJA_WEBUSB_URL     "handheldlegend.github.io/hoja2"

// HOJA Transport driver gates. Platform Edition PadBox GS: wired USB plus
// SNES (NESBUS) and N64/GameCube (Joybus) retro-console output.
#define HOJA_TRANSPORT_USB_DRIVER       USB_DRIVER_HAL
#define HOJA_TRANSPORT_JOYBUS64_DRIVER  JOYBUS_N64_DRIVER_HAL
#define HOJA_TRANSPORT_JOYBUSGC_DRIVER  JOYBUS_GC_DRIVER_HAL
#define HOJA_TRANSPORT_NESBUS_DRIVER    NESBUS_DRIVER_HAL

#define ADC_SMOOTHING_STRENGTH      0

// RGB gate + compile-time sizing. Layout/groups/key-mappings live in main.c.
#define HOJA_RGB_DRIVER             RGB_DRIVER_HAL
#define RGB_DRIVER_LED_COUNT        32
#define RGB_DRIVER_ORDER            RGB_ORDER_GRB

#endif
