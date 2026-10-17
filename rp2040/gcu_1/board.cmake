target_compile_definitions(${PROJECT_NAME} PRIVATE
    # GD25Q80: 1 MB of flash, not the 2 MB the Pico board file assumes
    PICO_FLASH_SIZE_BYTES=0x100000
)

# This file is included twice (early for build options, last for publish steps)
get_property(_gcu1_seen TARGET ${PROJECT_NAME} PROPERTY HOJA_BOARD_CMAKE_SEEN)
set_property(TARGET ${PROJECT_NAME} PROPERTY HOJA_BOARD_CMAKE_SEEN ON)

# FLOSS on the RP2040, with the ESP32 HCI bridge as its controller
if(NOT _gcu1_seen)
hoja_use_floss(${PROJECT_NAME})
endif()

# Released as the GCU R4K. Those units look for updates under builds/gcu_r4k, so publish there too,
# from the last include, once the UF2 exists.
if(_gcu1_seen)
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
endif()
