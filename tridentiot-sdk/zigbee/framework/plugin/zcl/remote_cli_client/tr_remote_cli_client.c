/// ****************************************************************************
/// @file tr_remote_cli_client.c
///
/// @brief ZCL REMOTE CLI cluster client implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_remote_cli_client.h"

#define PLUGIN_NAME (zb_char_t*)("Remote CLI Client")

#ifdef REMOTE_CLI_CLIENT_SUPPORTED_COMMANDS_RECEIVE
static zb_uint8_t gs_remote_cli_client_received_commands[] =
{
    REMOTE_CLI_CLIENT_SUPPORTED_COMMANDS_RECEIVE
};
#endif

#ifdef REMOTE_CLI_CLIENT_SUPPORTED_COMMANDS_SEND
static zb_uint8_t gs_remote_cli_client_generated_commands[] =
{
    REMOTE_CLI_CLIENT_SUPPORTED_COMMANDS_SEND
};
#endif

static zb_discover_cmd_list_t gs_remote_cli_client_cmd_list =
{
#ifdef REMOTE_CLI_CLIENT_SUPPORTED_COMMANDS_RECEIVE
    sizeof(gs_remote_cli_client_received_commands),  gs_remote_cli_client_received_commands,
#else
    0,                                               NULL,
#endif
#ifdef REMOTE_CLI_CLIENT_SUPPORTED_COMMANDS_SEND
    sizeof(gs_remote_cli_client_generated_commands), gs_remote_cli_client_generated_commands
#else
    0,                                               NULL
#endif
};

// packet sent handler
void tr_remote_cli_client_cli_cmd_sent_cb(zb_uint8_t param)
{
    zb_buf_free(param);
}

// helper function for sending cli commands to a remote device
void tr_remote_cli_client_send_cli_command(zb_char_t   *command,
                                           zb_uint16_t short_addr,
                                           zb_uint8_t  endpoint)
{
    zb_uint8_t *cmd_ptr;
    zb_uint8_t param = zb_buf_get_out();
    zb_addr_u  addr;

    // prep the packet buffer
    cmd_ptr = ZB_ZCL_START_PACKET(param);
    ZB_ZCL_CONSTRUCT_SPECIFIC_COMMAND_REQ_FRAME_CONTROL_A(cmd_ptr,
                                                          ZB_ZCL_FRAME_DIRECTION_TO_SRV,
                                                          ZB_TRUE,
                                                          TR_GLOBAL_RESPONSE_POLICY);
    ZB_ZCL_CONSTRUCT_COMMAND_HEADER_EXT(cmd_ptr,
                                        ZB_ZCL_GET_SEQ_NUM(),
                                        ZB_TRUE,
                                        REMOTE_CLI_MFG_ID,
                                        TR_ZCL_CMD_CLI_COMMAND_ID);

    // put the requested string (len byte is first) into the packet
    ZB_ZCL_PACKET_PUT_DATA8(cmd_ptr, strlen((const char*)command));
    ZB_ZCL_PACKET_PUT_DATA_N(cmd_ptr, command, strlen((const char*)command));

    // send the packet
    addr.addr_short = short_addr;

    ZB_ZCL_FINISH_PACKET(param, cmd_ptr)
    ZB_ZCL_SEND_COMMAND_SHORT(param,
                              addr,
                              ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                              endpoint,
                              1,
                              ZB_AF_HA_PROFILE_ID,
                              TR_ZCL_CLUSTER_REMOTE_CLI_ID,
                              tr_remote_cli_client_cli_cmd_sent_cb);
}

// helper function for sending the enable command to a remote device
void tr_remote_cli_client_send_cli_enable(tr_remote_cli_cli_status_arg_t cli_enable,
                                          zb_uint16_t                    poll_rate,
                                          zb_uint16_t                    short_addr,
                                          zb_uint8_t                     endpoint)
{
    zb_uint8_t *cmd_ptr = 0;
    zb_uint8_t param    = zb_buf_get_out();
    zb_addr_u  addr;

    // prep the packet buffer
    cmd_ptr = ZB_ZCL_START_PACKET(param);
    ZB_ZCL_CONSTRUCT_SPECIFIC_COMMAND_REQ_FRAME_CONTROL_A(cmd_ptr,
                                                          ZB_ZCL_FRAME_DIRECTION_TO_SRV,
                                                          ZB_TRUE,
                                                          TR_GLOBAL_RESPONSE_POLICY);
    ZB_ZCL_CONSTRUCT_COMMAND_HEADER_EXT(cmd_ptr,
                                        ZB_ZCL_GET_SEQ_NUM(),
                                        ZB_TRUE,
                                        REMOTE_CLI_MFG_ID,
                                        TR_ZCL_CMD_ENABLE_REMOTE_CLI_ID);

    // put the bytes into the packet
    ZB_ZCL_PACKET_PUT_DATA8(cmd_ptr, cli_enable.value);
    ZB_ZCL_PACKET_PUT_DATA16_VAL(cmd_ptr, poll_rate);

    // send the packet
    addr.addr_short = short_addr;

    ZB_ZCL_FINISH_PACKET(param, cmd_ptr)
    ZB_ZCL_SEND_COMMAND_SHORT(param,
                              addr,
                              ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                              endpoint,
                              1,
                              ZB_AF_HA_PROFILE_ID,
                              TR_ZCL_CLUSTER_REMOTE_CLI_ID,
                              tr_remote_cli_client_cli_cmd_sent_cb);
}

// handle the remote cli cli command response
static void remote_cli_client_cli_command_response(zb_uint8_t param)
{
    zb_uint8_t *payload = zb_buf_begin(param);
    zb_uint8_t resp_len = payload[0];
    zb_uint8_t x;

    // simply print the payload
    for (x = 1 ; x <= resp_len ; x++)
    {
        tr_remote_cli_client_printf("%c", payload[x]);
    }
}

// handle remote cli commands
static zb_bool_t remote_cli_client_cluster_handler(zb_uint8_t param)
{
    zb_bool_t           processed = ZB_TRUE;
    zb_zcl_parsed_hdr_t cmd_info;
    zb_ret_t            status = RET_OK;

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);

    // verify the manufacturer id
    if (cmd_info.is_manuf_specific && (cmd_info.manuf_specific == REMOTE_CLI_MFG_ID))
    {
        if (ZB_ZCL_GENERAL_GET_CMD_LISTS_PARAM == param)
        {
            ZCL_CTX().zb_zcl_cluster_cmd_list = &gs_remote_cli_client_cmd_list;
            return ZB_TRUE;
        }

        switch (cmd_info.cmd_id)
        {
            case TR_ZCL_CMD_CLI_COMMAND_RESPONSE_ID:
                ZB_SCHEDULE_CALLBACK(remote_cli_client_cli_command_response, param);
                status = RET_OK;
                break;

            default:
                tr_remote_cli_client_printf("RX:(%s) Unknown Cmd ID: %02X, EP: %02X\n",
                                            PLUGIN_NAME,
                                            cmd_info.cmd_id,
                                            ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
                processed = ZB_FALSE;
                break;
        }
    }
    else
    {
        processed = ZB_FALSE;
    }

    if (processed)
    {
        if (cmd_info.disable_default_response && status == RET_OK)
        {
            zb_buf_free(param);
        }
        else if (status != RET_BUSY)
        {
            ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);
            ZB_ZCL_SEND_DEFAULT_RESP(param,
                                     ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).source.u.short_addr,
                                     ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                     ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).src_endpoint,
                                     ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
                                     cmd_info.profile_id,
                                     TR_ZCL_CLUSTER_REMOTE_CLI_ID,
                                     cmd_info.seq_number,
                                     cmd_info.cmd_id,
                                     status == RET_OK ? TR_ZCL_STATUS_SUCCESS : TR_ZCL_STATUS_INVALID_FIELD);
        }
    }
    return processed;
}

// Remote CLI client cluster plugin init
void tr_remote_cli_client_init(void)
{
    zb_zcl_add_cluster_handlers(TR_ZCL_CLUSTER_REMOTE_CLI_ID,
                                TR_ZCL_CLUSTER_CLIENT_ROLE,
                                (zb_zcl_cluster_check_value_t)NULL,
                                (zb_zcl_cluster_write_attr_hook_t)NULL,
                                (zb_zcl_cluster_handler_t)remote_cli_client_cluster_handler);

    tr_remote_cli_client_init_cb();
}
