/// ****************************************************************************
/// @file tr_groups_client.h
///
/// @brief ZCL GROUPS cluster client implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_GROUPS_CLIENT_H
#define TR_GROUPS_CLIENT_H

#include "tr_af.h"
#include "zb_zcl_groups.h"

typedef struct tr_zcl_groups_view_group_res_s
{
    zb_uint8_t  status;
    zb_uint16_t group_id;
    zb_char_t   group_name[17];
}tr_zcl_groups_view_group_res_t;

/// ****************************************************************************
///                                 debug prints
/// ****************************************************************************
#if defined(TR_GROUPS_CLIENT_PLUGIN_PRINT_ENABLE) && (TR_GROUPS_CLIENT_PLUGIN_PRINT_ENABLE == 1)
#define tr_groups_client_printf(...)  tr_zcl_printf(__VA_ARGS__)
#define tr_groups_client_println(...) tr_zcl_println(__VA_ARGS__)
#else
#define tr_groups_client_printf(...)
#define tr_groups_client_println(...)
#endif

/// ****************************************************************************
/// @defgroup zcl_groups_client_cb Groups Client Callbacks
/// @ingroup zcl_app_callbacks
/// @{
/// ****************************************************************************

/// @brief Callback fires when groups client cluster is initialized
void tr_groups_client_init_cb(void);

/// @brief Callback that fires on receipt of groups client cluster add group response command
/// @param endpoint endpoint that received the command
/// @param group_id group id that was added
/// @param status   resulting status
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_groups_client_add_group_resp_cb(zb_uint8_t  endpoint,
                                             zb_uint16_t group_id,
                                             zb_uint8_t  status);

/// @brief Callback that fires on receipt of groups client cluster view group response command
/// @param endpoint   endpoint that received the command
/// @param group_id   group id that was viewed
/// @param group_name group name that was viewed
/// @param status     resulting status
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_groups_client_view_group_resp_cb(zb_uint8_t  endpoint,
                                              zb_uint16_t group_id,
                                              zb_char_t   *group_name,
                                              zb_uint8_t  status);

/// @brief Callback that fires on receipt of groups client cluster get group membership response command
/// @param endpoint      endpoint that received the command
/// @param capacity      number of group entries remaining in the groups table
/// @param group_count   number of groups that exist
/// @param group_id_list list of group ids retrieved by the command
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_groups_client_get_group_membership_resp_cb(zb_uint8_t  endpoint,
                                                        zb_uint8_t  capacity,
                                                        zb_uint8_t  group_count,
                                                        zb_uint16_t *group_id_list);

/// @brief Callback that fires on receipt of groups client cluster remove group response command
/// @param endpoint endpoint that received the command
/// @param group_id group id that was removed
/// @param status   resulting status
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_groups_client_remove_group_resp_cb(zb_uint8_t  endpoint,
                                                zb_uint16_t group_id,
                                                zb_uint8_t  status);

/// @} // end of zcl_app_callbacks

/// ****************************************************************************
/// function prototypes consumed by framework
/// ****************************************************************************
void tr_groups_client_init(void);

#endif // TR_GROUPS_CLIENT_H
