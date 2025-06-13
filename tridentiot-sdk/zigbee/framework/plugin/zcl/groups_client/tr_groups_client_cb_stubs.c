/// ****************************************************************************
/// @file tr_groups_client_cb_stubs.c
///
/// @brief weakly defined application callback stubs
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_groups_client.h"

ZB_WEAK void tr_groups_client_init_cb(void)
{
}

ZB_WEAK zb_bool_t tr_groups_client_add_group_resp_cb(zb_uint8_t  endpoint,
                                                     zb_uint16_t group_id,
                                                     zb_uint8_t  status)
{
    ZVUNUSED(endpoint);
    ZVUNUSED(group_id);
    ZVUNUSED(status);
    return ZB_FALSE;
}

ZB_WEAK zb_bool_t tr_groups_client_view_group_resp_cb(zb_uint8_t  endpoint,
                                                      zb_uint16_t group_id,
                                                      zb_char_t   *group_name,
                                                      zb_uint8_t  status)
{
    ZVUNUSED(endpoint);
    ZVUNUSED(group_id);
    ZVUNUSED(group_name);
    ZVUNUSED(status);
    return ZB_FALSE;
}

ZB_WEAK zb_bool_t tr_groups_client_get_group_membership_resp_cb(zb_uint8_t  endpoint,
                                                                zb_uint8_t  capacity,
                                                                zb_uint8_t  group_count,
                                                                zb_uint16_t *group_id_list)
{
    ZVUNUSED(endpoint);
    ZVUNUSED(capacity);
    ZVUNUSED(group_count);
    ZVUNUSED(group_id_list);
    return ZB_FALSE;
}

ZB_WEAK zb_bool_t tr_groups_client_remove_group_resp_cb(zb_uint8_t  endpoint,
                                                        zb_uint16_t group_id,
                                                        zb_uint8_t  status)
{
    ZVUNUSED(endpoint);
    ZVUNUSED(group_id);
    ZVUNUSED(status);
    return ZB_FALSE;
}
