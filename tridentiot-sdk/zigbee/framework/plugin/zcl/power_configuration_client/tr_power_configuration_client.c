/// ****************************************************************************
/// @file tr_power_configuration_client.c
///
/// @brief ZCL POWER CONFIGURATION cluster client implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_power_configuration_client.h"

#define PLUGIN_NAME (zb_char_t*)("Power Configuration Client")

#ifdef POWER_CONFIGURATION_CLIENT_SUPPORTED_COMMANDS_RECEIVE
static zb_uint8_t gs_power_configuration_client_received_commands[] =
{
    POWER_CONFIGURATION_CLIENT_SUPPORTED_COMMANDS_RECEIVE
};
#endif

#ifdef POWER_CONFIGURATION_CLIENT_SUPPORTED_COMMANDS_SEND
static zb_uint8_t gs_power_configuration_client_generated_commands[] =
{
    POWER_CONFIGURATION_CLIENT_SUPPORTED_COMMANDS_SEND
};
#endif

static zb_discover_cmd_list_t gs_power_configuration_client_cmd_list =
{
#ifdef POWER_CONFIGURATION_CLIENT_SUPPORTED_COMMANDS_RECEIVE
    sizeof(gs_power_configuration_client_received_commands),  gs_power_configuration_client_received_commands,
#else
    0,                                                        NULL,
#endif
#ifdef POWER_CONFIGURATION_CLIENT_SUPPORTED_COMMANDS_SEND
    sizeof(gs_power_configuration_client_generated_commands), gs_power_configuration_client_generated_commands
#else
    0,                                                        NULL
#endif
};

static zb_bool_t power_configuration_client_cluster_handler(zb_uint8_t param)
{
    zb_zcl_parsed_hdr_t cmd_info;

    if (ZB_ZCL_GENERAL_GET_CMD_LISTS_PARAM == param)
    {
        ZCL_CTX().zb_zcl_cluster_cmd_list = &gs_power_configuration_client_cmd_list;
        return ZB_TRUE;
    }

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);
    ZB_ASSERT(TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID == cmd_info.cluster_id);

    tr_power_configuration_client_printf("RX:(%s) CMD: %02X EP: %02X\n",
                                         PLUGIN_NAME,
                                         cmd_info.cmd_id,
                                         cmd_info.addr_data.common_data.dst_endpoint);

    return ZB_TRUE;
}

void tr_power_configuration_client_init(void)
{
    zb_zcl_add_cluster_handlers(TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID,
                                TR_ZCL_CLUSTER_CLIENT_ROLE,
                                (zb_zcl_cluster_check_value_t)NULL,
                                (zb_zcl_cluster_write_attr_hook_t)NULL,
                                power_configuration_client_cluster_handler);

    tr_power_configuration_client_init_cb();
}
