/// ****************************************************************************
/// @file tr_poll_control_server.h
///
/// @brief ZCL POLL CONTROL cluster server implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_POLL_CONTROL_SERVER_H
#define TR_POLL_CONTROL_SERVER_H

#include "tr_af.h"
#include "zb_zcl_poll_control.h"

/// ****************************************************************************
///                                 debug prints
/// ****************************************************************************
#if defined(TR_POLL_CONTROL_SERVER_PLUGIN_PRINT_ENABLE) && \
    (TR_POLL_CONTROL_SERVER_PLUGIN_PRINT_ENABLE == 1)
#define tr_poll_control_server_printf(...)  tr_zcl_printf(__VA_ARGS__)
#define tr_poll_control_server_println(...) tr_zcl_println(__VA_ARGS__)
#else
#define tr_poll_control_server_printf(...)
#define tr_poll_control_server_println(...)
#endif

/// ****************************************************************************
/// @defgroup zcl_poll_control_server_cb Poll Control Server Callbacks
/// @ingroup zcl_app_callbacks
/// @{
/// ****************************************************************************

/// @brief Callback fires when poll control server cluster is initialized
void tr_poll_control_server_init_cb(void);

/// @brief Callback fires when poll control server cluster is started
void tr_poll_control_server_started_cb(void);

/// @brief Callback fires when poll control server sends a check in command
/// @return ZB_FALSE to prevent sending the check in command
zb_bool_t tr_poll_control_server_send_check_in_cb(void);

/// @brief Callback that user can declare to handle poll control cluster check in response command
/// @param cmd_info struct that contains zcl header info
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_poll_control_server_check_in_response_cb(zb_zcl_parsed_hdr_t *cmd_info);

/// @brief Callback that user can declare to handle poll control cluster fast poll stop command
/// @param cmd_info struct that contains zcl header info
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_poll_control_server_fast_poll_stop_cb(zb_zcl_parsed_hdr_t *cmd_info);

/// @brief Callback that user can declare to handle poll control cluster set long poll interval command
/// @param cmd_info struct that contains zcl header info
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_poll_control_server_set_long_poll_interval_cb(zb_zcl_parsed_hdr_t *cmd_info);

/// @brief Callback that user can declare to handle poll control cluster set short poll interval command
/// @param cmd_info struct that contains zcl header info
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_poll_control_server_set_short_poll_interval_cb(zb_zcl_parsed_hdr_t *cmd_info);

/// @brief Callback fires when a poll control server attribute is about to be written
/// @param endpoint   device endpoint
/// @param attr_id    ZCL attribute id
/// @param new_value  pointer to the new attribute value
/// @param manuf_code manufacturer specific code
void tr_poll_control_server_write_attr_cb(zb_uint8_t  endpoint,
                                          zb_uint16_t attr_id,
                                          zb_uint8_t  *new_value,
                                          zb_uint16_t manuf_code);

/// @} // end of zcl_app_callbacks

/// ****************************************************************************
/// function prototypes consumed by framework
/// ****************************************************************************
void tr_poll_control_server_init(void);
void tr_poll_control_server_connection_state_cb(tr_conn_state_e conn_state);

#endif // TR_POLL_CONTROL_SERVER_H
