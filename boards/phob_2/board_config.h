#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#include "driver_define_helper.h"
#include "rgb_define_helper.h"

#define HOJA_BT_LOGGING_DEBUG 0

#define HOJA_PRODUCT        "Phob 2"

// URL that will display to open a config tool
#define HOJA_WEBUSB_URL     "handheldlegend.github.io/hoja2"
#define HOJA_MANUFACTURER   "PhobGCC"

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

// MCP3002 (2 of them)
extern adc_driver_cfg_s gcp_adc_1;
extern adc_driver_cfg_s gcp_adc_2;

// HAL ADC (1 instance)
extern adc_driver_cfg_s gcp_adc_hal;

#define HOJA_ADC_LX_CFG (adc_channel_cfg_s) {.ch_local = 0, .driver_cfg = &gcp_adc_1} 
#define HOJA_ADC_LY_CFG (adc_channel_cfg_s) {.ch_local = 1, .driver_cfg = &gcp_adc_1} 

#define HOJA_ADC_RX_CFG (adc_channel_cfg_s) {.ch_local = 0, .driver_cfg = &gcp_adc_2} 
#define HOJA_ADC_RY_CFG (adc_channel_cfg_s) {.ch_local = 1, .driver_cfg = &gcp_adc_2} 

#define HOJA_ADC_LT_CFG (adc_channel_cfg_s) {.ch_local = 1, .driver_cfg = &gcp_adc_hal, .ch_invert = 0} 
#define HOJA_ADC_RT_CFG (adc_channel_cfg_s) {.ch_local = 0, .driver_cfg = &gcp_adc_hal, .ch_invert = 0} 

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
#define HOJA_DEVICE_FCC_ELABEL      ""
#define HOJA_DEVICE_SNES_SUPPORTED      0
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
#define HOJA_BUTTONS_SUPPORTED_SYSTEM   0b001 // Home, Capture, Sync
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

#endif
