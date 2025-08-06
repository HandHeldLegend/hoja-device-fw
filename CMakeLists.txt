cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.1.1)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.1.1)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pico_w CACHE STRING "Board type")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)
set(PICO_SDK_INCLUSION_DONE TRUE)

# Project name for the parent
project(hoja_bulk_compile C CXX ASM)

if (PICO_SDK_VERSION_STRING VERSION_LESS "2.1.0")
    message(FATAL_ERROR "Raspberry Pi Pico SDK version 2.1.0 (or later) required. Your version is ${PICO_SDK_VERSION_STRING}")
endif()

# Initialize the SDK
pico_sdk_init()
set(PICO_SDK_INIT_DONE TRUE)

# Include HOJA Libarary
set(HOJA_LIB_DIR        ${CMAKE_CURRENT_LIST_DIR}/library/HOJA-LIB-RP2040)
set(HOJA_LIBOUTPUT_DIR  ${CMAKE_CURRENT_LIST_DIR}/library/artifacts)
add_subdirectory(${HOJA_LIB_DIR} ${HOJA_LIBOUTPUT_DIR})
set(HOJA_LIB_ADDED TRUE)

# Set up peripheral directories
set(SCRIPTS_DIR ${CMAKE_CURRENT_LIST_DIR}/scripts)
set(BOARDS_DIR  ${CMAKE_CURRENT_LIST_DIR}/boards)
set(ROOT_DIR    ${CMAKE_CURRENT_LIST_DIR})

# Define your list of directories
set(BULK_PROJECTS_DIRS 
    boards/progcc_3 
    boards/progcc_3p 
    boards/progcc_3.1 
    boards/progcc_3.2 
    boards/progcc_3.3 
    boards/progcc_4.0 
    boards/super_gamepad 
    boards/gcu_r4k 
    boards/gcu_proto 
    boards/gcu_s1 
    boards/phob_2 
    boards/pico_w 
    boards/phobri_64 
    boards/padbox_gs 
)

# Iterate through the list
foreach(DIR ${BULK_PROJECTS_DIRS})
    get_filename_component(PROJECT_NAME "${DIR}" NAME)

    # Copy CMakeLists
    file(COPY "${SCRIPTS_DIR}/CMakeLists.txt"
     DESTINATION "${ROOT_DIR}/boards/${PROJECT_NAME}")

    # Set the build folder for this project
    set(HOJA_BUILD_DIR ${ROOT_DIR}/boards/${PROJECT_NAME}/build)
    # Create builds directory for this project
    set(HOJA_BUILDS_PROJECT_DIR ${CMAKE_CURRENT_LIST_DIR}/builds/${PROJECT_NAME})
    # Make the dir if it doesn't exist
    file(MAKE_DIRECTORY ${HOJA_BUILDS_PROJECT_DIR})

    # Add/Run this CMAKE
    add_subdirectory(${DIR} ${HOJA_BUILD_DIR})
endforeach()