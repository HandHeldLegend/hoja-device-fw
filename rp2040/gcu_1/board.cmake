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

# Released as the GCU R4K. Those units look for updates under builds/gcu_r4k, so publish there too.
add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND}
    -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
    -DUF2_SOURCE=${HOJA_BUILD_DIR}/${PROJECT_NAME}.uf2
    -DBIN_SOURCE=${HOJA_BUILD_DIR}/${PROJECT_NAME}.bin
    -DUF2_TARGET=${BUILDS_DIR}/gcu_r4k
    -DBIN_TARGET=${BUILDS_DIR}/gcu_r4k
    -DMANIFEST_TARGET=${BUILDS_DIR}/gcu_r4k
    -DTARGET_NAME=gcu_r4k
    -P ${SCRIPTS_DIR}/manifest.cmake
    COMMENT "Publishing ${PROJECT_NAME} for GCU R4K units"
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)
