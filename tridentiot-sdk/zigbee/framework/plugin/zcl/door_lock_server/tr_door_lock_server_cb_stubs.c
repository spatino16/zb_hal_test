/// ****************************************************************************
/// @file tr_door_lock_server_cb_stubs.c
///
/// @brief weakly defined application callback stubs
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_door_lock_server.h"

ZB_WEAK void tr_door_lock_server_init_cb(void)
{
}

ZB_WEAK zb_bool_t tr_door_lock_server_lock_door_cb(zb_zcl_parsed_hdr_t *cmd_info,
                                                   zb_uint8_t          *pin_code)
{
    ZVUNUSED(cmd_info);
    ZVUNUSED(pin_code);
    return ZB_FALSE;
}

ZB_WEAK zb_bool_t tr_door_lock_server_unlock_door_cb(zb_zcl_parsed_hdr_t *cmd_info,
                                                     zb_uint8_t          *pin_code)
{
    ZVUNUSED(cmd_info);
    ZVUNUSED(pin_code);
    return ZB_FALSE;
}

ZB_WEAK zb_bool_t tr_door_lock_server_set_pin_cb(zb_zcl_parsed_hdr_t *cmd_info,
                                                 zb_uint16_t         user_id,
                                                 zb_uint8_t          user_status,
                                                 zb_uint8_t          user_type,
                                                 zb_uint8_t          *pin_code)
{
    ZVUNUSED(cmd_info);
    ZVUNUSED(user_id);
    ZVUNUSED(user_status);
    ZVUNUSED(user_type);
    ZVUNUSED(pin_code);
    return ZB_FALSE;
}

ZB_WEAK zb_bool_t tr_door_lock_server_get_pin_cb(zb_zcl_parsed_hdr_t *cmd_info,
                                                 zb_uint16_t         user_id)
{
    ZVUNUSED(cmd_info);
    ZVUNUSED(user_id);
    return ZB_FALSE;
}

ZB_WEAK zb_bool_t tr_door_lock_server_clear_pin_cb(zb_zcl_parsed_hdr_t *cmd_info,
                                                   zb_uint16_t         user_id)
{
    ZVUNUSED(cmd_info);
    ZVUNUSED(user_id);
    return ZB_FALSE;
}

ZB_WEAK zb_bool_t tr_door_lock_server_clear_all_pins_cb(zb_zcl_parsed_hdr_t *cmd_info)
{
    ZVUNUSED(cmd_info);
    return ZB_FALSE;
}

ZB_WEAK zb_bool_t tr_door_lock_server_set_user_status_cb(zb_zcl_parsed_hdr_t *cmd_info,
                                                         zb_uint16_t         user_id,
                                                         zb_uint8_t          user_status)
{
    ZVUNUSED(cmd_info);
    ZVUNUSED(user_id);
    ZVUNUSED(user_status);
    return ZB_FALSE;
}

ZB_WEAK zb_bool_t tr_door_lock_server_get_user_status_cb(zb_zcl_parsed_hdr_t *cmd_info,
                                                         zb_uint16_t         user_id)
{
    ZVUNUSED(cmd_info);
    ZVUNUSED(user_id);
    return ZB_FALSE;
}

ZB_WEAK zb_bool_t tr_door_lock_server_set_user_type_cb(zb_zcl_parsed_hdr_t *cmd_info,
                                                       zb_uint16_t         user_id,
                                                       zb_uint8_t          user_type)
{
    ZVUNUSED(cmd_info);
    ZVUNUSED(user_id);
    ZVUNUSED(user_type);
    return ZB_FALSE;
}

ZB_WEAK zb_bool_t tr_door_lock_server_get_user_type_cb(zb_zcl_parsed_hdr_t *cmd_info,
                                                       zb_uint16_t         user_id)
{
    ZVUNUSED(cmd_info);
    ZVUNUSED(user_id);
    return ZB_FALSE;
}

ZB_WEAK void tr_door_lock_server_write_attr_cb(zb_uint8_t  endpoint,
                                               zb_uint16_t attr_id,
                                               zb_uint8_t  *new_value,
                                               zb_uint16_t manuf_code)
{
    ZVUNUSED(endpoint);
    ZVUNUSED(attr_id);
    ZVUNUSED(new_value);
    ZVUNUSED(manuf_code);
}
