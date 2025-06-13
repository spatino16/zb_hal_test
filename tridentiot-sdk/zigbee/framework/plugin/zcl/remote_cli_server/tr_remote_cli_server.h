/// ****************************************************************************
/// @file tr_remote_cli_server.h
///
/// @brief ZCL REMOTE CLI cluster server implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_REMOTE_CLI_SERVER_H
#define TR_REMOTE_CLI_SERVER_H

#include "tr_af.h"
#include "tr_remote_cli_common.h"

#ifndef TR_REMOTE_CLI_SEND_BUFFER_SIZE
#define TR_REMOTE_CLI_SEND_BUFFER_SIZE 4096
#endif

#ifndef TR_REMOTE_CLI_SEND_PACKET_PAYLOAD_SIZE
#define TR_REMOTE_CLI_SEND_PACKET_PAYLOAD_SIZE 47
#endif

#define TR_REMOTE_CLI_SEND_PACKET_DELAY_MS 100

#define COLOR_START_CHAR                   '\033'
#define COLOR_END_CHAR                     'm'

/// ****************************************************************************
///                                 debug prints
/// ****************************************************************************
#if defined(TR_REMOTE_CLI_SERVER_PLUGIN_PRINT_ENABLE) && (TR_REMOTE_CLI_SERVER_PLUGIN_PRINT_ENABLE == 1)
#define tr_remote_cli_server_printf(...)  tr_zcl_printf(__VA_ARGS__)
#define tr_remote_cli_server_println(...) tr_zcl_println(__VA_ARGS__)
#else
#define tr_remote_cli_server_printf(...)
#define tr_remote_cli_server_println(...)
#endif

/// ****************************************************************************
/// @defgroup zcl_remote_cli_server_cb Remote CLI Server Callbacks
/// @ingroup zcl_app_callbacks
/// @{
/// ****************************************************************************

/// @brief Callback fires when remote cli server cluster is initialized
void tr_remote_cli_server_init_cb(void);

/// @brief Callback fires when a remote cli server attribute is about to be written
/// @param endpoint   device endpoint
/// @param attr_id    ZCL attribute id
/// @param new_value  pointer to the new attribute value
/// @param manuf_code manufacturer specific code
void tr_remote_cli_server_write_attr_cb(zb_uint8_t  endpoint,
                                        zb_uint16_t attr_id,
                                        zb_uint8_t  *new_value,
                                        zb_uint16_t manuf_code);

/// @} // end of zcl_app_callbacks

/// ****************************************************************************
/// function prototypes consumed by framework
/// ****************************************************************************
void tr_remote_cli_server_init(void);

#endif // TR_REMOTE_CLI_SERVER_H
