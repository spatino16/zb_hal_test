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
/* PURPOSE:
 */
#ifndef ZB_VENDOR_H
#define ZB_VENDOR_H 1

/********************************************************************/
/**                       Device Type Specific                     **/
/********************************************************************/
// TODO
#define ZB_ED_ROLE
// #define ZB_ROUTER_ROLE
// #define ZB_COORDINATOR_ROLE
// #define ZB_COORDINATOR_ONLY
// #define ZB_USE_SLEEP // TODO: review this when making sleep plugin - Arch 06/04/2024

#define ZB_PROMISCUOUS_MODE
#define ZB_DIRECT_PIB_ACCESS
// #define ZB_MAC_TESTING_MODE
#define NVRAM_NOT_AVAILABLE
#define ZB_NEVER_STOP_TIMER

#if defined ZB_COORDINATOR_ROLE || defined ZB_ROUTER_ROLE
#define ZB_MAC_PENDING_BIT_SOURCE_MATCHING // this implies a router role
#define ZB_MAC_SOFTWARE_PB_MATCHING        // this implies a router role
#endif


// TODO: review rest of configs for sniffer

/********************************************************************/
/**                    Same Across All Device Types                **/
/********************************************************************/
// #define ZB_CONFIGURABLE_MEM
#define APS_FRAGMENTATION

#ifndef ZB_USE_SLEEP
/*
 * Sleep functionality is implemented as a busy loop, so it doesn't actually
 * sleep. However, ZB_USE_SLEEP is defined for ED and this define is needed to
 * not call "sleep" from stack for "rx on when idle" devices.
 */
#define ZB_MAC_RADIO_CANT_WAKEUP_MCU // TODO: review this when making sleep plugin - Arch 06/04/2024
#endif

#ifndef ZB_MAC_TESTING_MODE
#define ZB_ENABLE_HA
#define ZB_ENABLE_ZCL
// #define ZB_ENABLE_SE // TODO
// #define ZB_ENABLE_ZGP
#endif

/* ZB3.0 BDB mode */
#define ZB_BDB_MODE
// #define ZB_BDB_ENABLE_FINDING_BINDING
// #define ZB_DISTRIBUTED_SECURITY_ON
#define ZB_SECURITY_INSTALLCODES

/* Device support */
// #define ZB_ALL_DEVICE_SUPPORT

/* OOM detection */
#define ZB_CHECK_OOM_STATUS
#define ZB_SEND_OOM_STATUS

// # define ZB_NO_NVRAM_VER_MIGRATION

/* Enabled specific feature to send packets via binding in parallel */
// #define ZB_BIND_TRANS_PARALLEL

// #define ZB_USE_OSIF_OTA_ROUTINES // TODO: review when making OTA plugin - Arch 06/04/2024

/********************************************************************/
/**                       Stack Configurations                     **/
/********************************************************************/
#ifndef ZB_ED_ROLE
#define ZB_CONFIG_IOBUF_POOL_SIZE  40
#define ZB_CONFIG_SCHEDULER_Q_SIZE 40
#else
#define ZB_CONFIG_IOBUF_POOL_SIZE  30
#define ZB_CONFIG_SCHEDULER_Q_SIZE 30
#endif

/********************************************************************/
/**                             To Review                          **/
/********************************************************************/
// TODO: review
// # define ZB_APS_USER_PAYLOAD
// #define ZB_PROMISCUOUS_MODE
/* Production config is not implemented on EFR32 */
// #define ZB_PRODUCTION_CONFIG

// ZGP
#ifdef  ZB_ENABLE_ZGP
#define MAC_AUTO_DELAY_IN_MAC_GP_SEND
#define MAC_AUTO_GPDF_RETX
#define ZB_ENABLE_ZGP_GPCB
#define ZB_ZGP_TRANSL_CMD_PLD_MAX_SIZE 3
#define ZGP_CLUSTER_TEST
#define ZB_ZGP_SKIP_GPDF_ON_NWK_LAYER
#define ZB_ZGP_RUNTIME_WORK_MODE_WITH_PROXIES
#define ZB_ZGP_SINK_TBL_SIZE  10
#define ZB_ZGP_PROXY_TBL_SIZE 4
#endif

// SE
#ifdef ZB_ENABLE_SE
#define ZB_ENABLE_SE_CLUSTERS
#endif

#if (defined ZB_ZCL_SUPPORT_CLUSTER_WWAH && defined ZB_ZCL_ENABLE_WWAH_SERVER)
#define ZB_NWK_RETRY_COUNT
#define ZB_BEACON_SURVEY
#define ZB_PARENT_CLASSIFICATION
#endif


// #define ZB_REJOIN_BACKOFF // TODO: review how this impact WWAH cluster - Arch 06/04/2024

#endif /* ZB_VENDOR_H */
