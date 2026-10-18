target_compile_definitions(${PROJECT_NAME} PRIVATE
    PICO_FLASH_SIZE_BYTES=0x200000
)

# The HOJA bootloader in front of the firmware (WinUSB update mode; the board has no LEDs)
hoja_use_bootloader(${PROJECT_NAME} "Phob 2" 0x200000 0)
