/* ZBOSS Zigbee software protocol stack
 *
 * Copyright (c) 2012-2020 DSR Corporation, Denver CO, USA.
 * http://www.dsr-zboss.com
 * http://www.dsr-corporation.com
 * All rights reserved.
 *
 * This is unpublished proprietary source code of DSR Corporation
 * The copyright notice does not evidence any actual or intended
 * publication of such source code.
 *
 * ZBOSS is a registered trademark of Data Storage Research LLC d/b/a DSR
 * Corporation
 *
 * Commercial Usage
 * Licensees holding valid DSR Commercial licenses may use
 * this file in accordance with the DSR Commercial License
 * Agreement provided with the Software or, alternatively, in accordance
 * with the terms contained in a written agreement between you and
 * DSR.
 */
/* PURPOSE: Stack Configuration file - DO NOT EDIT!!!
 */
#ifndef ZB_VENDOR_H
#define ZB_VENDOR_H 1

/********************************************************************/
/**                       Device Type Specific                     **/
/********************************************************************/

// TODO: look into: #define USE_HW_LONG_ADDR for all projects
// looks like its used to get EUI64 from the chip...

#ifdef NCP_FW_VERSION
#undef NCP_FW_VERSION
#endif
#define NCP_FW_VERSION 0x11223344

#ifdef NCP_STACK_VERSION
#undef NCP_STACK_VERSION
#endif
#define NCP_STACK_VERSION 0x00010203


#define NCP_MODE
// #define SNCP_MODE

#define ZB_HAVE_SERIAL
#define ZB_NCP_TRANSPORT_TYPE_SERIAL
#define ZB_UART_BAUD_RATE 115200
#define ZB_NCP_USE_OSIF_RX_BUFFER

#define ZB_ROUTER_ROLE

#define ZB_USE_SLEEP

// TODO: enable at some point
// #define ZB_NCP_ENABLE_OTA_CMD
// #define ZB_NCP_ENABLE_CUSTOM_COMMANDS
// #define ZB_NCP_ENABLE_ZDO_RAW_CMD

#define APS_FRAGMENTATION
#define ZB_SECURITY_INSTALLCODES

#define ZB_BDB_MODE

#define ZB_APSDE_REQ_ROUTING_FEATURES

#define ZB_APS_SRC_BINDING_TABLE_SIZE 100U
#define ZB_APS_DST_BINDING_TABLE_SIZE 100U

#define ZB_ED_RX_OFF_WHEN_IDLE

#define ZB_NWK_RETRY_COUNT

// TODO
// #define ZB_ENABLE_ZCL

#define ZB_NO_SE_COMMISSIONING
// #define ZB_ENABLE_SE_MIN_CONFIG
// #define ZB_SE_ENABLE_KEC_CLUSTER
// #define ZB_SE_DISABLE_TIME_SYNC
// #define ZB_SECURITY_INSTALLCODES_ONLY

#define ZB_LITE_NO_OLD_CB

#define ZB_NO_NVRAM_VER_MIGRATION

#define ZB_MAC_CONFIGURABLE_TX_POWER
#define ZB_MAC_DEFAULT_TX_POWER_24_GHZ +10

// #define ZB_MINIMAL_CONTEXT
#define ZB_CONFIGURABLE_MEM

#define ZB_MAC_INTERFACE_SINGLE
#define ZB_MAC_MONOLITHIC

#endif /* ZB_VENDOR_H */
