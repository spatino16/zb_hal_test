/// ****************************************************************************
/// @file tr_basic_client.c
///
/// @brief ZCL BASIC cluster client implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_basic_client.h"

#define PLUGIN_NAME (zb_char_t*)("Basic Client")

#ifdef BASIC_CLIENT_SUPPORTED_COMMANDS_RECEIVE
static zb_uint8_t gs_basic_client_received_commands[] =
{
    BASIC_CLIENT_SUPPORTED_COMMANDS_RECEIVE
};
#endif

#ifdef BASIC_CLIENT_SUPPORTED_COMMANDS_SEND
static zb_uint8_t gs_basic_client_generated_commands[] =
{
    BASIC_CLIENT_SUPPORTED_COMMANDS_SEND
};
#endif

static zb_discover_cmd_list_t gs_basic_client_cmd_list =
{
#ifdef BASIC_CLIENT_SUPPORTED_COMMANDS_RECEIVE
    sizeof(gs_basic_client_received_commands),  gs_basic_client_received_commands,
#else
    0,                                          NULL,
#endif
#ifdef BASIC_CLIENT_SUPPORTED_COMMANDS_SEND
    sizeof(gs_basic_client_generated_commands), gs_basic_client_generated_commands
#else
    0,                                          NULL
#endif
};

static zb_bool_t basic_client_cluster_handler(zb_uint8_t param)
{
    zb_zcl_parsed_hdr_t cmd_info;

    if (ZB_ZCL_GENERAL_GET_CMD_LISTS_PARAM == param)
    {
        ZCL_CTX().zb_zcl_cluster_cmd_list = &gs_basic_client_cmd_list;
        return ZB_TRUE;
    }

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);
    ZB_ASSERT(TR_ZCL_CLUSTER_BASIC_ID == cmd_info.cluster_id);

    tr_basic_client_printf("RX:(%s) CMD: %02X EP: %02X\n",
                           PLUGIN_NAME,
                           cmd_info.cmd_id,
                           cmd_info.addr_data.common_data.dst_endpoint);

    return ZB_TRUE;
}

void tr_basic_client_init(void)
{
    zb_zcl_add_cluster_handlers(TR_ZCL_CLUSTER_BASIC_ID,
                                TR_ZCL_CLUSTER_CLIENT_ROLE,
                                (zb_zcl_cluster_check_value_t)NULL,
                                (zb_zcl_cluster_write_attr_hook_t)NULL,
                                basic_client_cluster_handler);

    tr_basic_client_init_cb();
}
