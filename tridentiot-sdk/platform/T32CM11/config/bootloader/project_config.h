/**************************************************************************//**
* @file     project_config.h
* @version
* @brief   define project config
*
* @copyright
******************************************************************************/

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
#define SUPPORT_MULTITASKING 0

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
#define CRYPTO_AES_ENABLE 0

/*Support ECC SECP192R1/P192 curve */
#define CRYPTO_SECP192R1_ENABLE 0

/*Support ECC SECP256R1/P256 curve */
#define CRYPTO_SECP256R1_ENABLE 1

/*Support ECC SECT163R2/B163 curve */
#define CRYPTO_SECT163R2_ENABLE        0

#define SUPPORT_FREERTOS_PORT          0
#define SUPPORT_FREERTOS_SLEEP         0
#define SUPPORT_FREERTOS_NEST_CRITICAL 0

// ==========================================================
// <h> RF FW
// ==========================================================
#define RFB_ZIGBEE_ENABLED       FALSE

#define RF_FW_INCLUDE_MULTI_2P4G FALSE

#define RF_FW_INCLUDE_BLE        FALSE

#define RF_FW_INCLUDE_PCI        FALSE

#define SUPPORT_15P4             0

#endif /* ifndef ___PROJECT_CONFIG_H__ */
