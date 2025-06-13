# SPDX-License-Identifier: LicenseRef-TridentMSLA
# SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
cmake_minimum_required(VERSION 3.23.5)

project(bootloader
        VERSION 1.0
        LANGUAGES C ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(APP_NAME "bootloader")
set(CHIP_PN ${PLATFORM}${FLASH_SIZE})

if(NOT DEFINED TRIDENT_SDK_ROOT)
    set(TRIDENT_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
endif()

set(TRIDENT_PLATFORM_PATH "${TRIDENT_SDK_ROOT}/platform/${PLATFORM}")
set(TRIDENT_FRAMEWORK_PATH "${TRIDENT_SDK_ROOT}/framework")

# Set global compile definitions that will propagate to all targets
set(GLOBAL_COMPILE_DEFINITIONS
    TR_PLATFORM_${CHIP_PN}
    # [USER HELP] Add additional compile definitions here
)

if(CMAKE_BUILD_TYPE MATCHES Release)
    set(PRINT_ENABLE "0")
elseif(CMAKE_BUILD_TYPE MATCHES Debug)
    set(PRINT_ENABLE "1")
endif()

if(DEFINED PRINT_ENABLE)
    list(APPEND GLOBAL_COMPILE_DEFINITIONS PRINT_ENABLE=${PRINT_ENABLE})
endif()

################################################################################
# Bootloader Platform Library Definition - Built only once
################################################################################
if(${CHIP_PN} STREQUAL "T32CM11C")
    message(STATUS "T32CM11C bootloader build")

  set(STARTUP
    ${TRIDENT_PLATFORM_PATH}/Library/RT58x/Device/GCC/gcc_startup_cm3_mcu.S
    ${TRIDENT_PLATFORM_PATH}/Library/RT58x/Device/system_cm3_mcu.c
  )

  set(PERIPHERAL
    ${TRIDENT_PLATFORM_PATH}/Library/RT58x/Peripheral/flashctl.c
    ${TRIDENT_PLATFORM_PATH}/Library/RT58x/Peripheral/mp_sector.c
    ${TRIDENT_PLATFORM_PATH}/Library/RT58x/Peripheral/sysctrl.c
    ${TRIDENT_PLATFORM_PATH}/Library/RT58x/Peripheral/sysfun.c
  )

  set(FOTA
    ${TRIDENT_PLATFORM_PATH}/Middleware/FOTA/LzmaDec.c
  )

  set(SYSTEM
    ${TRIDENT_PLATFORM_PATH}/stubs/system_stubs.c
  )

  set(UTILITY
    ${TRIDENT_PLATFORM_PATH}/tokens/tr_platform_tokens.c
  )

  set(CRYPTO
    ${TRIDENT_PLATFORM_PATH}/Library/RT58x/Crypto/crypto.c
    ${TRIDENT_PLATFORM_PATH}/Library/RT58x/Crypto/crypto_util.c
    ${TRIDENT_PLATFORM_PATH}/Library/RT58x/Crypto/sha256.c
  )

  set(PLATFORM_SOURCE
    ${STARTUP}
    ${PERIPHERAL}
    ${FOTA}
    ${SYSTEM}
    ${UTILITY}
    ${CRYPTO}
  )

  set(PLATFORM_INCLUDES
    ${TRIDENT_PLATFORM_PATH}/config/bootloader
    ${TRIDENT_PLATFORM_PATH}/Library/CMSIS/Include
    ${TRIDENT_PLATFORM_PATH}/Library/RT58x/Peripheral/Include
    ${TRIDENT_PLATFORM_PATH}/Library/RT58x/PHY/include
    ${TRIDENT_PLATFORM_PATH}/Library/RT58x/Crypto/include
    ${TRIDENT_PLATFORM_PATH}/Library/RT58x/Include
    ${TRIDENT_PLATFORM_PATH}/Middleware/FOTA/Include
    ${TRIDENT_PLATFORM_PATH}/tokens
  )

  add_library(bootloader_platform 
    OBJECT
    ${PLATFORM_SOURCE}
  )

  target_include_directories(bootloader_platform
    PUBLIC
    ${PLATFORM_INCLUDES}
  )

  target_compile_definitions(bootloader_platform
    PUBLIC
    ${GLOBAL_COMPILE_DEFINITIONS}
  )

else()
    message(STATUS "UNKNOWN platform, skipping bootloader build")
endif()

################################################################################
# Function to build bootloader for different board targets
################################################################################

function(add_hardware_target TARGET_NAME HAL_CONFIG_DIR)
    set(ELF_NAME ${TARGET_NAME}.elf)

    set(FRAMEWORK
        ${TRIDENT_FRAMEWORK_PATH}/utility/printf/tr_printf.c
        ${TRIDENT_FRAMEWORK_PATH}/hal/${PLATFORM}/tr_hal_common.c
        ${TRIDENT_FRAMEWORK_PATH}/hal/${PLATFORM}/tr_hal_gpio.c
        ${TRIDENT_FRAMEWORK_PATH}/hal/${PLATFORM}/tr_hal_uart.c
    )

    # Collect all .c files in the passed in hal_config_* directory
    file(GLOB HAL_CONFIG ${HAL_CONFIG_DIR}/*.c)

    # Collect all .c files in the project source directory
    file(GLOB_RECURSE PROJECT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)

    add_executable(${ELF_NAME}
                ${FRAMEWORK}
                ${HAL_CONFIG}
                ${PROJECT_SRC}
    )

    # Find all directories within src so include paths can be included
    file(GLOB_RECURSE ALL_DIRS LIST_DIRECTORIES true ${CMAKE_CURRENT_SOURCE_DIR}/src/*)
    set(SRC_INCLUDE_DIRS "")
    foreach(DIR ${ALL_DIRS})
        if(IS_DIRECTORY ${DIR})
            list(APPEND SRC_INCLUDE_DIRS ${DIR})
        endif()
    endforeach()
    list(REMOVE_DUPLICATES SRC_INCLUDE_DIRS)

    target_include_directories(${ELF_NAME}
        PRIVATE
        ${TRIDENT_FRAMEWORK_PATH}/common/tokens
        ${TRIDENT_FRAMEWORK_PATH}/utility/printf
        ${TRIDENT_FRAMEWORK_PATH}/hal/${PLATFORM}
        ${TRIDENT_FRAMEWORK_PATH}/hal/include
        ${HAL_CONFIG_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${SRC_INCLUDE_DIRS}
    )

    target_compile_definitions(${ELF_NAME}
        PUBLIC
        ${GLOBAL_COMPILE_DEFINITIONS}
    )

    target_link_libraries(${ELF_NAME} 
        PRIVATE 
        bootloader_platform
    )

    add_custom_command(
        TARGET ${ELF_NAME}
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_CURRENT_BINARY_DIR}/${ELF_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -O srec ${CMAKE_CURRENT_BINARY_DIR}/${ELF_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.s37
        COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_CURRENT_BINARY_DIR}/${ELF_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.bin
    )
endfunction()

################################################################################
# Build app for all configured board targets
################################################################################
# [USER HELP]
# This will build your app for every compatible hal_config_* directory that is
# defined in the root of your project folder. If you wish to remove a build for
# a specific board target, just delete the hal_config_<board>_<platform> directory
# for the <board> you wish to remove.

# Find all hal_config directories in the project root
file(GLOB HAL_CONFIG_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/hal_config_*")

foreach(HAL_CONFIG_DIR ${HAL_CONFIG_DIRS})
    # Get the directory name without the full path
    get_filename_component(DIR_NAME ${HAL_CONFIG_DIR} NAME)
  
    # Extract the board name from the directory name
    string(REGEX REPLACE "hal_config_([^_]+)_.*" "\\1" BOARD_NAME ${DIR_NAME})
  
    # Extract the platform name from the directory name
    string(REGEX REPLACE "hal_config_[^_]+_(.*)" "\\1" PLATFORM_NAME ${DIR_NAME})
  
    # Build board targets for desired platform
    if("${PLATFORM}" STREQUAL "${PLATFORM_NAME}")
        set(TARGET_NAME "${APP_NAME}-${BOARD_NAME}")
        message(STATUS "Adding build target: ${TARGET_NAME} from directory: ${DIR_NAME}")
        add_hardware_target(${TARGET_NAME} ${HAL_CONFIG_DIR})
    endif()
endforeach()