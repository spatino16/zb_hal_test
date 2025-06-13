/// ****************************************************************************
/// @file tr_alarms_server_cb_stubs.c
///
/// @brief weakly defined application callback stubs
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_alarms_server.h"

ZB_WEAK void tr_alarms_server_init_cb(void)
{
}

ZB_WEAK zb_bool_t tr_alarms_server_reset_alarm_cb(zb_zcl_parsed_hdr_t *cmd_info)
{
    ZVUNUSED(cmd_info);
    // can be declared in user application to init app specific functionality
    // return ZB_FALSE to allow framework to continue processing the command
    return ZB_FALSE;
}

ZB_WEAK zb_bool_t tr_alarms_server_reset_all_alarms_cb(zb_zcl_parsed_hdr_t *cmd_info)
{
    ZVUNUSED(cmd_info);
    // can be declared in user application to init app specific functionality
    // return ZB_FALSE to allow framework to continue processing the command
    return ZB_FALSE;
}

ZB_WEAK void tr_alarms_server_write_attr_cb(zb_uint8_t  endpoint,
                                            zb_uint16_t attr_id,
                                            zb_uint8_t  *new_value,
                                            zb_uint16_t manuf_code)
{
    ZVUNUSED(endpoint);
    ZVUNUSED(attr_id);
    ZVUNUSED(new_value);
    ZVUNUSED(manuf_code);
}
