# THIS IS MEAN TO BE COPIED. DO NOT EDIT UNLES YOU ARE IN THE SCRIPTS FOLDER

cmake_minimum_required(VERSION 3.13)

if(NOT DEFINED PICO_SDK_INIT_DONE)
    include($ENV{PICO_SDK_PATH}/pico_sdk_init.cmake)
    set(CMAKE_C_STANDARD 11)
    set(CMAKE_CXX_STANDARD 17)
endif()

if(NOT DEFINED PROJECT_NAME)
    get_filename_component(PROJECT_NAME "${CMAKE_CURRENT_LIST_DIR}" NAME)
endif()

if(NOT DEFINED PICO_SDK_INIT_DONE)
    project(${PROJECT_NAME} C CXX ASM)
    pico_sdk_init()
else()
    project(${PROJECT_NAME})
endif()

if(NOT DEFINED HOJA_LIB_ADDED)
    set(HOJA_LIB_DIR        ${CMAKE_CURRENT_LIST_DIR}/../../library/HOJA-LIB-RP2040)
    set(HOJA_LIBOUTPUT_DIR  ${CMAKE_CURRENT_LIST_DIR}/../../library/artifacts)
    add_subdirectory(${HOJA_LIB_DIR} ${HOJA_LIBOUTPUT_DIR})
endif()

if(NOT DEFINED SCRIPTS_DIR)
    set(SCRIPTS_DIR ${CMAKE_CURRENT_LIST_DIR}/../../scripts)
endif()

if(NOT DEFINED HOJA_BUILD_DIR)
    set(HOJA_BUILD_DIR   ${CMAKE_CURRENT_LIST_DIR}/build)
endif()

if(NOT DEFINED HOJA_BUILDS_PROJECT_DIR)
    set(HOJA_BUILDS_PROJECT_DIR ${CMAKE_CURRENT_LIST_DIR}/../../builds/${PROJECT_NAME})
endif()

add_custom_target(
    ${PROJECT_NAME}_TIMESTAMP_GEN ALL
    COMMAND ${CMAKE_COMMAND} 
    -DOUTPUTDIR=${CMAKE_CURRENT_LIST_DIR}
    -P ${SCRIPTS_DIR}/timestamp.cmake
    BYPRODUCTS ${CMAKE_CURRENT_LIST_DIR}/timestamp.h
    COMMENT "Generating build timestamp"
    VERBATIM
)

# Create a custom target that depends on the timestamp
add_custom_target(${PROJECT_NAME}_TIMESTAMP ALL
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/timestamp.h
)

# Rest of your existing CMake file remains the same
add_executable(${PROJECT_NAME}
    main.c
)

if(EXISTS "${CMAKE_CURRENT_LIST_DIR}/pico_override.cmake")
    message("Including pico_override.cmake for ${PROJECT_NAME}")
    include(${CMAKE_CURRENT_LIST_DIR}/pico_override.cmake)
    target_compile_definitions(${PROJECT_NAME}
        PRIVATE
        CYW43_DEFAULT_PIN_WL_REG_ON=${BT_ON_PIN}
        CYW43_DEFAULT_PIN_WL_DATA_OUT=${BT_DATA_OUT_PIN}
        CYW43_DEFAULT_PIN_WL_DATA_IN=${BT_DATA_IN_PIN}
        CYW43_DEFAULT_PIN_WL_HOST_WAKE=${BT_WAKE_PIN}
        CYW43_DEFAULT_PIN_WL_CLOCK=${BT_CLOCK_PIN}
        CYW43_DEFAULT_PIN_WL_CS=${BT_CS_PIN}
    )
endif()

target_compile_definitions(${PROJECT_NAME}
    PRIVATE
    PICO_FLASH_SAFE_EXECUTE_PICO_SUPPORT_MULTICORE_LOCKOUT=1
    PICO_XOSC_STARTUP_DELAY_MULTIPLIER=64
    CYW43_LWIP=0
    )

# Make your targets depend on the timestamp
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_TIMESTAMP)

# Use the passed-in build directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${HOJA_BUILD_DIR}
    RUNTIME_OUTPUT_DIRECTORY ${HOJA_BUILD_DIR}
    ARCHIVE_OUTPUT_DIRECTORY ${HOJA_BUILD_DIR}
    LIBRARY_OUTPUT_DIRECTORY ${HOJA_BUILD_DIR}
)

include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
        hoja_lib
    )

pico_enable_stdio_usb(${PROJECT_NAME} 0)
pico_enable_stdio_uart(${PROJECT_NAME} 0)
pico_add_extra_outputs(${PROJECT_NAME})

set(CMAKE_EXE_LINKER_FLAGS "-Wl,--print-memory-usage")

add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} 
    -DSOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
    -DUF2_SOURCE=${HOJA_BUILD_DIR}/${PROJECT_NAME}.uf2
    -DBIN_SOURCE=${HOJA_BUILD_DIR}/${PROJECT_NAME}.bin

    -DUF2_TARGET=${HOJA_BUILDS_PROJECT_DIR}
    -DBIN_TARGET=${HOJA_BUILDS_PROJECT_DIR}
    -DMANIFEST_TARGET=${HOJA_BUILDS_PROJECT_DIR}

    -P ${SCRIPTS_DIR}/manifest.cmake
    COMMENT "Moving build files and generating manifest.json"
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
)