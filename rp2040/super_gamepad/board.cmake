# BTstack on the RP2040 with the ESP32 HCI bridge
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    pico_btstack_classic
    pico_btstack_ble
    pico_btstack_flash_bank
    pico_btstack_run_loop_async_context
    pico_async_context_threadsafe_background
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    # BTstack runs in an interrupt on core 1 and shares its stack
    PICO_CORE1_STACK_SIZE=0x1000
    # GD25Q80: 1 MB of flash, not the 2 MB the Pico board file assumes
    PICO_FLASH_SIZE_BYTES=0x100000
)
