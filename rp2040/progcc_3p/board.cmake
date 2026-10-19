target_compile_definitions(${PROJECT_NAME} PRIVATE
    # GD25Q80: 1 MB of flash, not the 2 MB the Pico board file assumes
    PICO_FLASH_SIZE_BYTES=0x100000
)

# FLOSS on the RP2040, with the ESP32 HCI bridge as its controller
hoja_use_floss(${PROJECT_NAME})
# The HOJA bootloader in front of the firmware (WinUSB update mode, its LEDs while it works). No
# radio region: the radio is the ESP32, with its own firmware.
hoja_use_bootloader(${PROJECT_NAME} "ProGCC 3+" 0x100000 0 RGB_PIN 23 RGB_LEDS 24)
