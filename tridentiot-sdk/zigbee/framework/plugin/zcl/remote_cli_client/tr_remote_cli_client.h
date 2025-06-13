/// ****************************************************************************
/// @file tr_remote_cli_client.h
///
/// @brief ZCL REMOTE CLI cluster client implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_REMOTE_CLI_CLIENT_H
#define TR_REMOTE_CLI_CLIENT_H

#include "tr_af.h"
#include "tr_remote_cli_common.h"

/// ****************************************************************************
///                                 debug prints
/// ****************************************************************************
#if defined(TR_REMOTE_CLI_CLIENT_PLUGIN_PRINT_ENABLE) && (TR_REMOTE_CLI_CLIENT_PLUGIN_PRINT_ENABLE == 1)
#define tr_remote_cli_client_printf(...)  tr_zcl_printf(__VA_ARGS__)
#define tr_remote_cli_client_println(...) tr_zcl_println(__VA_ARGS__)
#else
#define tr_remote_cli_client_printf(...)
#define tr_remote_cli_client_println(...)
#endif

/// ****************************************************************************
/// @defgroup zcl_remote_cli_client_cb Remote CLI Client Callbacks
/// @ingroup zcl_app_callbacks
/// @{
/// ****************************************************************************

/// @brief Callback fires when remote cli client cluster is initialized
void tr_remote_cli_client_init_cb(void);

/// @} // end of zcl_app_callbacks

/// ****************************************************************************
/// @defgroup zcl_remote_cli_client_apis Remote CLI Client APIs
/// @ingroup zcl_api_references
/// @{
/// ****************************************************************************

/// @brief API to send a CLI command to a remote device
/// @param command pointer to a length prefixed ZCL character string
/// @param short_addr node id to send the remote CLI command to
/// @param endpoint endpoint on remote device
void tr_remote_cli_client_send_cli_command(zb_char_t   *command,
                                           zb_uint16_t short_addr,
                                           zb_uint8_t  endpoint);

/// @brief API for enabling remote CLI interactive mode on a remote device
/// @param cli_enable enable bitfield
/// @param poll_rate temporary poll rate in quarter seconds
/// @param short_addr node id of remote device
/// @param endpoint endpoint of remote device
void tr_remote_cli_client_send_cli_enable(tr_remote_cli_cli_status_arg_t cli_enable,
                                          zb_uint16_t                    poll_rate,
                                          zb_uint16_t                    short_addr,
                                          zb_uint8_t                     endpoint);

/// @} // end of zcl_api_references

/// ****************************************************************************
/// function prototypes consumed by framework
/// ****************************************************************************
void tr_remote_cli_client_init(void);

#endif // TR_REMOTE_CLI_CLIENT_H
