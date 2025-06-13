/// ****************************************************************************
/// @file project_config.h
///
/// @brief stack configuration file - DO NOT EDIT!!
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "chip_define.h"
#include "cm3_mcu.h"

#ifndef ___PROJECT_CONFIG_H__
#define ___PROJECT_CONFIG_H__

/* TODO
 * - peripheral defines should go away after we move to the Trident HAL
 */

#define RT570

#define CHIP_TYPE    RT583

#define CHIP_VERSION RT58X_MPB

#define USE_FREERTOS

#define SET_SYS_CLK SYS_CLK_32MHZ

#define MODULE_ENABLE(module) (module > 0)

/*
 * If system support multi-tasking, some hardware need mutex to protect
 */
#define SUPPORT_MULTITASKING 1

/*System use UART0 */
#define SUPPORT_UART0             1

#define UART0_USER_HANDLE_RECV_RX 1

/*System use UART1 */
#define SUPPORT_UART1        1

#define SUPPORT_UART1_TX_DMA 1

#define SUPPORT_UART1_RX_DMA 1

/*enable this option, UART1 will support CTS/RTS pin for flow control*/
#define SUPPORT_UART1_FLOWCNTL 0

/*System use UART2 */
#define SUPPORT_UART2        1

#define SUPPORT_UART2_TX_DMA 1

#define SUPPORT_UART2_RX_DMA 1


#define SUPPORT_QSPI_DMA 1

/*Support AES  */
#define CRYPTO_AES_ENABLE 1

/*Support ECC SECP192R1/P192 curve */
#define CRYPTO_SECP192R1_ENABLE 1

/*Support ECC SECP256R1/P256 curve */
#define CRYPTO_SECP256R1_ENABLE 1

/*Support ECC SECT163R2/B163 curve */
#define CRYPTO_SECT163R2_ENABLE        1

#define SUPPORT_FREERTOS_PORT          1
#define SUPPORT_FREERTOS_SLEEP         1
#define SUPPORT_FREERTOS_NEST_CRITICAL 0

// ==========================================================
// <h> RF FW
// ==========================================================
#define RFB_ZIGBEE_ENABLED       TRUE

#define RF_FW_INCLUDE_MULTI_2P4G FALSE

#define RF_FW_INCLUDE_BLE        FALSE

#define RF_FW_INCLUDE_PCI        TRUE

#define SUPPORT_15P4             1

// ==========================================================
// <h> moved from zb_config_platform.h
// ==========================================================
#define Timer0 0
#define Timer1 1
#define Timer2 2
#define Timer3 3

 #if (defined(ZB_ED_ROLE) && (CHIP_VERSION == RT58X_MPB))
 #define UsedTimer Timer3  // Timer3 is needed for sleep
 #else
 #define UsedTimer Timer2
 #endif

#endif /* ifndef ___PROJECT_CONFIG_H__ */
