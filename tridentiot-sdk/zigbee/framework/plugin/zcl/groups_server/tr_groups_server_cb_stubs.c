/// ****************************************************************************
/// @file tr_groups_server_cb_stubs.c
///
/// @brief weakly defined application callback stubs
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_groups_server.h"

ZB_WEAK void tr_groups_server_init_cb(void)
{
}

ZB_WEAK zb_bool_t tr_groups_server_add_group_cb(zb_uint8_t  endpoint,
                                                zb_uint16_t group_id,
                                                zb_char_t   *group_name)
{
    ZVUNUSED(endpoint);
    ZVUNUSED(group_id);
    ZVUNUSED(group_name);
    return ZB_FALSE;
}

ZB_WEAK zb_bool_t tr_groups_server_add_group_if_identifying_cb(zb_bool_t   is_identifying,
                                                               zb_uint8_t  endpoint,
                                                               zb_uint16_t group_id,
                                                               zb_char_t   *group_name)
{
    ZVUNUSED(is_identifying);
    ZVUNUSED(endpoint);
    ZVUNUSED(group_id);
    ZVUNUSED(group_name);
    return ZB_FALSE;
}

ZB_WEAK zb_bool_t tr_groups_server_view_group_cb(zb_uint8_t  endpoint,
                                                 zb_uint16_t group_id,
                                                 zb_char_t   *group_name)
{
    ZVUNUSED(endpoint);
    ZVUNUSED(group_id);
    ZVUNUSED(group_name);
    return ZB_FALSE;
}

ZB_WEAK zb_bool_t tr_groups_server_get_group_membership_cb(zb_uint8_t  endpoint,
                                                           zb_uint8_t  group_count,
                                                           zb_uint16_t *group_id_list)
{
    ZVUNUSED(endpoint);
    ZVUNUSED(group_count);
    ZVUNUSED(group_id_list);
    return ZB_FALSE;
}

ZB_WEAK zb_bool_t tr_groups_server_remove_group_cb(zb_uint8_t  endpoint,
                                                   zb_uint16_t group_id)
{
    ZVUNUSED(endpoint);
    ZVUNUSED(group_id);
    return ZB_FALSE;
}

ZB_WEAK zb_bool_t tr_groups_server_remove_all_groups_cb(zb_uint8_t endpoint)
{
    ZVUNUSED(endpoint);
    return ZB_FALSE;
}
