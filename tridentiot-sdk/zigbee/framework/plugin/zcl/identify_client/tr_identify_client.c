/// ****************************************************************************
/// @file tr_identify_client.c
///
/// @brief ZCL IDENTIFY cluster client implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_identify_client.h"

#define PLUGIN_NAME (zb_char_t*)("Identify Client")

#ifdef IDENTIFY_CLIENT_SUPPORTED_COMMANDS_RECEIVE
static zb_uint8_t gs_identify_client_received_commands[] =
{
    IDENTIFY_CLIENT_SUPPORTED_COMMANDS_RECEIVE
};
#endif
#ifdef IDENTIFY_CLIENT_SUPPORTED_COMMANDS_SEND
static zb_uint8_t gs_identify_client_generated_commands[] =
{
    IDENTIFY_CLIENT_SUPPORTED_COMMANDS_SEND
};
#endif

zb_discover_cmd_list_t gs_identify_client_cmd_list =
{
#ifdef IDENTIFY_CLIENT_SUPPORTED_COMMANDS_RECEIVE
    sizeof(gs_identify_client_received_commands),  gs_identify_client_received_commands,
#else
    0,                                             NULL,
#endif
#ifdef IDENTIFY_CLIENT_SUPPORTED_COMMANDS_SEND
    sizeof(gs_identify_client_generated_commands), gs_identify_client_generated_commands
#else
    0,                                             NULL,
#endif
};

static zb_bool_t identify_client_cluster_handler(zb_uint8_t param)
{
    zb_zcl_parsed_hdr_t cmd_info;

    if (ZB_ZCL_GENERAL_GET_CMD_LISTS_PARAM == param)
    {
        ZCL_CTX().zb_zcl_cluster_cmd_list = &gs_identify_client_cmd_list;
        return ZB_TRUE;
    }

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);
    ZB_ASSERT(TR_ZCL_CLUSTER_IDENTIFY_ID == cmd_info.cluster_id);

    if (cmd_info.cmd_direction == ZB_ZCL_FRAME_DIRECTION_TO_CLI &&
        cmd_info.cmd_id == TR_ZCL_CMD_IDENTIFY_QUERY_RESPONSE_ID)
    {
        tr_identify_client_printf("RX:(%s) Identify Query Resp, EP: %02X\n",
                                  PLUGIN_NAME,
                                  ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
        // TODO: revisit this to see if we want to add any additional hooks to bdb things
#if defined ZB_BDB_ENABLE_FINDING_BINDING

        if (ZCL_SELECTOR().process_identify_query_res != NULL)
        {
            ZCL_SELECTOR().process_identify_query_res(param);
        }
#endif /* defined ZB_BDB_ENABLE_FINDING_BINDING */

        tr_identify_client_identify_query_resp_cb(&cmd_info);
    }
    else
    {
        tr_identify_client_printf("RX:(%s) Uknown Command, CMD: %02X, EP: %02X\n",
                                  PLUGIN_NAME,
                                  cmd_info.cmd_id,
                                  ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
    }

    return ZB_TRUE;
}

void tr_identify_client_init(void)
{
    zb_zcl_add_cluster_handlers(TR_ZCL_CLUSTER_IDENTIFY_ID,
                                TR_ZCL_CLUSTER_CLIENT_ROLE,
                                (zb_zcl_cluster_check_value_t)NULL,
                                (zb_zcl_cluster_write_attr_hook_t)NULL,
                                identify_client_cluster_handler);

    tr_identify_client_init_cb();
}
