/// ****************************************************************************
/// @file tr_groups_client.c
///
/// @brief ZCL GROUPS cluster client implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include <stdlib.h>
#include "tr_groups_client.h"

#define PLUGIN_NAME (zb_char_t*)("Groups Client")

#ifdef GROUPS_CLIENT_SUPPORTED_COMMANDS_RECEIVE
static zb_uint8_t gs_groups_client_received_commands[] =
{
    GROUPS_CLIENT_SUPPORTED_COMMANDS_RECEIVE
};
#endif

#ifdef GROUPS_CLIENT_SUPPORTED_COMMANDS_SEND
static zb_uint8_t gs_groups_client_generated_commands[] =
{
    GROUPS_CLIENT_SUPPORTED_COMMANDS_SEND
};
#endif

static zb_discover_cmd_list_t gs_groups_client_cmd_list =
{
#ifdef GROUPS_CLIENT_SUPPORTED_COMMANDS_RECEIVE
    sizeof(gs_groups_client_received_commands),  gs_groups_client_received_commands,
#else
    0,                                           NULL,
#endif
#ifdef GROUPS_CLIENT_SUPPORTED_COMMANDS_SEND
    sizeof(gs_groups_client_generated_commands), gs_groups_client_generated_commands
#else
    0,                                           NULL
#endif
};

zb_bool_t groups_client_cluster_handler(zb_uint8_t param)
{
    zb_bool_t                                processed = ZB_TRUE;
    zb_zcl_parsed_hdr_t                      cmd_info;
    zb_zcl_groups_add_group_res_t            *add_group_resp;
    tr_zcl_groups_view_group_res_t           view_group_resp;
    zb_zcl_groups_get_group_membership_res_t *get_membership_resp;
    zb_zcl_groups_remove_group_res_t         *rem_group_resp;
    zb_uint16_t                              *group_id_list;

    if (ZB_ZCL_GENERAL_GET_CMD_LISTS_PARAM == param)
    {
        ZCL_CTX().zb_zcl_cluster_cmd_list = &gs_groups_client_cmd_list;
        return ZB_TRUE;
    }

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);
    ZB_ASSERT(cmd_info.cluster_id == TR_ZCL_CLUSTER_GROUPS_ID);

    if (cmd_info.cmd_direction == ZB_ZCL_FRAME_DIRECTION_TO_CLI)
    {
        switch (cmd_info.cmd_id)
        {
            case TR_ZCL_CMD_ADD_GROUP_RESPONSE_ID:
                ZB_ZCL_GROUPS_GET_ADD_GROUP_RES(param, add_group_resp);

                tr_groups_client_printf("RX:(%s) Add Group Resp\n EP: %02X\n ID: %04X\n STATUS: %X\n",
                                        PLUGIN_NAME,
                                        cmd_info.addr_data.common_data.dst_endpoint,
                                        add_group_resp->group_id,
                                        add_group_resp->status);

                if (tr_groups_client_add_group_resp_cb(cmd_info.addr_data.common_data.dst_endpoint,
                                                       add_group_resp->group_id,
                                                       add_group_resp->status))
                {
                    zb_buf_free(param);
                    return ZB_TRUE;
                }
                processed = ZB_TRUE;
                break;

            case TR_ZCL_CMD_VIEW_GROUP_RESPONSE_ID:
                // extract response data
                if (zb_buf_len(param) >= sizeof(zb_uint32_t))
                {
                    zb_uint8_t *buf               = zb_buf_begin(param);
                    view_group_resp.status        = (zb_uint8_t)buf[0];
                    view_group_resp.group_id      = (zb_uint16_t)((buf[2] << 8) | buf[1]);
                    view_group_resp.group_name[0] = (zb_uint8_t)buf[3];

                    if (view_group_resp.group_name[0] > 0)
                    {
                        ZB_MEMCPY(&view_group_resp.group_name[1], (zb_char_t*)&buf[4], view_group_resp.group_name[0]);
                    }
                }
                else
                {
                    view_group_resp.status        = TR_ZCL_STATUS_MALFORMED_COMMAND;
                    view_group_resp.group_id      = ZB_ZCL_NULL_ID;
                    view_group_resp.group_name[0] = 0;
                }

                tr_groups_client_printf("RX:(%s) View Group Resp\n EP: %02X\n ID: %04X\n NAME: ",
                                        PLUGIN_NAME,
                                        cmd_info.addr_data.common_data.dst_endpoint,
                                        view_group_resp.group_id);

                for (zb_uint8_t i = 0 ; i < view_group_resp.group_name[0] ; i++)
                {
                    tr_groups_client_printf("%c", view_group_resp.group_name[i + 1]);
                }
                tr_groups_client_printf("\n STATUS: %X\n", view_group_resp.status);

                if (tr_groups_client_view_group_resp_cb(cmd_info.addr_data.common_data.dst_endpoint,
                                                        view_group_resp.group_id,
                                                        view_group_resp.group_name,
                                                        view_group_resp.status))
                {
                    zb_buf_free(param);
                    return ZB_TRUE;
                }
                processed = ZB_TRUE;
                break;

            case TR_ZCL_CMD_GET_GROUP_MEMBERSHIP_RESPONSE_ID:
                ZB_ZCL_GROUPS_GET_GROUP_MEMBERSHIP_RES(param, get_membership_resp);

                tr_groups_client_printf("RX:(%s) Get Group Membership Resp\n EP: %02X\n Capacity: %d\n",
                                        PLUGIN_NAME,
                                        cmd_info.addr_data.common_data.dst_endpoint,
                                        get_membership_resp->capacity);
                tr_groups_client_printf(" Count: %d\n Group IDs: ", get_membership_resp->group_count);

                for (zb_uint8_t i = 0 ; i < get_membership_resp->group_count ; i++)
                {
                    tr_groups_client_printf("%04X, ", get_membership_resp->group_id[i]);
                }
                tr_groups_client_printf("\n");

                group_id_list = (zb_uint16_t*)malloc(get_membership_resp->group_count * sizeof(zb_uint16_t));
                ZB_MEMCPY(group_id_list, get_membership_resp->group_id, (size_t)(get_membership_resp->group_count * sizeof(zb_uint16_t)));

                if (tr_groups_client_get_group_membership_resp_cb(cmd_info.addr_data.common_data.dst_endpoint,
                                                                  get_membership_resp->capacity,
                                                                  get_membership_resp->group_count,
                                                                  group_id_list))
                {
                    zb_buf_free(param);
                    free(group_id_list);
                    return ZB_TRUE;
                }
                free(group_id_list);
                processed = ZB_TRUE;
                break;

            case TR_ZCL_CMD_REMOVE_GROUP_RESPONSE_ID:
                ZB_ZCL_GROUPS_GET_REMOVE_GROUP_RES(param, rem_group_resp);

                tr_groups_client_printf("RX:(%s) Remove Group Resp\n EP: %02X\n ID: %04X\n STATUS: %X\n",
                                        PLUGIN_NAME,
                                        cmd_info.addr_data.common_data.dst_endpoint,
                                        rem_group_resp->group_id,
                                        rem_group_resp->status);

                if (tr_groups_client_remove_group_resp_cb(cmd_info.addr_data.common_data.dst_endpoint,
                                                          rem_group_resp->group_id,
                                                          rem_group_resp->status))
                {
                    zb_buf_free(param);
                    return ZB_TRUE;
                }
                processed = ZB_TRUE;
                break;

            default:
                tr_groups_client_printf("cmd not supported\n");
                processed = ZB_FALSE;
        }
    }
    else
    {
        tr_groups_client_printf("direction - to server, skip cmd\n");
        processed = ZB_FALSE;
    }

    /* Do nothing, just send default response if necessary */
    if (!cmd_info.disable_default_response && processed)
    {
        ZB_ZCL_SEND_DEFAULT_RESP(
            param,
            ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).source.u.short_addr,
            ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
            ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).src_endpoint,
            ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
            cmd_info.profile_id,
            cmd_info.cluster_id,
            cmd_info.seq_number,
            cmd_info.cmd_id,
            TR_ZCL_STATUS_SUCCESS);
    }
    else
    {
        zb_buf_free(param);
    }

    return processed;
}

// groups client cluster plugin init
void tr_groups_client_init(void)
{
    zb_zcl_add_cluster_handlers(TR_ZCL_CLUSTER_GROUPS_ID,
                                TR_ZCL_CLUSTER_CLIENT_ROLE,
                                (zb_zcl_cluster_check_value_t)NULL,
                                (zb_zcl_cluster_write_attr_hook_t)NULL,
                                groups_client_cluster_handler);

    tr_groups_client_init_cb();
}
