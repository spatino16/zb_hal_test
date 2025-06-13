/// ****************************************************************************
/// @file tr_groups_server.h
///
/// @brief ZCL GROUPS cluster server implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_GROUPS_SERVER_H
#define TR_GROUPS_SERVER_H

#include "tr_af.h"
#include "zb_zcl_groups.h"
#include "zb_zcl.h"
#include "zb_zdo.h"
#include "zb_aps.h"

/// ****************************************************************************
///                                 debug prints
/// ****************************************************************************
#if defined(TR_GROUPS_SERVER_PLUGIN_PRINT_ENABLE) && (TR_GROUPS_SERVER_PLUGIN_PRINT_ENABLE == 1)
#define tr_groups_server_printf(...)  tr_zcl_printf(__VA_ARGS__)
#define tr_groups_server_println(...) tr_zcl_println(__VA_ARGS__)
#else
#define tr_groups_server_printf(...)
#define tr_groups_server_println(...)
#endif

/// ****************************************************************************
/// @defgroup zcl_groups_server_cb Groups Server Callbacks
/// @ingroup zcl_app_callbacks
/// @{
/// ****************************************************************************

/// @brief Callback fires when groups server cluster is initialized
void tr_groups_server_init_cb(void);

/// @brief Callback that fires on receipt of groups server cluster add group command
/// @param endpoint   endpoint that received the command
/// @param group_id   group id being added
/// @param group_name optional group name being added
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_groups_server_add_group_cb(zb_uint8_t  endpoint,
                                        zb_uint16_t group_id,
                                        zb_char_t   *group_name);

/// @brief Callback that fires on receipt of groups server cluster add group if identifying command
/// @param is_identifying ZB_TRUE if actively identifying, ZB_FALSE otherwise
/// @param endpoint       endpoint that received the command
/// @param group_id       group id being added
/// @param group_name     optional group name being added
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_groups_server_add_group_if_identifying_cb(zb_bool_t   is_identifying,
                                                       zb_uint8_t  endpoint,
                                                       zb_uint16_t group_id,
                                                       zb_char_t   *group_name);

/// @brief Callback that fires on receipt of groups server cluster view group command
/// @param endpoint   endpoint that received the command
/// @param group_id   group id being checked
/// @param group_name group name that should be sent in the response
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_groups_server_view_group_cb(zb_uint8_t  endpoint,
                                         zb_uint16_t group_id,
                                         zb_char_t   *group_name);

/// @brief Callback that fires on receipt of groups server cluster get group membership command
/// @param endpoint      endpoint that received the command
/// @param group_count   number of groups being checked
/// @param group_id_list list of group ids being checked
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_groups_server_get_group_membership_cb(zb_uint8_t  endpoint,
                                                   zb_uint8_t  group_count,
                                                   zb_uint16_t *group_id_list);

/// @brief Callback that fires on receipt of groups server cluster remove group command
/// @param endpoint endpoint that received the command
/// @param group_id group id to be removed
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_groups_server_remove_group_cb(zb_uint8_t  endpoint,
                                           zb_uint16_t group_id);

/// @brief Callback that fires on receipt of groups server cluster remove all groups command
/// @param endpoint endpoint that received the command
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_groups_server_remove_all_groups_cb(zb_uint8_t endpoint);

/// @} // end of zcl_app_callbacks

/// ****************************************************************************
/// function prototypes consumed by framework
/// ****************************************************************************
void tr_groups_server_init(void);

#endif // TR_GROUPS_SERVER_H
