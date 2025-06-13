
cmake_minimum_required(VERSION 3.23.5)

project(zb_hal_test
        VERSION 1.0
        LANGUAGES C ASM)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Common variables
set(APP_NAME "zb_hal_test")
set(DEVICE_TYPE "end_device")
set(CHIP_PN ${PLATFORM}${FLASH_SIZE})

set(TRIDENT_SDK_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/tridentiot-sdk")

set(TRIDENT_PLATFORM_PATH "${TRIDENT_SDK_ROOT}/platform/${PLATFORM}")
set(TRIDENT_FRAMEWORK_PATH "${TRIDENT_SDK_ROOT}/framework")
set(ZIGBEE_FRAMEWORK_PATH "${TRIDENT_SDK_ROOT}/zigbee/framework")
set(ZIGBEE_LIBRARY_PATH "${TRIDENT_SDK_ROOT}/zigbee/library")
set(ZBOSS_STABLE_PATH "${TRIDENT_SDK_ROOT}/zigbee/thirdparty/zboss_r23")
set(TR_CMAKE_TOOLS_DIR "${TRIDENT_SDK_ROOT}/tools/cmake")

if(${PLATFORM} STREQUAL "T32CM11")
    set(BTL_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/bootloader/bootloader_T32Cx1x")
    set(BTL_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/bootloader")
    set(BTL_TOOLCHAIN_FILE_PATH "${BTL_SOURCE_DIR}/arm-gcc-toolchain-BTL-T32CM11.cmake")
    set(BTL_LINKER_SCRIPT_PATH "${BTL_SOURCE_DIR}/bootloader_gcc_cm3_mcu.ld")
endif()

# add package dependencies
find_package(Python3 REQUIRED)
if(NOT DEFINED Python3_FOUND)
    message(FATAL_ERROR "Python not found")
endif()

# include cmake functions
include(${TRIDENT_FRAMEWORK_PATH}/common/tokens/generator/tr_generate_token_json.cmake)
include(${TR_CMAKE_TOOLS_DIR}/tr_combine_files.cmake)

# Set global compile definitions that will propagate to all targets
set(GLOBAL_COMPILE_DEFINITIONS
    TR_PLATFORM_${CHIP_PN}
    # [USER HELP] Add additional compile definitions here
)

# Add test build definition if defined
if(DEFINED TR_TEST_BUILD)
    list(APPEND GLOBAL_COMPILE_DEFINITIONS TR_TEST_BUILD="${TR_TEST_BUILD}")
endif()

# Determine stack library based on device type
if("${DEVICE_TYPE}" STREQUAL "end_device")
    set(STACK_LIB "Zigbee_Stack_Lib/lib${CHIP_PN}_Stack_ZED.a")
elseif("${DEVICE_TYPE}" STREQUAL "router")
    set(STACK_LIB "Zigbee_Stack_Lib/lib${CHIP_PN}_Stack_ZR.a")
elseif("${DEVICE_TYPE}" STREQUAL "coordinator")
    set(STACK_LIB "Zigbee_Stack_Lib/lib${CHIP_PN}_Stack_ZC.a")
else()
    set(STACK_LIB "")
    message(STATUS "DEVICE_TYPE ${DEVICE_TYPE} is not recognized!")
endif()

add_library(zigbee_stack_lib STATIC IMPORTED)
set_target_properties(zigbee_stack_lib PROPERTIES
    IMPORTED_LOCATION "${ZIGBEE_LIBRARY_PATH}/${STACK_LIB}"
)

################################################################################
# Bootloader - Built according to bootloader CMakeLists.txt
################################################################################
if(DEFINED BTL_SOURCE_DIR AND DEFINED BTL_BINARY_DIR)
    if(EXISTS "${BTL_SOURCE_DIR}")
        message(STATUS "Bootloader project found at: ${BTL_SOURCE_DIR}")

        if(DEFINED BTL_TOOLCHAIN_FILE_PATH AND DEFINED BTL_LINKER_SCRIPT_PATH)
            include(ExternalProject)

            ExternalProject_Add(
                bootloader
                SOURCE_DIR ${BTL_SOURCE_DIR}
                BINARY_DIR ${BTL_BINARY_DIR}
                CMAKE_ARGS 
                    -DCMAKE_TOOLCHAIN_FILE=${BTL_TOOLCHAIN_FILE_PATH}
                    -DLINKER_SCRIPT_PATH=${BTL_LINKER_SCRIPT_PATH}
                    -DPLATFORM=${PLATFORM}
                    -DFLASH_SIZE=${FLASH_SIZE}
                    -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
                    -DTRIDENT_SDK_ROOT=${TRIDENT_SDK_ROOT}
                # skip installation step 
                INSTALL_COMMAND ""
            )
        else()
            message(WARNING "BTL_TOOLCHAIN_FILE_PATH and/or BTL_LINKER_SCRIPT_PATH missing")
        endif()

    else()
        message(WARNING "Bootloader project not found at: ${BTL_SOURCE_DIR}")
    endif()
else()
    message(STATUS "Skipping bootloader build")
endif()

################################################################################
# Platform Library - Built only once
################################################################################
add_subdirectory(${TRIDENT_PLATFORM_PATH} ${CMAKE_CURRENT_BINARY_DIR}/platform)

target_compile_definitions(${CHIP_PN} PUBLIC ${GLOBAL_COMPILE_DEFINITIONS})

################################################################################
# Framework Library - Built only once
################################################################################
set(FRAMEWORK
    ${TRIDENT_FRAMEWORK_PATH}/common/tokens/tr_mfg_tokens.c
    ${TRIDENT_FRAMEWORK_PATH}/utility/tlv_parser/tr_tlv_parser.c
    ${ZIGBEE_FRAMEWORK_PATH}/common/tr_af.c
    ${ZIGBEE_FRAMEWORK_PATH}/common/tr_af_cb_stubs.c
    ${ZIGBEE_FRAMEWORK_PATH}/utility/nvram/tr_nvram_attr.c
    ${ZIGBEE_FRAMEWORK_PATH}/common/tr_zcl_common.c
    ${ZIGBEE_FRAMEWORK_PATH}/common/tr_zcl_common_cb_stubs.c
    ${ZIGBEE_FRAMEWORK_PATH}/socket/main/tr_main.c
    ${ZBOSS_STABLE_PATH}/zcl/zcl_attr_value.c
)

# [USER HELP] Add additional plugin source path/to/filename.c here
set(PLUGIN_FILES
    ${TRIDENT_FRAMEWORK_PATH}/utility/printf/tr_printf.c
    ${TRIDENT_FRAMEWORK_PATH}/utility/cli/tr_cli.c
    ${TRIDENT_FRAMEWORK_PATH}/utility/cli/tr_cli_buffer.c
    ${TRIDENT_FRAMEWORK_PATH}/utility/cli/tr_cli_command_parser.c
    ${TRIDENT_FRAMEWORK_PATH}/utility/ring_buffer/tr_ring_buffer.c
    ${ZIGBEE_FRAMEWORK_PATH}/utility/circular_buffer/tr_circular_buffer.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/services/cli/tr_cli_argument_parser.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/services/cli/tr_cli_general_cmds.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/services/cli/tr_cli_zcl_global_cmds.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/services/cli/tr_cli_network_cmds.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/services/cli/tr_cli_print_cmds.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/services/cli/tr_cli_zdo_cmds.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/services/cli/tr_cli_reporting_cmds.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/services/cli/tr_cli_binding_table_cmds.c 
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/services/cli/tr_cli_find_and_bind_cmds.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/services/debug_print/tr_debug_print.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/services/debug_print/tr_debug_print_cli.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/services/bind/tr_bind.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/services/network_rejoin/tr_network_rejoin.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/services/network_rejoin/tr_network_rejoin_cb_stubs.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/basic_server/tr_basic_server.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/basic_server/tr_basic_server_cb_stubs.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/identify_server/tr_identify_server.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/identify_server/tr_identify_server_cb_stubs.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/identify_client/tr_identify_client.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/identify_client/tr_identify_client_cb_stubs.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/identify_client/tr_identify_client_cli.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/groups_server/tr_groups_server.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/groups_server/tr_groups_server_cb_stubs.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/groups_server/tr_groups_server_cli.c
    ${ZBOSS_STABLE_PATH}/zcl/zcl_scenes.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/scenes_server/tr_scenes_server.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/scenes_server/tr_scenes_server_cb_stubs.c
    ${ZBOSS_STABLE_PATH}/zcl/zcl_on_off_commands.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/on_off_server/tr_on_off_server.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/on_off_server/tr_on_off_server_cb_stubs.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/ota_upgrade_client/tr_osif_ota.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/ota_upgrade_client/tr_over_the_air_bootloading_client.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/ota_upgrade_client/tr_over_the_air_bootloading_client_cli.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/ota_upgrade_client/tr_ota_upgrade_client_cb_stubs.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/ota_upgrade_client/tr_ota_upgrade_client_minimal.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/ota_upgrade_common/tr_ota_upgrade_common.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/remote_cli_client/tr_remote_cli_client.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/remote_cli_client/tr_remote_cli_client_cb_stubs.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/remote_cli_client/tr_remote_cli_client_cli.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/remote_cli_server/tr_remote_cli_server.c
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/remote_cli_server/tr_remote_cli_server_cb_stubs.c
)

set(FRAMEWORK_SOURCE
    ${FRAMEWORK}
    ${PLUGIN_FILES}
)

# [USER HELP] Add additional plugin include directory paths here
set(FRAMEWORK_INCLUDE
    ${CMAKE_CURRENT_SOURCE_DIR}/generated
    ${TRIDENT_PLATFORM_PATH}/config/${DEVICE_TYPE}
    ${TRIDENT_FRAMEWORK_PATH}/common/tokens
    ${TRIDENT_FRAMEWORK_PATH}/utility/printf
    ${TRIDENT_FRAMEWORK_PATH}/utility/cli
    ${TRIDENT_FRAMEWORK_PATH}/utility/ring_buffer
    ${TRIDENT_FRAMEWORK_PATH}/utility/tlv_parser
    ${ZIGBEE_FRAMEWORK_PATH}/include
    ${ZIGBEE_FRAMEWORK_PATH}/include/stubs
    ${ZIGBEE_FRAMEWORK_PATH}/utility/nvram
    ${ZIGBEE_FRAMEWORK_PATH}/utility/circular_buffer
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/basic_server
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/identify_server
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/identify_client
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/groups_server
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/scenes_server
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/on_off_server
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/ota_upgrade_client
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/ota_upgrade_common
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/remote_cli_client
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/remote_cli_server
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/zcl/remote_cli_common
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/services/cli
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/services/debug_print
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/services/bind
    ${ZIGBEE_FRAMEWORK_PATH}/plugin/services/network_rejoin
    ${ZBOSS_STABLE_PATH}/include
    ${ZBOSS_STABLE_PATH}/include/zcl
)

# Create the shared framework library and link with needed libraries
# Link the framework with the platform libraries (zigbee_framework dependency on ${CHIP_PN})
add_library(zigbee_framework 
    OBJECT
    ${FRAMEWORK_SOURCE}
)

# Get the INTERFACE_INCLUDE_DIRECTORIES from the platform library and add them to the framework
target_include_directories(zigbee_framework
    PUBLIC
    ${FRAMEWORK_INCLUDE}
    $<TARGET_PROPERTY:${CHIP_PN},INTERFACE_INCLUDE_DIRECTORIES>
)

# Apply global definitions to the framework
target_compile_definitions(zigbee_framework
    PUBLIC
    ${GLOBAL_COMPILE_DEFINITIONS}
)

# Link with the platform and stack libraries directly
target_link_libraries(zigbee_framework 
    PRIVATE 
    ${CHIP_PN}
    zigbee_stack_lib
)

################################################################################
# Function to build app for different board targets
################################################################################
function(add_hardware_target TARGET_NAME HAL_CONFIG_DIR BOARD_NAME)
    set(ELF_NAME ${TARGET_NAME}.elf)
    
    set(HAL
        ${TRIDENT_FRAMEWORK_PATH}/hal/${PLATFORM}/tr_hal_common.c
        ${TRIDENT_FRAMEWORK_PATH}/hal/${PLATFORM}/tr_hal_gpio.c
        ${TRIDENT_FRAMEWORK_PATH}/hal/${PLATFORM}/tr_hal_rtc.c
        ${TRIDENT_FRAMEWORK_PATH}/hal/${PLATFORM}/tr_hal_spi.c
        ${TRIDENT_FRAMEWORK_PATH}/hal/${PLATFORM}/tr_hal_wdog.c
        ${TRIDENT_FRAMEWORK_PATH}/hal/${PLATFORM}/tr_hal_timers.c
        ${TRIDENT_FRAMEWORK_PATH}/hal/${PLATFORM}/tr_hal_uart.c
        # [USER HELP] Add additional HAL files here
    )

    if(${PLATFORM} STREQUAL "T32CM11")
        list(APPEND HAL
            ${TRIDENT_PLATFORM_PATH}/Library/RT58x/Peripheral/dma.c
            ${TRIDENT_PLATFORM_PATH}/Library/RT58x/Peripheral/tr_lpm.c
        )
    endif()
    
    # Collect all .c files in the passed in hal_config_* directory
    file(GLOB HAL_CONFIG ${HAL_CONFIG_DIR}/*.c)

    # Collect all .c files in the project generated directory
    file(GLOB GENERATED ${CMAKE_CURRENT_SOURCE_DIR}/generated/*.c)

    # Recursively collect all .c files in the project source directory
    file(GLOB_RECURSE PROJECT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/*.c)
    
    # Create the executable for this hardware target
    add_executable(${ELF_NAME}
        ${HAL}
        ${HAL_CONFIG}
        ${GENERATED}
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
        ${HAL_CONFIG_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/generated
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${SRC_INCLUDE_DIRS}
        ${TRIDENT_FRAMEWORK_PATH}/hal/${PLATFORM}
        ${TRIDENT_FRAMEWORK_PATH}/hal/include 
        # [USER HELP] Add additional application include directory paths here
    )
    
    target_compile_definitions(${ELF_NAME}
        PUBLIC
        ${GLOBAL_COMPILE_DEFINITIONS}
    )

    target_link_libraries(${ELF_NAME} 
        PRIVATE 
        ${CHIP_PN}
        zigbee_framework
        zigbee_stack_lib
    )

    # Generate output files (hex, s37, bin) for this target
    add_custom_command(
        TARGET ${ELF_NAME}
        POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} -O ihex ${CMAKE_CURRENT_BINARY_DIR}/${ELF_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.hex
        COMMAND ${CMAKE_OBJCOPY} -O srec ${CMAKE_CURRENT_BINARY_DIR}/${ELF_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.s37
        COMMAND ${CMAKE_OBJCOPY} -O binary ${CMAKE_CURRENT_BINARY_DIR}/${ELF_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.bin
        COMMAND ${Python3_EXECUTABLE} ${TR_CMAKE_TOOLS_DIR}/gen_ozone_project.py -e ${CMAKE_CURRENT_BINARY_DIR}/${ELF_NAME} -o ${CMAKE_CURRENT_BINARY_DIR} -t ${TRIDENT_PLATFORM_PATH}/templates/${CHIP_PN}_ozone_template.jdebug.jinja
    )

    if(DEFINED BTL_BINARY_DIR)
        # Attempt to create combo hex file, depend on ${ELF_NAME} being built first
        tr_combine_files(
            "${BTL_BINARY_DIR}/bootloader-${BOARD_NAME}.hex"
            "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.hex"
            "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}-combined.hex"
            ${ELF_NAME}
        )
    endif()
endfunction()

################################################################################
# Generate APP tokens JSON file
################################################################################
if(DEFINED PLATFORM AND DEFINED FLASH_SIZE)
    message(STATUS "Chip Target: ${CHIP_PN}")

    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/src/project_app_tokens.h)
        tr_generate_token_json(
            TYPE "APP"
            CHIP "${PLATFORM}"
            FLASH_SIZE "${FLASH_SIZE}"
            TOKEN_PATH "${TRIDENT_FRAMEWORK_PATH}/common/tokens"
            PLATFORM_PATH "${TRIDENT_PLATFORM_PATH}"
            HEADER_FILE "${CMAKE_CURRENT_SOURCE_DIR}/src/project_app_tokens.h"
            OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}"
        )
    else()
        message(STATUS "Skipping Token Generation - APP Tokens Not Defined")
    endif()
endif()

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
        add_hardware_target(${TARGET_NAME} ${HAL_CONFIG_DIR} ${BOARD_NAME})
    endif()
endforeach()