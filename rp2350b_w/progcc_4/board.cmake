target_compile_definitions(${PROJECT_NAME} PRIVATE
    # RP2354B: 2 MB of flash stacked in the package (the QSPI pins aren't wired), not the 4 MB the
    # Latte Pro board file assumes
    PICO_FLASH_SIZE_BYTES=0x200000
)

# FLOSS as the Bluetooth host stack instead of BTstack
hoja_use_floss(${PROJECT_NAME})
# The HOJA bootloader in front of the firmware (updates over the air, WinUSB update mode). The RM2's
# firmware gets the same 256 KB region as the GCU 2.
hoja_use_bootloader(${PROJECT_NAME} "ProGCC 4" 0x200000 0x40000 RGB_PIN 35 RGB_LEDS 20)
