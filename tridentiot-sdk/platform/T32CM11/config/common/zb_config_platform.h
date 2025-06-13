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
#ifndef ZB_CONFIG_PLATFORM_H
#define ZB_CONFIG_PLATFORM_H 1

////////////////////////////////////////////////////////////////////////
// platform specific things that need to be included in the stack build
////////////////////////////////////////////////////////////////////////
#define ZB_LITTLE_ENDIAN
#define ZB_NEED_ALIGN
#define ZB_USE_NVRAM

// TODO: review this - should be able to support HW AES - Arch 10/09/2024
// #define ZB_SOFT_SECURITY
/* May use soft security + HW AES */
#ifndef ZB_SOFT_SECURITY
#define ZB_HW_ZB_AES128
#define ZB_HW_ZB_AES128_DEC
#endif

/* our MAC */
#define ZB_MANUAL_ACK       /* Manual ACK parse in MAC LL layer */
#define ZB_AUTO_ACK_TX
#define ZB_MAC_RX_QUEUE_CAP 4
#define ZB_MAC_TIMESTAMP_IN_PKT
#define MAC_AUTO_DELAY_IN_MAC_GP_SEND
/* This LL MAC implementation always set Pending bit to 1 to be able to implement Injected LEAVE requires since r21.
   TODO: disable auto ACK TX, implement manual ACK send, use SW pending bit matching.
 */
// JJ #define ZB_MAC_STICKY_PENDING_BIT

// TODO: these are defined in Silabs implementation, review - Arch 02/20/2024
#define ZB_MAC_AUTO_ACK_RECV
#define ZB_MANUAL_ADDR_FILTER
#define ZB_MAC_BLOCK_RECV_WHEN_IN_BUF_IS_FULL
#define ZB_MAC_POLL_INDICATION_CALLS_REDUCED

#define ZB_GPD_TX_OFFSET_US                                         \
        (ZB_MILLISECONDS_TO_USEC(ZB_GPD_RX_OFFSET_MS) -             \
         2 * ZB_MAC_A_UNIT_BACKOFF_PERIOD * ZB_SYMBOL_DURATION_USEC \
         - 900)


////////////////////////////////////////////////////
// generic stuff that can be platform independent
////////////////////////////////////////////////////
#define ZB_VOLATILE
#define ZB_SDCC_XDATA
#define ZB_CALLBACK
#define ZB_SDCC_BANKED
#define ZB_KEIL_REENTRANT

#ifdef ZB_CONFIG_DEFAULT_KERNEL_DEFINITION
#ifdef ZB_ED_ROLE
#define ZB_CONFIG_ROLE_ZED

#define ZB_CONFIG_OVERALL_NETWORK_SIZE 16
#define ZB_CONFIG_LIGHT_TRAFFIC
#define ZB_CONFIG_APPLICATION_MODERATE
#endif

#ifdef ZB_ROUTER_ROLE
#define ZB_CONFIG_ROLE_ZR

#define ZB_CONFIG_OVERALL_NETWORK_SIZE 64
#define ZB_CONFIG_HIGH_TRAFFIC
#define ZB_CONFIG_APPLICATION_MODERATE
#define ZB_CONFIG_NWK_DISC_TABLE_SIZE ZB_CONFIG_OVERALL_NETWORK_SIZE
#endif

#ifdef ZB_COORDINATOR_ROLE
#define ZB_CONFIG_ROLE_ZC

#define ZB_CONFIG_OVERALL_NETWORK_SIZE 128
#define ZB_CONFIG_HIGH_TRAFFIC
#define ZB_CONFIG_APPLICATION_COMPLEX
#endif

#endif /* ifdef ZB_CONFIG_DEFAULT_KERNEL_DEFINITION */


#if defined ZB_COORDINATOR_ROLE || defined ZB_ROUTER_ROLE
#define ZB_MAC_PENDING_BIT_SOURCE_MATCHING // this implies a router role
#define ZB_MAC_SOFTWARE_PB_MATCHING        // this implies a router role
#endif


////////////////////////////////////////////////////
// stuff that should find a new home
////////////////////////////////////////////////////
// LCD added 9/10/24
// these values are used by ZBOSS to calculate the lqa used in neighbor tables
// for path cost in the r23 stack.
// TODO: Make sure these are accurate
#define ZB_RSSI_MIN -100
#define ZB_RSSI_MAX -15

// Enable TX power configuration and set the default power
#define ZB_MAC_CONFIGURABLE_TX_POWER
#ifdef ZB_MAC_DEFAULT_TX_POWER_24_GHZ
#undef ZB_MAC_DEFAULT_TX_POWER_24_GHZ
#define ZB_MAC_DEFAULT_TX_POWER_24_GHZ 10
#endif

#endif /* ZB_CONFIG_PLATFORM_H */
