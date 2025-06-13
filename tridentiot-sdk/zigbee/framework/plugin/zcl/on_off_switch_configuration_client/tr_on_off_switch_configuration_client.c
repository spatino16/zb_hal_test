/// ****************************************************************************
/// @file tr_on_off_switch_configuration_client.c
///
/// @brief ZCL ON/OFF SWITCH CONFIG cluster client implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_on_off_switch_configuration_client.h"

// on off switch config client cluster plugin init
void tr_on_off_switch_configuration_client_init(void)
{
    zb_zcl_add_cluster_handlers(TR_ZCL_CLUSTER_ON_OFF_SWITCH_CONFIGURATION_ID,
                                TR_ZCL_CLUSTER_CLIENT_ROLE,
                                (zb_zcl_cluster_check_value_t)NULL,
                                (zb_zcl_cluster_write_attr_hook_t)NULL,
                                (zb_zcl_cluster_handler_t)NULL);

    tr_on_off_switch_configuration_client_init_cb();
}
