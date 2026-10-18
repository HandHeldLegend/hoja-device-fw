target_compile_definitions(${PROJECT_NAME} PRIVATE
    # GD25Q80: 1 MB of flash, not the 2 MB the Pico W board file assumes (past 1 MB it wraps to 0)
    PICO_FLASH_SIZE_BYTES=0x100000
)

# FLOSS as the Bluetooth host stack instead of BTstack
hoja_use_floss(${PROJECT_NAME})
# The HOJA bootloader in front of the firmware (updates over the air, WinUSB update mode)
hoja_use_bootloader(${PROJECT_NAME} "GCU 2" 0x100000 0x40000)
