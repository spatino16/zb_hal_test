/// ****************************************************************************
/// @file tr_bind.h
///
/// @brief binding plugin that extends binding capabilities especially for
/// groups related behavior.
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_BIND_H
#define TR_BIND_H
#include "zb_common.h"

void zb_apsme_move_dst_bind_table(zb_uint8_t dst_idx,
                                  zb_uint8_t src_idx,
                                  zb_uint8_t cnt);

/// ****************************************************************************
/// @defgroup services_api_bind Bind API References
/// @ingroup services_api_references
/// @{
/// ****************************************************************************

/// @brief API to check if a groups binding exists
/// @param endpoint device endpoint
/// @param group_id group id to search for
/// @return ZB_TRUE if binding exists, ZB_FALSE if it doesn't
zb_bool_t tr_bind_check_groups_binding_exists(zb_uint8_t  endpoint,
                                              zb_uint16_t group_id);

/// @brief API to add a group binding
/// @param endpoint device endpoint
/// @param group_id group id to add
/// @return ZB_TRUE if it was added, ZB_FALSE if it wasn't
zb_bool_t tr_bind_add_group_binding(zb_uint8_t  endpoint,
                                    zb_uint16_t group_id);

/// @brief API to remove a group binding
/// @param endpoint device endpoint
/// @param group_id group id of binding to remove
/// @return ZB_TRUE if it was removed, ZB_FALSE if it wasn't
zb_bool_t tr_bind_remove_group_binding(zb_uint8_t  endpoint,
                                       zb_uint16_t group_id);

/// @brief API to remove all group bindings
/// @param endpoint device endpoint
void tr_bind_remove_all_groups_bindings(zb_uint8_t endpoint);

/// @} // end of services_api_references

#endif // TR_BIND_H
