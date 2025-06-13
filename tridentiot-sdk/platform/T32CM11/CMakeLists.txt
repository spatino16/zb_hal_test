set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(DEFINED FLASH_SIZE)
    if(${FLASH_SIZE} STREQUAL "C")
        message(STATUS "T32CM11C Platform Build")
        set(LIB_NAME "T32CM11C")

        set(STARTUP
            ${CMAKE_CURRENT_SOURCE_DIR}/Library/RT58x/Device/GCC/gcc_startup_cm3_mcu.S
            ${CMAKE_CURRENT_SOURCE_DIR}/Library/RT58x/Device/system_cm3_mcu.c
        )

        set(PERIPHERAL
            ${CMAKE_CURRENT_SOURCE_DIR}/Library/RT58x/Peripheral/flashctl.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Library/RT58x/Peripheral/flashds.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Library/RT58x/Peripheral/mp_sector.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Library/RT58x/Peripheral/sysctrl.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Library/RT58x/Peripheral/sysfun.c
        )

        set(FOTA
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/FOTA/LzmaDec.c
        )

        set(MEMMGMT
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/MemoryMgmt/mem_mgmt.c
        )

        set(SYSTEM
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Portable/System/sys_arch.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Portable/System/sys_memory.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Portable/System/sys_taskdbg.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Portable/System/sys_timer.c
            ${CMAKE_CURRENT_SOURCE_DIR}/stubs/system_stubs.c
        )

        set(UTILITY
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Portable/Utility/fsm.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Portable/Utility/shell.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Portable/Utility/util_bda.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Portable/Utility/util_bstream.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Portable/Utility/util_fcs.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Portable/Utility/util_list.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Portable/Utility/util_log.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Portable/Utility/util_printf.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Portable/Utility/util_queue.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Portable/Utility/util_string.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Portable/Utility/util.c
            ${CMAKE_CURRENT_SOURCE_DIR}/tokens/tr_platform_tokens.c
        )

        set(FREERTOS
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Third_Party/FreeRTOS/Source/list.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3/port.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Third_Party/FreeRTOS/Source/portable/MemMang/heap_4.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Third_Party/FreeRTOS/Source/queue.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Third_Party/FreeRTOS/Source/tasks.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Third_Party/FreeRTOS/Source/timers.c
        )

        set(CRYPTO
            ${CMAKE_CURRENT_SOURCE_DIR}/Library/RT58x/Crypto/aes.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Library/RT58x/Crypto/crypto.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Library/RT58x/Crypto/crypto_util.c
            ${CMAKE_CURRENT_SOURCE_DIR}/Library/RT58x/Crypto/sha256.c
        )

        set(PLATFORM_SOURCE
            ${STARTUP}
            ${PERIPHERAL}
            ${FOTA}
            ${MEMMGMT}
            ${SYSTEM}
            ${UTILITY}
            ${FREERTOS}
            ${CRYPTO}
        )

        add_library(${LIB_NAME}
                    OBJECT 
                    ${PLATFORM_SOURCE})

        target_include_directories(${LIB_NAME}
                                PUBLIC
                                ${CMAKE_CURRENT_SOURCE_DIR}/config/common
                                ${CMAKE_CURRENT_SOURCE_DIR}/zboss/include
                                ${CMAKE_CURRENT_SOURCE_DIR}/zboss/osif
                                ${CMAKE_CURRENT_SOURCE_DIR}/tokens
                                ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Third_Party/FreeRTOS/Source/include
                                ${CMAKE_CURRENT_SOURCE_DIR}/Library/CMSIS/Include
                                ${CMAKE_CURRENT_SOURCE_DIR}/Library/RT58x/Peripheral/Include
                                ${CMAKE_CURRENT_SOURCE_DIR}/Library/RT58x/PHY/include
                                ${CMAKE_CURRENT_SOURCE_DIR}/Library/RT58x/PHY/rt569mp/include
                                ${CMAKE_CURRENT_SOURCE_DIR}/Library/RT58x/Crypto/include
                                ${CMAKE_CURRENT_SOURCE_DIR}/Library/RT58x/Include
                                ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/FOTA/Include
                                ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Portable/Utility/include
                                ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Portable/System/Include
                                ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/MemoryMgmt/Include
                                ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/RFB/include
                                ${CMAKE_CURRENT_SOURCE_DIR}/Middleware/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3
                                )
    else()
        message(ERROR "Unsupported Platform: T32CM11${FLASH_SIZE}")
    endif()
else()
    message(ERROR "T32CM11 FLASH_SIZE not defined!")
endif()