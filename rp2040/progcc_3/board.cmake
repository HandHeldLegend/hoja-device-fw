target_compile_definitions(${PROJECT_NAME} PRIVATE
    PICO_FLASH_SIZE_BYTES=0x200000
)

# The code every report runs, from RAM instead of flash (before the bootloader patches the script)
hoja_use_report_path_in_ram(${PROJECT_NAME})
# The HOJA bootloader in front of the firmware (WinUSB update mode; its LEDs while it works)
hoja_use_bootloader(${PROJECT_NAME} "ProGCC 3" 0x200000 0 RGB_PIN 15 RGB_LEDS 24)
