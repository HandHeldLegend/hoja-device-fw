# GNU Arm toolchain for the nRF52833 (Cortex-M4 with a single-precision FPU). The compiler comes from
# PATH, or from the Raspberry Pi Pico extension's install (the same one the RP2040 boards build with).
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR arm)

if(WIN32)
    set(_userhome $ENV{USERPROFILE})
    set(_exe .exe)
else()
    set(_userhome $ENV{HOME})
    set(_exe "")
endif()
file(TO_CMAKE_PATH "${_userhome}" _userhome)

find_program(ARM_GCC arm-none-eabi-gcc${_exe}
    HINTS
        ${_userhome}/.pico-sdk/toolchain/14_2_Rel1/bin
        ${_userhome}/.pico-sdk/toolchain/15_2_Rel1/bin
)
if(NOT ARM_GCC)
    message(FATAL_ERROR "arm-none-eabi-gcc not found (PATH, or ~/.pico-sdk/toolchain)")
endif()
get_filename_component(_bin ${ARM_GCC} DIRECTORY)

set(CMAKE_C_COMPILER   ${_bin}/arm-none-eabi-gcc${_exe})
set(CMAKE_CXX_COMPILER ${_bin}/arm-none-eabi-g++${_exe})
set(CMAKE_ASM_COMPILER ${_bin}/arm-none-eabi-gcc${_exe})
set(CMAKE_OBJCOPY      ${_bin}/arm-none-eabi-objcopy${_exe} CACHE FILEPATH "")
set(CMAKE_SIZE         ${_bin}/arm-none-eabi-size${_exe} CACHE FILEPATH "")

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(_cpu "-mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16")
set(CMAKE_C_FLAGS_INIT   "${_cpu} -ffunction-sections -fdata-sections -fno-common")
set(CMAKE_CXX_FLAGS_INIT "${_cpu} -ffunction-sections -fdata-sections -fno-common")
set(CMAKE_ASM_FLAGS_INIT "${_cpu} -x assembler-with-cpp")
set(CMAKE_EXE_LINKER_FLAGS_INIT "${_cpu}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
