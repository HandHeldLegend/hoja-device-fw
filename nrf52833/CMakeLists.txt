cmake_minimum_required(VERSION 3.20)

# Boards on the nRF52833:
#   cmake -S nrf52833 -B nrf52833/build -G Ninja
#   cmake --build nrf52833/build
if(NOT CMAKE_TOOLCHAIN_FILE)
    set(CMAKE_TOOLCHAIN_FILE ${CMAKE_CURRENT_LIST_DIR}/toolchain.cmake)
endif()
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(nrf52833_boards C ASM)

set(CMAKE_C_FLAGS_RELEASE "-O2 -g -DNDEBUG")

set(SCRIPTS_DIR ${CMAKE_CURRENT_LIST_DIR}/../scripts)
set(ROOT_DIR    ${CMAKE_CURRENT_LIST_DIR}/..)
set(DEPS_DIR    ${CMAKE_CURRENT_LIST_DIR}/deps)

# A dependency at a pinned commit, cloned into deps/ the first time
function(hoja_fetch_dep NAME URL COMMIT)
    set(_dir ${DEPS_DIR}/${NAME})
    if(NOT EXISTS ${_dir}/.git)
        find_package(Git REQUIRED)
        message(STATUS "Fetching ${NAME} (${COMMIT})")
        execute_process(COMMAND ${GIT_EXECUTABLE} clone --filter=blob:none ${URL} ${_dir} RESULT_VARIABLE _res)
        if(_res)
            message(FATAL_ERROR "Could not clone ${URL}")
        endif()
    endif()
    execute_process(COMMAND ${GIT_EXECUTABLE} -C ${_dir} rev-parse HEAD OUTPUT_VARIABLE _head OUTPUT_STRIP_TRAILING_WHITESPACE)
    if(NOT _head MATCHES "^${COMMIT}")
        find_package(Git REQUIRED)
        execute_process(COMMAND ${GIT_EXECUTABLE} -C ${_dir} checkout --quiet ${COMMIT} RESULT_VARIABLE _res)
        if(_res)
            message(FATAL_ERROR "Could not check out ${NAME} at ${COMMIT}")
        endif()
    endif()
    set(${NAME}_DIR ${_dir} PARENT_SCOPE)
endfunction()

# nrfx 3.4.0, the commit TinyUSB 0.18 builds its nRF boards with: register headers, startup, errata
hoja_fetch_dep(nrfx https://github.com/NordicSemiconductor/nrfx.git 7c47cc0a56ce44658e6da2458e86cd8783ccc4a2)
# CMSIS 6.1.0: the Cortex-M4 core header
hoja_fetch_dep(cmsis https://github.com/ARM-software/CMSIS_6.git b0bbb0423b278ca632cfe1474eb227961d835fd2)
set(NRFX_DIR  ${nrfx_DIR})
set(CMSIS_DIR ${cmsis_DIR})

# Bluetooth LE: NimBLE's open controller, built into the firmware (the library's HOJA_NRF52_BLE)
option(HOJA_NRF52_BLE "Build the Bluetooth LE controller in" ON)
if(HOJA_NRF52_BLE)
    hoja_fetch_dep(nimble https://github.com/apache/mynewt-nimble.git 648fc0213fb7aff71f56423151e342fdd3b8f775)
    set(NIMBLE_DIR ${nimble_DIR})
endif()

# TinyUSB: the copy in the Pico SDK the other boards build with, or TINYUSB_DIR
if(NOT DEFINED TINYUSB_DIR)
    if(WIN32)
        set(_userhome $ENV{USERPROFILE})
    else()
        set(_userhome $ENV{HOME})
    endif()
    file(TO_CMAKE_PATH "${_userhome}" _userhome)
    if(DEFINED ENV{PICO_SDK_PATH})
        file(TO_CMAKE_PATH "$ENV{PICO_SDK_PATH}" _sdk)
    else()
        set(_sdk ${_userhome}/.pico-sdk/sdk/2.2.0)
    endif()
    set(TINYUSB_DIR ${_sdk}/lib/tinyusb)
endif()

set(HOJA_PLATFORM  nrf52)
set(HOJA_NRF52_MCU NRF52833_XXAA)
set(HOJA_LIB_DIR        ${CMAKE_CURRENT_LIST_DIR}/../library/HOJA-LIB-RP2040)
set(HOJA_LIBOUTPUT_DIR  ${CMAKE_CURRENT_LIST_DIR}/../library/artifacts/nrf52833)
add_subdirectory(${HOJA_LIB_DIR} ${HOJA_LIBOUTPUT_DIR})

set(BULK_PROJECTS_DIRS
    steam_controller
)

foreach(DIR ${BULK_PROJECTS_DIRS})
    get_filename_component(PROJECT_NAME "${DIR}" NAME)
    set(HOJA_BUILD_DIR ${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}/build)
    add_subdirectory(${DIR} ${HOJA_BUILD_DIR})
endforeach()
