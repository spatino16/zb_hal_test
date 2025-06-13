/// ****************************************************************************
/// @file tr_on_off_client.c
///
/// @brief ZCL ON/OFF cluster client implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_on_off_client.h"

#define PLUGIN_NAME (zb_char_t*)("On Off Client")

#ifdef ON_OFF_CLIENT_SUPPORTED_COMMANDS_RECEIVE
static zb_uint8_t gs_on_off_client_received_commands[] =
{
    ON_OFF_CLIENT_SUPPORTED_COMMANDS_RECEIVE
};
#endif

#ifdef ON_OFF_CLIENT_SUPPORTED_COMMANDS_SEND
static zb_uint8_t gs_on_off_client_generated_commands[] =
{
    ON_OFF_CLIENT_SUPPORTED_COMMANDS_SEND
};
#endif

static zb_discover_cmd_list_t gs_on_off_client_cmd_list =
{
#ifdef ON_OFF_CLIENT_SUPPORTED_COMMANDS_RECEIVE
    sizeof(gs_on_off_client_received_commands),  gs_on_off_client_received_commands,
#else
    0,                                           NULL,
#endif
#ifdef ON_OFF_CLIENT_SUPPORTED_COMMANDS_SEND
    sizeof(gs_on_off_client_generated_commands), gs_on_off_client_generated_commands
#else
    0,                                           NULL
#endif
};

static zb_bool_t on_off_client_cluster_handler(zb_uint8_t param)
{
    zb_zcl_parsed_hdr_t cmd_info;

    if (ZB_ZCL_GENERAL_GET_CMD_LISTS_PARAM == param)
    {
        ZCL_CTX().zb_zcl_cluster_cmd_list = &gs_on_off_client_cmd_list;
        return ZB_TRUE;
    }

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);
    ZB_ASSERT(TR_ZCL_CLUSTER_ON_OFF_ID == cmd_info.cluster_id);

    tr_on_off_client_printf("RX:(%s) CMD: %02X EP: %02X\n",
                            PLUGIN_NAME,
                            cmd_info.cmd_id,
                            cmd_info.addr_data.common_data.dst_endpoint);

    return ZB_TRUE;
}

void tr_on_off_client_init(void)
{
    zb_zcl_add_cluster_handlers(TR_ZCL_CLUSTER_ON_OFF_ID,
                                TR_ZCL_CLUSTER_CLIENT_ROLE,
                                (zb_zcl_cluster_check_value_t)NULL,
                                (zb_zcl_cluster_write_attr_hook_t)NULL,
                                on_off_client_cluster_handler);

    tr_on_off_client_init_cb();
}
