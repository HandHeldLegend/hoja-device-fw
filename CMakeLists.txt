cmake_minimum_required(VERSION 3.13)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(EXTERNAL_BOARD_DEF_DIR ${CMAKE_CURRENT_LIST_DIR}/picoboards)
set(PICO_BOARD_HEADER_DIRS ${EXTERNAL_BOARD_DEF_DIR} )

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

include(pico_sdk_import.cmake)
project(hoja-device-fw)

include(ExternalProject)

# Set up peripheral directories
set(SCRIPTS_DIR ${CMAKE_CURRENT_LIST_DIR}/scripts)
set(BOARDS_DIR  ${CMAKE_CURRENT_LIST_DIR}/boards)
set(ROOT_DIR    ${CMAKE_CURRENT_LIST_DIR})

set(USE_EXTERNAL_BOARD_DEF TRUE)

set(MY_LIST 
    progcc_3:pico
    progcc_3.2:pico
    progcc_3s:lattepro_w
    gcu_r4k:pico
    gcu_r5:gcur5_w
    gcu_s1:pico
    latte_jr:pico2
    latte_pro:lattepro_w
    padbox_gs:pico
    padbox_gs_c:pico
)

set(PICO_SDK_INIT_PATH ${CMAKE_CURRENT_LIST_DIR})

foreach(ITEM ${MY_LIST})
    # This regex matches:
    # 1. ^([^:]+)  -> Everything from the start that is NOT a colon (Group 1: BOARD_NAME)
    # 2. :         -> The literal colon separator
    # 3. (.+)$     -> Everything remaining until the end (Group 2: BOARD_TYPE)
    string(REGEX MATCH "^([^:]+):(.+)$" MATCHED "${ITEM}")
    
    if(MATCHED)
        set(BOARD_NAME "${CMAKE_MATCH_1}")
        set(BOARD_TYPE "${CMAKE_MATCH_2}")

        file(COPY "${SCRIPTS_DIR}/CMakeLists.txt"
             DESTINATION "${BOARDS_DIR}/${BOARD_NAME}")
        
        ExternalProject_Add(${BOARD_NAME}_${BOARD_TYPE}
            PREFIX ${BOARD_NAME}
            SOURCE_DIR "${BOARDS_DIR}/${BOARD_NAME}"
            BINARY_DIR "${CMAKE_BINARY_DIR}/subprojects/${BOARD_NAME}"
            CMAKE_ARGS
                "--no-warn-unused-cli"
                "-DUSE_EXTERNAL_BOARD_DEF=TRUE"
                "-DPICO_SDK_PATH=${PICO_SDK_PATH}"
                "-DEXTERNAL_BOARD_DEF_DIR=${EXTERNAL_BOARD_DEF_DIR}"
                "-DCMAKE_BUILD_TYPE=Debug"
                "-DPICO_SDK_INIT_PATH=${ROOT_DIR}"
                "-DTARGET_BOARD=${BOARD_TYPE}"
                "-DTARGET_NAME=${BOARD_NAME}"
                "-DPICO_DEOPTIMIZED_DEBUG=0"
            BUILD_ALWAYS 1 # force dependency checking
            INSTALL_COMMAND ""
            )

    endif()
endforeach()



# Iterate through the list
#foreach(DIR ${BULK_PROJECTS_DIRS})
#    get_filename_component(PROJECT_NAME "${DIR}" NAME)
#
#    # Copy CMakeLists
#    file(COPY "${SCRIPTS_DIR}/CMakeLists.txt"
#     DESTINATION "${ROOT_DIR}/boards/${PROJECT_NAME}")
#
#    # Set the build folder for this project
#    set(HOJA_BUILD_DIR ${ROOT_DIR}/boards/${PROJECT_NAME}/build)
#    # Create builds directory for this project
#    set(HOJA_BUILDS_PROJECT_DIR ${CMAKE_CURRENT_LIST_DIR}/builds/${PROJECT_NAME})
#    # Make the dir if it doesn't exist
#    file(MAKE_DIRECTORY ${HOJA_BUILDS_PROJECT_DIR})
#
#    # Add/Run this CMAKE
#    add_subdirectory(${DIR} ${HOJA_BUILD_DIR})
#endforeach()