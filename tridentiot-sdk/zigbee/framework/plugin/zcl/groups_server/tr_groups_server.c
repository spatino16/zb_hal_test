/// ****************************************************************************
/// @file tr_groups_server.c
///
/// @brief ZCL GROUPS cluster server implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include <stdlib.h>
#include "tr_groups_server.h"
#include "tr_bind.h"

/* TODO
 * - revisit remove group functionality after adding scenes support
 * - test without scenes cluster to make sure things dont break
 */

#define PLUGIN_NAME (zb_char_t*)("Groups Server")

#ifdef GROUPS_SERVER_SUPPORTED_COMMANDS_RECEIVE
static zb_uint8_t gs_groups_server_received_commands[] =
{
    GROUPS_SERVER_SUPPORTED_COMMANDS_RECEIVE
};
#endif

#ifdef GROUPS_SERVER_SUPPORTED_COMMANDS_SEND
static zb_uint8_t gs_groups_server_generated_commands[] =
{
    GROUPS_SERVER_SUPPORTED_COMMANDS_SEND
};
#endif

static zb_discover_cmd_list_t gs_groups_server_cmd_list =
{
#ifdef GROUPS_SERVER_SUPPORTED_COMMANDS_RECEIVE
    sizeof(gs_groups_server_received_commands),  gs_groups_server_received_commands,
#else
    0,                                           NULL,
#endif
#ifdef GROUPS_SERVER_SUPPORTED_COMMANDS_SEND
    sizeof(gs_groups_server_generated_commands), gs_groups_server_generated_commands
#else
    0,                                           NULL
#endif
};

static zb_ret_t aps_status_to_zcl_status(zb_ret_t aps_status)
{
    zb_ret_t zcl_status;

    switch (aps_status)
    {
        case ERROR_CODE(ERROR_CATEGORY_APS, ZB_APS_STATUS_TABLE_FULL):
            zcl_status = TR_ZCL_STATUS_INSUFFICIENT_SPACE;
            break;

        case ERROR_CODE(ERROR_CATEGORY_APS, ZB_APS_STATUS_INVALID_GROUP):
            zcl_status = TR_ZCL_STATUS_NOT_FOUND;
            break;

        case ERROR_CODE(ERROR_CATEGORY_APS, ZB_APS_STATUS_SUCCESS):
            zcl_status = TR_ZCL_STATUS_SUCCESS;
            break;

        default:
            zcl_status = TR_ZCL_STATUS_FAILURE;
            break;
    }

    return zcl_status;
}

static void add_group_send_default_resp(zb_uint8_t          param,
                                        zb_zcl_parsed_hdr_t *cmd_info,
                                        zb_ret_t            status)
{
    if (ZB_ZCL_CHECK_IF_SEND_DEFAULT_RESP(*cmd_info, status))
    {
        ZB_ZCL_SEND_DEFAULT_RESP(
            param,
            ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).source.u.short_addr,
            ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
            ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).src_endpoint,
            ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).dst_endpoint,
            cmd_info->profile_id,
            cmd_info->cluster_id,
            cmd_info->seq_number,
            cmd_info->cmd_id,
            status);
    }
    else
    {
        zb_buf_free(param);
    }
}

static void add_group_send_default_resp_cb(zb_uint8_t param)
{
    zb_zcl_parsed_hdr_t       cmd_info;
    zb_apsme_add_group_conf_t *conf_param;
    zb_ret_t                  status;

    ZB_MEMCPY(&cmd_info, zb_buf_begin(param), sizeof(zb_zcl_parsed_hdr_t));
    conf_param = ZB_BUF_GET_PARAM(param, zb_apsme_add_group_conf_t);
    status     = aps_status_to_zcl_status(conf_param->status);

    /* Send default response, if enabled */
    add_group_send_default_resp(param, &cmd_info, status);
}

static void send_add_group_resp(zb_uint8_t          param,
                                zb_zcl_parsed_hdr_t *cmd_info,
                                zb_ret_t            status,
                                zb_uint16_t         group_id)
{
    zb_uint8_t *resp_data;

    /* Construct response packet header */
    resp_data = ZB_ZCL_START_PACKET(param);

    /* NOTE: currently, manufacturer specific is not supported */
    ZB_ZCL_CONSTRUCT_SPECIFIC_COMMAND_RES_FRAME_CONTROL(resp_data);
    ZB_ZCL_CONSTRUCT_COMMAND_HEADER(resp_data, cmd_info->seq_number, TR_ZCL_CMD_ADD_GROUP_RESPONSE_ID);

    /* Add group response format, ZCL8 spec 3.6.2.4.1 */
    /* | status 1b | group id 2b | */

    ZB_ZCL_PACKET_PUT_DATA8(resp_data, status);
    ZB_ZCL_PACKET_PUT_DATA16_VAL(resp_data, group_id);

    ZB_ZCL_FINISH_N_SEND_PACKET(param,
                                resp_data,
                                ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).source.u.short_addr,
                                ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).src_endpoint,
                                ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).dst_endpoint,
                                cmd_info->profile_id,
                                cmd_info->cluster_id,
                                NULL);
}

static void send_add_group_resp_cb(zb_uint8_t param)
{
    zb_zcl_parsed_hdr_t       cmd_info;
    zb_apsme_add_group_conf_t *conf_param;
    zb_ret_t                  status;

    ZB_MEMCPY(&cmd_info, zb_buf_begin(param), sizeof(zb_zcl_parsed_hdr_t));

    conf_param = ZB_BUF_GET_PARAM(param, zb_apsme_add_group_conf_t);
    status     = aps_status_to_zcl_status(conf_param->status);

    send_add_group_resp(param, &cmd_info, status, conf_param->group_address);
}

/* Just do nothing, used as gag */
static void dummy_handler(zb_uint8_t unused)
{
    ZVUNUSED(unused);
}

static void send_remove_group_resp(zb_uint8_t          param,
                                   zb_zcl_parsed_hdr_t *cmd_info,
                                   zb_ret_t            status,
                                   zb_uint16_t         group_id)
{
    zb_uint8_t *resp_data;

    /* Construct response packet header */
    resp_data = ZB_ZCL_START_PACKET(param);

    /* NOTE: currently, manufacturer specific is not supported */
    ZB_ZCL_CONSTRUCT_SPECIFIC_COMMAND_RES_FRAME_CONTROL(resp_data);
    ZB_ZCL_CONSTRUCT_COMMAND_HEADER(
        resp_data,
        cmd_info->seq_number,
        TR_ZCL_CMD_REMOVE_GROUP_RESPONSE_ID);

    /* Remove group response format, ZCL8 spec 3.6.2.4.4.1 */
    /* | status 1b | group id 2b | */

    ZB_ZCL_PACKET_PUT_DATA8(resp_data, status);
    ZB_ZCL_PACKET_PUT_DATA16_VAL(resp_data, group_id);

    ZB_ZCL_FINISH_N_SEND_PACKET(param,
                                resp_data,
                                ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).source.u.short_addr,
                                ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).src_endpoint,
                                ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).dst_endpoint,
                                cmd_info->profile_id,
                                cmd_info->cluster_id,
                                NULL);
}

static void send_remove_group_resp_cb(zb_uint8_t param)
{
    zb_zcl_parsed_hdr_t          cmd_info;
    zb_apsme_remove_group_conf_t *conf_param;
    zb_ret_t                     status;

    ZB_MEMCPY(&cmd_info, zb_buf_begin(param), sizeof(zb_zcl_parsed_hdr_t));

    conf_param = ZB_BUF_GET_PARAM(param, zb_apsme_remove_group_conf_t);
    status     = aps_status_to_zcl_status(conf_param->status);

    send_remove_group_resp(param, &cmd_info, status, conf_param->group_address);
}

static void remove_all_groups_send_default_resp_cb(zb_uint8_t param)
{
    zb_zcl_parsed_hdr_t               cmd_info;
    zb_apsme_remove_all_groups_conf_t *conf_param;
    zb_uint8_t                        *resp_data;
    zb_ret_t                          status;

    ZB_MEMCPY(&cmd_info, zb_buf_begin(param), sizeof(zb_zcl_parsed_hdr_t));

    conf_param = ZB_BUF_GET_PARAM(param, zb_apsme_remove_all_groups_conf_t);

    /* Map APS status to ZCL status */
    status = aps_status_to_zcl_status(conf_param->status);

    /* If no groups found answer with success status. See ZCL8 Spec  3.6.2.3.6.2 */
    if (status == TR_ZCL_STATUS_NOT_FOUND)
    {
        status = TR_ZCL_STATUS_SUCCESS;
    }

    /* Construct response packet header */
    resp_data = ZB_ZCL_START_PACKET(param);

    /* NOTE: currently, manufacturer specific is not supported */
    ZB_ZCL_CONSTRUCT_SPECIFIC_COMMAND_RES_FRAME_CONTROL(resp_data);
    ZB_ZCL_CONSTRUCT_COMMAND_HEADER(
        resp_data,
        cmd_info.seq_number,
        TR_ZCL_CMD_REMOVE_GROUP_RESPONSE_ID);

    /* Send default response, if enabled */
    if (!cmd_info.disable_default_response)
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
            status);
    }
    else
    {
        zb_buf_free(param);
    }
}

static void add_group_handler(zb_uint8_t param,
                              zb_bool_t  check_identifying)
{
    zb_apsme_add_group_req_t *aps_req;
    zb_ret_t                 status    = TR_ZCL_STATUS_SUCCESS;
    zb_bool_t                add_group = ZB_TRUE;
    zb_zcl_parsed_hdr_t      cmd_info;
    zb_zcl_parsed_hdr_t      *resp_cmd_info            = NULL;
    zb_bool_t                respond_with_default_resp = ZB_FALSE;
    zb_uint16_t              group_id                  = 0;
    zb_char_t                group_name[17]            = { 0 };

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);

    // extract group id and optional group name
    if (zb_buf_len(param) >= sizeof(zb_uint16_t))
    {
        zb_uint8_t *buf = zb_buf_begin(param);
        group_id        = (zb_uint16_t)((buf[1] << 8) | buf[0]);
        group_name[0]   = (zb_uint8_t)(buf[2] > 16 ? 16 : buf[2]);

        if (group_name[0] > 0)
        {
            ZB_MEMCPY(&group_name[1], (zb_char_t*)&buf[3], group_name[0]);
        }
    }
    else
    {
        group_id      = ZB_ZCL_NULL_ID;
        group_name[0] = 0;
    }

    tr_groups_server_printf("RX:(%s) Add Group ", PLUGIN_NAME);

    /* ZCL8 spec. 3.6.2.3.7.2 Effect on Receipt (for Add Group If Identifying Command)
     * 1. The device verifies that it is currently identifying itself...
     */
    if (check_identifying)
    {
        add_group = zb_zcl_is_identifying(ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint) ? ZB_TRUE : ZB_FALSE;

        tr_groups_server_printf("If Identifying Cmd, EP: %02X, ID: %04X\n",
                                ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
                                group_id);

        if (tr_groups_server_add_group_if_identifying_cb(add_group,
                                                         ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
                                                         group_id,
                                                         group_name))
        {
            zb_buf_free(param);
            return;
        }
    }
    else
    {
        tr_groups_server_printf("Cmd, EP: %02X, ID: %04X\n",
                                ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
                                group_id);

        if (tr_groups_server_add_group_cb(ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
                                          group_id,
                                          group_name))
        {
            zb_buf_free(param);
            return;
        }
    }

    if (add_group)
    {
        /* ZCL8 spec. 3.6.2.3.2.2, 3.6.2.3.7.2: The device verifies that the Group ID field contains a valid group identifier
         * in the range 0x0001 – 0xfff7. If the Group ID field contains a group identifier outside this range,
         * the status SHALL be INVALID_VALUE and the device continues from step 5.*/
        if (group_id >= ZB_ZCL_ATTR_SCENES_CURRENT_GROUP_MIN_VALUE &&
            group_id <= ZB_ZCL_ATTR_SCENES_CURRENT_GROUP_MAX_VALUE)
        {
            zb_buf_reuse(param);

            aps_req                = ZB_BUF_GET_PARAM(param, zb_apsme_add_group_req_t);
            aps_req->group_address = group_id;
            aps_req->endpoint      = ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint;

            /* zb_apsme_add_group_request calls ZDO_RUN_CALLBACK_BY_INDEX that
               kills buffer if callback not found; register dummy handler
               to avoid this situation. */
            aps_req->confirm_cb = NULL;

            if (!check_identifying)
            {
                if (ZB_APS_FC_GET_DELIVERY_MODE(cmd_info.addr_data.common_data.fc) == ZB_APS_DELIVERY_UNICAST)
                {
                    aps_req->confirm_cb = send_add_group_resp_cb;
                }
            }
            else
            {
                aps_req->confirm_cb = add_group_send_default_resp_cb;
            }

            resp_cmd_info = zb_buf_initial_alloc(param, sizeof(zb_zcl_parsed_hdr_t));
            ZB_MEMCPY(resp_cmd_info, &cmd_info, sizeof(zb_zcl_parsed_hdr_t));

#ifdef TR_GROUPS_SERVER_AUTO_BIND_ENABLE
            tr_bind_add_group_binding(aps_req->endpoint, aps_req->group_address);
#endif

            zb_zdo_add_group_req(param);
        }
        else
        {
            tr_groups_server_printf("Error, invalid group ID for add group request\n");
            status = TR_ZCL_STATUS_INVALID_VALUE;

            if (!check_identifying)
            {
                if (ZB_APS_FC_GET_DELIVERY_MODE(cmd_info.addr_data.common_data.fc) == ZB_APS_DELIVERY_UNICAST)
                {
                    send_add_group_resp(param, &cmd_info, status, group_id);
                }
                else
                {
                    /* Broadcast or Groupcast Add Group - drop packet */
                    zb_buf_free(param);
                }
            }
            else
            {
                respond_with_default_resp = ZB_TRUE;
            }
        }
    }
    else
    {
        /* ZCL8 spec. 3.6.2.3.7.2 Effect on Receipt (for Add Group If Identifying Command)
         * 1. ... If the device it not currently identifying itself ...
         */
        respond_with_default_resp = ZB_TRUE;
    } /* id add_group */

    if (respond_with_default_resp)
    {
        if (!add_group)
        {
            /* ZCL8 spec. 3.6.2.3.7.2 Effect on Receipt (for Add Group If Identifying Command)
             * 1. ...If the device it not currently identifying
             * itself, the Add Group If Identifying command was received as unicast and a default response is
             * requested, the device SHALL generate a Default Response command with the Status field set to
             * SUCCESS and SHALL transmit it back to the originator of the Add Group If Identifying command...
             * Test spec, G-TC-02S - status should be SUCCESS.
             */
            status = TR_ZCL_STATUS_SUCCESS;
        }

        add_group_send_default_resp(param, &cmd_info, status);
    }
}

static void view_group_handler(zb_uint8_t param)
{
    zb_zcl_groups_view_group_req_t       view_group_req;
    zb_uint8_t                           *resp_data;
    zb_uint8_t                           status = TR_ZCL_STATUS_SUCCESS;
    zb_apsme_get_group_membership_req_t  *aps_req;
    zb_apsme_get_group_membership_conf_t *conf;
    zb_zcl_parsed_hdr_t                  cmd_info;
    zb_char_t                            group_name[17] = { 0 };

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);
    ZB_ZCL_GROUPS_GET_VIEW_GROUP_REQ(param, view_group_req);

    tr_groups_server_printf("RX:(%s) View Group Cmd, EP: %02X, ID: %04X\n",
                            PLUGIN_NAME,
                            ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
                            view_group_req.group_id);

    if (view_group_req.group_id >= ZB_ZCL_ATTR_SCENES_CURRENT_GROUP_MIN_VALUE &&
        view_group_req.group_id <= ZB_ZCL_ATTR_SCENES_CURRENT_GROUP_MAX_VALUE)
    {

        if (tr_groups_server_view_group_cb(ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
                                           view_group_req.group_id,
                                           group_name))
        {
            zb_buf_free(param);
            return;
        }

        zb_buf_reuse(param);
        aps_req = zb_buf_initial_alloc(param, sizeof(zb_apsme_get_group_membership_req_t));

        aps_req->n_groups  = 1;
        aps_req->groups[0] = view_group_req.group_id;
        aps_req->endpoint  = ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint;

        /* TODO: do not rely on function synchronicity and send response from confirm_cb */
        /* See comment to zb_apsme_add_group_request() call */
        aps_req->confirm_cb = dummy_handler;
        zb_zdo_get_group_membership_req(param);
        conf = ZB_BUF_GET_PARAM(param, zb_apsme_get_group_membership_conf_t);

        if (!conf->n_groups)
        {
            status = TR_ZCL_STATUS_NOT_FOUND;
        }
    }
    else
    {
        tr_groups_server_printf("Error, invalid group ID for view group request\n");
        status = TR_ZCL_STATUS_INVALID_VALUE;
    }

    /** [ZB_ZCL_CONSTRUCT_FRAME_HEADER] */
    /* Construct response packet header */
    resp_data = ZB_ZCL_START_PACKET(param);

    /* NOTE: currently, manufacturer specific is not supported */
    ZB_ZCL_CONSTRUCT_SPECIFIC_COMMAND_RES_FRAME_CONTROL(resp_data);
    ZB_ZCL_CONSTRUCT_COMMAND_HEADER(
        resp_data,
        cmd_info.seq_number,
        TR_ZCL_CMD_VIEW_GROUP_RESPONSE_ID);
    /** [ZB_ZCL_CONSTRUCT_FRAME_HEADER] */

    /* View group response format, ZCL8 spec 3.6.2.4.2.1 */
    /* | status 1b | group id 2b | Group name XXb| */

    ZB_ZCL_PACKET_PUT_DATA8(resp_data, status);
    ZB_ZCL_PACKET_PUT_DATA16_VAL(resp_data, view_group_req.group_id);

    // populate group name if the application passes one back from the callback
    if (group_name[0] > 0)
    {
        ZB_ZCL_PACKET_PUT_DATA_N(resp_data, group_name, group_name[0] + 1);
    }
    else
    {
        ZB_ZCL_PACKET_PUT_DATA8(resp_data, ZB_ZCL_NULL_STRING);
    }

    ZB_ZCL_FINISH_N_SEND_PACKET(param,
                                resp_data,
                                ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).source.u.short_addr,
                                ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).src_endpoint,
                                ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
                                cmd_info.profile_id,
                                cmd_info.cluster_id,
                                NULL);
}

static void get_group_membership_handler(zb_uint8_t param)
{
    zb_zcl_groups_get_group_membership_req_t *get_member_req;
    zb_uint8_t                               *resp_data;
    zb_apsme_get_group_membership_req_t      *aps_req;
    zb_apsme_get_group_membership_conf_t     *conf;
    zb_ushort_t                              i;
    zb_zcl_parsed_hdr_t                      cmd_info;
    zb_bool_t                                recv_pkt_is_valid = ZB_FALSE;

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);
    ZB_ZCL_GROUPS_GET_GROUP_MEMBERSHIP_REQ(param, get_member_req);

    tr_groups_server_printf("RX:(%s) Get Group Membership Cmd, EP: %02X\n",
                            PLUGIN_NAME,
                            ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);

    recv_pkt_is_valid = (get_member_req) ? ZB_TRUE : ZB_FALSE;

    // reuse runtime buffer - to simplify passing parameters
    zb_buf_reuse(ZCL_CTX().runtime_buf);

    if (recv_pkt_is_valid)
    {
        aps_req = zb_buf_initial_alloc(
            ZCL_CTX().runtime_buf,
            (sizeof(zb_apsme_get_group_membership_req_t)
             + (get_member_req->group_count - 1) * sizeof(zb_uint16_t)));

        aps_req->n_groups = get_member_req->group_count;
        ZB_MEMCPY(aps_req->groups,
                  get_member_req->group_id,
                  get_member_req->group_count * sizeof(zb_uint16_t));

        /* TODO: do not rely on function synchronicity and send response from confirm_cb */
        aps_req->confirm_cb = dummy_handler;
        aps_req->endpoint   = cmd_info.addr_data.common_data.dst_endpoint;
        zb_zdo_get_group_membership_req(ZCL_CTX().runtime_buf);
    }

    conf = ZB_BUF_GET_PARAM(ZCL_CTX().runtime_buf, zb_apsme_get_group_membership_conf_t);

    // copy group ID list into a separate buffer to prevent compiler warning
    zb_uint16_t group_ids[ZB_APS_GROUP_TABLE_SIZE] = { 0 };
    ZB_MEMCPY(group_ids, get_member_req->group_id, get_member_req->group_count * sizeof(zb_uint16_t));

    if (tr_groups_server_get_group_membership_cb(ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
                                                 get_member_req->group_count,
                                                 group_ids))
    {
        zb_buf_free(param);
        return;
    }

    /* Construct response packet header */
    resp_data = ZB_ZCL_START_PACKET(param);

    /* NOTE: currently, manufacturer specific is not supported */
    ZB_ZCL_CONSTRUCT_SPECIFIC_COMMAND_RES_FRAME_CONTROL(resp_data);
    ZB_ZCL_CONSTRUCT_COMMAND_HEADER(
        resp_data,
        cmd_info.seq_number,
        TR_ZCL_CMD_GET_GROUP_MEMBERSHIP_RESPONSE_ID);

    if (recv_pkt_is_valid)
    {
        /* Get group membership response format, ZCL8 spec 3.6.2.4.3.1 */
        /* | capacity 1b | group count 1b | Group id 2b x NN | */

        ZB_ZCL_PACKET_PUT_DATA8(resp_data, conf->capacity);
        ZB_ZCL_PACKET_PUT_DATA8(resp_data, conf->n_groups);

        for (i = 0 ; i < conf->n_groups ; i++)
        {
            ZB_ZCL_PACKET_PUT_DATA16_VAL(resp_data, conf->groups[i]);
        }
    }
    else
    {
        ZB_ZCL_PACKET_PUT_DATA8(resp_data, 0);
        ZB_ZCL_PACKET_PUT_DATA8(resp_data, 0);
    }

    ZB_ZCL_FINISH_N_SEND_PACKET(param,
                                resp_data,
                                ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).source.u.short_addr,
                                ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).src_endpoint,
                                ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
                                cmd_info.profile_id,
                                cmd_info.cluster_id,
                                NULL);

}

static void remove_group_handler(zb_uint8_t param)
{
    zb_zcl_groups_remove_group_req_t rem_group_req;
    zb_apsme_remove_group_req_t      *aps_req;
    zb_ret_t                         status = TR_ZCL_STATUS_SUCCESS;
    zb_zcl_parsed_hdr_t              cmd_info;
    zb_zcl_parsed_hdr_t              *resp_cmd_info;

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);
    ZB_ZCL_GROUPS_GET_REMOVE_GROUP_REQ(param, rem_group_req);

    tr_groups_server_printf("RX:(%s) Remove Group Cmd, EP: %02X, ID: %04X\n",
                            PLUGIN_NAME,
                            ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
                            rem_group_req.group_id);

    if (tr_groups_server_remove_group_cb(ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
                                         rem_group_req.group_id))
    {
        zb_buf_free(param);
        return;
    }

    zb_buf_reuse(param);

    resp_cmd_info = zb_buf_initial_alloc(param, sizeof(zb_zcl_parsed_hdr_t));
    ZB_MEMCPY(resp_cmd_info, &cmd_info, sizeof(zb_zcl_parsed_hdr_t));

    if (rem_group_req.group_id >= ZB_ZCL_ATTR_SCENES_CURRENT_GROUP_MIN_VALUE &&
        rem_group_req.group_id <= ZB_ZCL_ATTR_SCENES_CURRENT_GROUP_MAX_VALUE)
    {
        aps_req                = ZB_BUF_GET_PARAM(param, zb_apsme_remove_group_req_t);
        aps_req->group_address = rem_group_req.group_id;
        aps_req->endpoint      = ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint;

#ifdef ZB_ZCL_SUPPORT_CLUSTER_SCENES
        // TODO: revisit this after adding scenes plugin
        /* Remove associated scenes */
        zb_buf_get_out_delayed_ext(zb_zcl_scenes_remove_all_scenes_in_all_endpoints_by_group_id,
                                   rem_group_req.group_id,
                                   0);
#endif

#ifdef TR_GROUPS_SERVER_AUTO_BIND_ENABLE
        tr_bind_remove_group_binding(aps_req->endpoint, aps_req->group_address);
#endif

        /* See comment for zb_apsme_add_group_request() call */
        aps_req->confirm_cb = send_remove_group_resp_cb;
        zb_zdo_remove_group_req(param);
    }
    else
    {
        tr_groups_server_printf("Error, invalid group ID for remove group request\n");
        status = TR_ZCL_STATUS_INVALID_VALUE;

        send_remove_group_resp(param, &cmd_info, status, rem_group_req.group_id);
    }
}

static void remove_all_groups_handler(zb_uint8_t param)
{
    zb_apsme_remove_all_groups_req_t *aps_req;
    zb_zcl_parsed_hdr_t              cmd_info;
    zb_zcl_parsed_hdr_t              *resp_cmd_info;

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);

    tr_groups_server_printf("RX:(%s) Remove All Groups Cmd, EP: %02X\n",
                            PLUGIN_NAME,
                            ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);

    if (tr_groups_server_remove_all_groups_cb(ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint))
    {
        zb_buf_free(param);
        return;
    }

    zb_buf_reuse(param);

    resp_cmd_info = zb_buf_initial_alloc(param, sizeof(zb_zcl_parsed_hdr_t));
    ZB_MEMCPY(resp_cmd_info, &cmd_info, sizeof(zb_zcl_parsed_hdr_t));

    aps_req           = ZB_BUF_GET_PARAM(param, zb_apsme_remove_all_groups_req_t);
    aps_req->endpoint = ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint;

#ifdef ZB_ZCL_SUPPORT_CLUSTER_SCENES
    // TODO: revisit this after adding scenes plugin
    /* Remove scenes associated with every group in every endpoint */
    zb_buf_get_out_delayed(zb_zcl_scenes_remove_all_scenes_in_all_endpoints);
#endif

    /* See comment for zb_apsme_add_group_request() call */
    aps_req->confirm_cb = remove_all_groups_send_default_resp_cb;

#ifdef TR_GROUPS_SERVER_AUTO_BIND_ENABLE
    tr_bind_remove_all_groups_bindings(aps_req->endpoint);
#endif

    zb_zdo_remove_all_groups_req(param);
}

static zb_ret_t groups_server_check_value(zb_uint16_t attr_id,
                                          zb_uint8_t  endpoint,
                                          zb_uint8_t  *value)
{
    ZVUNUSED(attr_id);
    ZVUNUSED(value);
    ZVUNUSED(endpoint);

    /* All values for mandatory attributes are allowed, extra check for
     * optional attributes is needed */

    return RET_OK;
}

static zb_bool_t groups_server_cluster_handler(zb_uint8_t param)
{
    zb_bool_t           processed = ZB_TRUE;
    zb_zcl_parsed_hdr_t *cmd_info;

    if (ZB_ZCL_GENERAL_GET_CMD_LISTS_PARAM == param)
    {
        ZCL_CTX().zb_zcl_cluster_cmd_list = &gs_groups_server_cmd_list;
        return ZB_TRUE;
    }

    cmd_info = ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t);
    ZB_ASSERT(TR_ZCL_CLUSTER_GROUPS_ID == cmd_info->cluster_id);

    if (cmd_info->cmd_direction == ZB_ZCL_FRAME_DIRECTION_TO_SRV)
    {
        switch (cmd_info->cmd_id)
        {
            case TR_ZCL_CMD_ADD_GROUP_ID:
                add_group_handler(param, ZB_FALSE);
                break;

            case TR_ZCL_CMD_VIEW_GROUP_ID:
                view_group_handler(param);
                break;

            case TR_ZCL_CMD_GET_GROUP_MEMBERSHIP_ID:
                get_group_membership_handler(param);
                break;

            case TR_ZCL_CMD_REMOVE_GROUP_ID:
                remove_group_handler(param);
                break;

            case TR_ZCL_CMD_REMOVE_ALL_GROUPS_ID:
                remove_all_groups_handler(param);
                break;

            case TR_ZCL_CMD_ADD_GROUP_IF_IDENTIFYING_ID:
                add_group_handler(param, ZB_TRUE);
                break;

            default:
                processed = ZB_FALSE;
        }
    }
    else
    {
        zb_buf_free(param);
    }

    return processed;
}

// groups server cluster plugin init
void tr_groups_server_init(void)
{
    zb_zcl_add_cluster_handlers(TR_ZCL_CLUSTER_GROUPS_ID,
                                TR_ZCL_CLUSTER_SERVER_ROLE,
                                groups_server_check_value,
                                (zb_zcl_cluster_write_attr_hook_t)NULL,
                                groups_server_cluster_handler);

    tr_groups_server_init_cb();
}
