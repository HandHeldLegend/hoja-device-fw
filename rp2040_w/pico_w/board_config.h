#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "driver_define_helper.h"
#include "rgb_define_helper.h"

#define HOJA_BT_LOGGING_DEBUG 0
#define HOJA_WEBUSB_URL     "handheldlegend.github.io/hoja2"

// HOJA Transport driver gates. Pico-W test build: onboard CYW43 Bluetooth + USB.
#define HOJA_TRANSPORT_BT_DRIVER        BT_DRIVER_HAL
#define HOJA_TRANSPORT_USB_DRIVER       USB_DRIVER_HAL

// Bluetooth Driver Setup
#define BLUETOOTH_DRIVER_PIO_INSTANCE   0

#define ADC_SMOOTHING_STRENGTH      0

#endif
