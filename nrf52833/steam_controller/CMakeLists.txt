cmake_minimum_required(VERSION 3.20)

# Valve Steam Controller (2026). The output is steam_controller.fw: the firmware in the container
# Valve's bootloader takes over USB (scripts/steam_controller_fw.py packs and sends it).

add_custom_target(
    ${PROJECT_NAME}_TIMESTAMP_GEN ALL
    COMMAND ${CMAKE_COMMAND}
    -DOUTPUTDIR=${CMAKE_CURRENT_LIST_DIR}
    -P ${SCRIPTS_DIR}/timestamp.cmake
    BYPRODUCTS ${CMAKE_CURRENT_LIST_DIR}/timestamp.h
    COMMENT "Generating build timestamp"
    VERBATIM
)

add_executable(${PROJECT_NAME}
    main.c
)
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_TIMESTAMP_GEN)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# Bench: a Pro Controller 2 for a Switch 2 over Bluetooth LE, started beside USB. With it comes the
# log read over the config interface (HOJA_BENCH_LOG): see "Bluetooth (bench)" in README.md.
option(STEAM_CONTROLLER_NS2_BLE_TEST "Bench: a Pro Controller 2 for a Switch 2 over Bluetooth LE, beside USB" OFF)
if(STEAM_CONTROLLER_NS2_BLE_TEST)
    target_compile_definitions(${PROJECT_NAME} PRIVATE STEAM_CONTROLLER_NS2_BLE_TEST=1 HOJA_BENCH_LOG=1)
endif()

option(STEAM_CONTROLLER_BLE_SNOOP "Bench: with the test above, every HCI packet in the log (slows the main loop)" OFF)
if(STEAM_CONTROLLER_BLE_SNOOP)
    target_compile_definitions(${PROJECT_NAME} PRIVATE STEAM_CONTROLLER_BLE_SNOOP=1)
endif()

set(STEAM_CONTROLLER_BLE_RETURN_S 0 CACHE STRING
    "Bench: with the test above, the Bluetooth mode restarts back to USB when its console lets go, or after this many seconds with none (0: stays)")
if(STEAM_CONTROLLER_BLE_RETURN_S)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        STEAM_CONTROLLER_BLE_RETURN_S=${STEAM_CONTROLLER_BLE_RETURN_S})
endif()

hoja_nrf52_firmware(${PROJECT_NAME} ${CMAKE_CURRENT_LIST_DIR}/steam_controller.ld)

find_package(Python3 REQUIRED COMPONENTS Interpreter)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${Python3_EXECUTABLE} ${SCRIPTS_DIR}/steam_controller_fw.py pack
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.bin
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/${PROJECT_NAME}.fw
    COMMENT "Packing ${PROJECT_NAME}.fw"
    VERBATIM
)
