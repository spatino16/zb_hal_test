/// ****************************************************************************
/// @file tr_plugin_config.h
///
/// @brief contains the definitions for configuration of the plugins
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_PLUGIN_CONFIG_H
#define TR_PLUGIN_CONFIG_H

#include <stdio.h>

#ifdef TR_CLI_PROMPT
#undef TR_CLI_PROMPT
#endif
#define TR_CLI_PROMPT "bulb> "

#ifdef TR_CLI_MAX_PROMPT_LEN
#undef TR_CLI_MAX_PROMPT_LEN
#endif
#define TR_CLI_MAX_PROMPT_LEN sizeof(TR_CLI_PROMPT)

// Plugin CLI Configs
#define TR_GROUPS_SERVER_CLI_ENABLE
#define TR_IDENTIFY_CLIENT_CLI_ENABLE
#define TR_OVER_THE_AIR_BOOTLOADING_CLIENT_CLI_ENABLE
#define TR_REMOTE_CLI_CLIENT_CLI_ENABLE

// Debug print user configuration
#ifndef TR_TEST_BUILD
#define TR_DEBUG_PRINT_COLOR_ENABLED
#endif

#define TR_DEBUG_PRINT_STACK_ENABLED
#define TR_DEBUG_PRINT_CORE_ENABLED
#define TR_DEBUG_PRINT_APP_ENABLED
#define TR_DEBUG_PRINT_ZCL_ENABLED
#define TR_DEBUG_PRINT_RX_MSGS_ENABLED

// Debug print color configuration
#ifdef TR_DEBUG_PRINT_COLOR_ENABLED
#define TR_DEBUG_PRINT_STACK_COLOR   DEBUG_BLUE
#define TR_DEBUG_PRINT_CORE_COLOR    DEBUG_DEFAULT
#define TR_DEBUG_PRINT_APP_COLOR     DEBUG_MAGENTA
#define TR_DEBUG_PRINT_ZCL_COLOR     DEBUG_GREEN
#define TR_DEBUG_PRINT_RX_MSGS_COLOR DEBUG_YELLOW
#endif

// Plugin Enable Configs
#define TR_NETWORK_REJOIN_PLUGIN_ENABLE
#define TR_BASIC_SERVER_PLUGIN_ENABLE
#define TR_GROUPS_SERVER_PLUGIN_ENABLE
#define TR_IDENTIFY_CLIENT_PLUGIN_ENABLE
#define TR_IDENTIFY_SERVER_PLUGIN_ENABLE
#define TR_ON_OFF_SERVER_PLUGIN_ENABLE
#define TR_OVER_THE_AIR_BOOTLOADING_CLIENT_PLUGIN_ENABLE
#define TR_REMOTE_CLI_CLIENT_PLUGIN_ENABLE
#define TR_REMOTE_CLI_SERVER_PLUGIN_ENABLE
#define TR_SCENES_SERVER_PLUGIN_ENABLE

// Plugin Print Configs
#define TR_NETWORK_REJOIN_PLUGIN_PRINT_ENABLE                  1
#define TR_BASIC_SERVER_PLUGIN_PRINT_ENABLE                    1
#define TR_GROUPS_SERVER_PLUGIN_PRINT_ENABLE                   1
#define TR_IDENTIFY_CLIENT_PLUGIN_PRINT_ENABLE                 1
#define TR_IDENTIFY_SERVER_PLUGIN_PRINT_ENABLE                 1
#define TR_ON_OFF_SERVER_PLUGIN_PRINT_ENABLE                   1
#define TR_OVER_THE_AIR_BOOTLOADING_CLIENT_PLUGIN_PRINT_ENABLE 1
#define TR_REMOTE_CLI_CLIENT_PLUGIN_PRINT_ENABLE               1
#define TR_REMOTE_CLI_SERVER_PLUGIN_PRINT_ENABLE               1
#define TR_SCENES_SERVER_PLUGIN_PRINT_ENABLE                   1

// Stack Behavior Configs
// #define TR_ZIGBEE_R22
// #define TR_DONT_USE_ECDHE_CURVE_25519_HASH_SHA256
// #define TR_DONT_USE_ECDHE_CURVE_25519_HASH_AESMMO128
#define TR_ENABLE_JOINING_DISTRIBUTED_NETWORKS

// Channel Mask Configs
#define TR_PRIMARY_CHANNEL_MASK   0x02108800
#define TR_SECONDARY_CHANNEL_MASK 0x05EF7000

// End-Device Keepalive Configs
#define TR_ED_AGING_TIMEOUT      ED_AGING_TIMEOUT_64MIN
#define TR_KEEPALIVE_INTERVAL_MS 300000
#define TR_KEEPALIVE_METHOD      BOTH_KEEPALIVE_METHODS

// Network Rejoin Plugin Configs
#define TR_NWK_REJOIN_DELAY_MULTIPLIER  2
#define TR_NWK_REJOIN_INITIAL_DELAY_SEC 1
#define TR_NWK_REJOIN_MAX_DELAY_SEC     900

// Groups Server Plugin Configs
#define TR_GROUPS_SERVER_AUTO_BIND_ENABLE

// Over the Air Bootloading Client Plugin Configs
#define TR_OTA_UPGRADE_QUERY_DELAY_MIN 60

#endif /* TR_PLUGIN_CONFIG_H */
