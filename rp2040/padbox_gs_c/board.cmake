target_compile_definitions(${PROJECT_NAME} PRIVATE
    PICO_FLASH_SIZE_BYTES=0x200000
)

# The HOJA bootloader in front of the firmware (WinUSB update mode; its LEDs while it works)
hoja_use_bootloader(${PROJECT_NAME} "Padbox GS-C" 0x200000 0 RGB_PIN 14 RGB_LEDS 13)
