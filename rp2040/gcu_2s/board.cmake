target_compile_definitions(${PROJECT_NAME} PRIVATE
    # GD25Q80: 1 MB of flash, not the 2 MB the Pico board file assumes (measured: past 1 MB it wraps to 0)
    PICO_FLASH_SIZE_BYTES=0x100000
)

# The code every report runs, from RAM instead of flash (before the bootloader patches the script)
hoja_use_report_path_in_ram(${PROJECT_NAME})
# The HOJA bootloader in front of the firmware (WinUSB update mode; its LEDs while it works)
hoja_use_bootloader(${PROJECT_NAME} "GCU 2S" 0x100000 0 RGB_PIN 27 RGB_LEDS 17)
