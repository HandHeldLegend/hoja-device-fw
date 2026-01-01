# THIS IS MEAN TO BE COPIED. DO NOT EDIT UNLES YOU ARE IN THE SCRIPTS FOLDER

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(PICO_BOARD ${TARGET_BOARD})
message("${TARGET_BOARD}")

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================

include(${PICO_SDK_INIT_PATH}/pico_sdk_import.cmake)

if (PICO_SDK_VERSION_STRING VERSION_LESS "2.2.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 2.2.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

if(DEFINED USE_EXTERNAL_BOARD_DEF)
    set(PICO_BOARD_HEADER_DIRS ${EXTERNAL_BOARD_DEF_DIR} )
endif()

project(${TARGET_NAME} C CXX ASM)

pico_sdk_init()

if (PICO_BOARD MATCHES "_w")
    set(HOJA_USE_BT_HAL TRUE)
else()
    set(HOJA_USE_BT_HAL FALSE)
endif()

set(HOJA_LIB_DIR        ${CMAKE_CURRENT_LIST_DIR}/../../library/HOJA-LIB-RP2040)
set(HOJA_LIBOUTPUT_DIR  ${CMAKE_CURRENT_LIST_DIR}/../../library/artifacts/${TARGET_BOARD})
add_subdirectory(${HOJA_LIB_DIR} ${HOJA_LIBOUTPUT_DIR})

set(SCRIPTS_DIR ${CMAKE_CURRENT_LIST_DIR}/../../scripts)

set(HOJA_BUILD_DIR   ${CMAKE_CURRENT_LIST_DIR}/build)

set(HOJA_BUILDS_PROJECT_DIR ${CMAKE_CURRENT_LIST_DIR}/../../builds/${PROJECT_NAME})

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