target_compile_definitions(${PROJECT_NAME} PRIVATE
    # 2 MB of flash, not the 4 MB the Pico 2 W board file assumes (measured: past 2 MB it wraps to 0)
    PICO_FLASH_SIZE_BYTES=0x200000
)

# The code every report runs, from RAM instead of flash (before the bootloader patches the script)
hoja_use_report_path_in_ram(${PROJECT_NAME})
# The HOJA bootloader in front of the firmware (WinUSB update mode, its LEDs while it works). No
# radio region: the board has no radio.
hoja_use_bootloader(${PROJECT_NAME} "ProGCC 3S" 0x200000 0 RGB_PIN 24 RGB_LEDS 32)
