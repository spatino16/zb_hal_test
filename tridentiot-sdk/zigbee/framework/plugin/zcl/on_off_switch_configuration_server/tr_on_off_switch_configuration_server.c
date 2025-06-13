/// ****************************************************************************
/// @file tr_on_off_switch_configuration_server.c
///
/// @brief ZCL ON/OFF SWITCH CONFIG cluster server implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_on_off_switch_configuration_server.h"

static zb_ret_t on_off_switch_configuration_server_check_value(zb_uint16_t attr_id,
                                                               zb_uint8_t  endpoint,
                                                               zb_uint8_t  *value)
{
    zb_ret_t ret = RET_OK;
    ZVUNUSED(endpoint);

    switch (attr_id)
    {
        case TR_ZCL_ATTR_ON_OFF_SWITCH_CONFIGURATION_SWITCH_TYPE_ID:
            if (ZB_ZCL_ON_OFF_SWITCH_CONFIGURATION_SWITCH_TYPE_MAX_VALUE < *value)
            {
                ret = RET_ERROR;
            }
            break;

        case TR_ZCL_ATTR_ON_OFF_SWITCH_CONFIGURATION_SWITCH_ACTIONS_ID:
            if (ZB_ZCL_ON_OFF_SWITCH_CONFIGURATION_SWITCH_ACTIONS_MAX_VALUE < *value)
            {
                ret = RET_ERROR;
            }
            break;

        default:
            break;
    }

    return ret;
}

static void on_off_switch_configuration_server_write_attr_hook(zb_uint8_t  endpoint,
                                                               zb_uint16_t attr_id,
                                                               zb_uint8_t  *new_value,
                                                               zb_uint16_t manuf_code)
{
    tr_on_off_switch_configuration_server_write_attr_cb(endpoint, attr_id, new_value, manuf_code);
}

// on off switch configuration server cluster plugin init
void tr_on_off_switch_configuration_server_init(void)
{
    zb_zcl_add_cluster_handlers(TR_ZCL_CLUSTER_ON_OFF_SWITCH_CONFIGURATION_ID,
                                TR_ZCL_CLUSTER_SERVER_ROLE,
                                on_off_switch_configuration_server_check_value,
                                on_off_switch_configuration_server_write_attr_hook,
                                (zb_zcl_cluster_handler_t)NULL);

    tr_on_off_switch_configuration_server_init_cb();
}
