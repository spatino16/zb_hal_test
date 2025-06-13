/// ****************************************************************************
/// @file tr_poll_control_server_cb_stubs.c
///
/// @brief weakly defined application callback stubs
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_poll_control_server.h"

ZB_WEAK void tr_poll_control_server_init_cb(void)
{
}

ZB_WEAK void tr_poll_control_server_started_cb(void)
{
}

ZB_WEAK zb_bool_t tr_poll_control_server_send_check_in_cb(void)
{
    // can be declared in user application to provide app specific functionality
    // return ZB_TRUE to allow framework to send the check in command
    return ZB_TRUE;
}

ZB_WEAK zb_bool_t tr_poll_control_server_check_in_response_cb(zb_zcl_parsed_hdr_t *cmd_info)
{
    ZVUNUSED(cmd_info);
    // can be declared in user application to provide app specific functionality
    // return ZB_FALSE to allow framework to continue processing the command
    return ZB_FALSE;
}

ZB_WEAK zb_bool_t tr_poll_control_server_fast_poll_stop_cb(zb_zcl_parsed_hdr_t *cmd_info)
{
    ZVUNUSED(cmd_info);
    // can be declared in user application to provide app specific functionality
    // return ZB_FALSE to allow framework to continue processing the command
    return ZB_FALSE;
}

ZB_WEAK zb_bool_t tr_poll_control_server_set_long_poll_interval_cb(zb_zcl_parsed_hdr_t *cmd_info)
{
    ZVUNUSED(cmd_info);
    // can be declared in user application to provide app specific functionality
    // return ZB_FALSE to allow framework to continue processing the command
    return ZB_FALSE;
}

ZB_WEAK zb_bool_t tr_poll_control_server_set_short_poll_interval_cb(zb_zcl_parsed_hdr_t *cmd_info)

{
    ZVUNUSED(cmd_info);
    // can be declared in user application to provide app specific functionality
    // return ZB_FALSE to allow framework to continue processing the command
    return ZB_FALSE;
}

ZB_WEAK void tr_poll_control_server_write_attr_cb(zb_uint8_t  endpoint,
                                                  zb_uint16_t attr_id,
                                                  zb_uint8_t  *new_value,
                                                  zb_uint16_t manuf_code)
{
    ZVUNUSED(endpoint);
    ZVUNUSED(attr_id);
    ZVUNUSED(new_value);
    ZVUNUSED(manuf_code);
}
