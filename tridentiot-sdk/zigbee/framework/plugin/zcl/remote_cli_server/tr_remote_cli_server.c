/// ****************************************************************************
/// @file tr_remote_cli_server.c
///
/// @brief ZCL REMOTE CLI cluster server implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_remote_cli_server.h"
#include "tr_circular_buffer.h"

#define PLUGIN_NAME (zb_char_t*)("Remote CLI Server")

// declare and initialize the buffer used to store data to be sent out
TR_CIRCULAR_BUFFER(rcli_send_buffer, TR_REMOTE_CLI_SEND_BUFFER_SIZE)

static zb_zcl_attr_t *rcli_status_attr_desc;
static zb_bool_t   rcli_tmp_enable = ZB_FALSE;
static zb_uint8_t  endpoint        = 1;
static zb_uint16_t short_addr      = 0xFFFF;

void remote_cli_server_send_response(zb_uint8_t param);
extern void tr_cli_char_received(char data);

#ifdef REMOTE_CLI_SERVER_SUPPORTED_COMMANDS_RECEIVE
static zb_uint8_t gs_remote_cli_server_received_commands[] =
{
    REMOTE_CLI_SERVER_SUPPORTED_COMMANDS_RECEIVE
};
#endif

#ifdef REMOTE_CLI_SERVER_SUPPORTED_COMMANDS_SEND
static zb_uint8_t gs_remote_cli_server_generated_commands[] =
{
    REMOTE_CLI_SERVER_SUPPORTED_COMMANDS_SEND
};
#endif

static zb_discover_cmd_list_t gs_remote_cli_server_cmd_list =
{
#ifdef REMOTE_CLI_SERVER_SUPPORTED_COMMANDS_RECEIVE
    sizeof(gs_remote_cli_server_received_commands),  gs_remote_cli_server_received_commands,
#else
    0,                                               NULL,
#endif
#ifdef REMOTE_CLI_SERVER_SUPPORTED_COMMANDS_SEND
    sizeof(gs_remote_cli_server_generated_commands), gs_remote_cli_server_generated_commands
#else
    0,                                               NULL
#endif
};

// callback after sending a packet of cli response data
// see if there is more to send and schedule it if so
void remote_cli_server_send_response_cb(zb_uint8_t param)
{
    if (TR_CIRCULAR_BUFFER_IS_EMPTY(rcli_send_buffer))
    {
        zb_buf_free(param);
    }
    else
    {
        ZB_SCHEDULE_ALARM(remote_cli_server_send_response, param, 0);
    }
}

// send a packet of cli response data
void remote_cli_server_send_response(zb_uint8_t param)
{
    zb_uint8_t *cmd_ptr;
    zb_addr_u  addr;
    zb_bool_t  is_manuf_specific = ZB_TRUE;

    if (param == 0)
    {
        param = zb_buf_get_out();
    }

    addr.addr_short = short_addr;

    cmd_ptr = ZB_ZCL_START_PACKET(param);
    ZB_ZCL_CONSTRUCT_SPECIFIC_COMMAND_REQ_FRAME_CONTROL_A(cmd_ptr,
                                                          ZB_ZCL_FRAME_DIRECTION_TO_CLI,
                                                          is_manuf_specific,
                                                          TR_GLOBAL_RESPONSE_POLICY);
    ZB_ZCL_CONSTRUCT_COMMAND_HEADER_EXT(cmd_ptr,
                                        ZB_ZCL_GET_SEQ_NUM(),
                                        is_manuf_specific,
                                        REMOTE_CLI_MFG_ID,
                                        TR_ZCL_CMD_CLI_COMMAND_RESPONSE_ID);

    // dequeue the rcli response data into the send buffer
    // send either the defined payload size or as much as is left in the buffered output data
    ZB_ZCL_PACKET_PUT_DATA8(cmd_ptr,
                            (rcli_send_buffer.count >=
                             TR_REMOTE_CLI_SEND_PACKET_PAYLOAD_SIZE ? TR_REMOTE_CLI_SEND_PACKET_PAYLOAD_SIZE : rcli_send_buffer.count));
    TR_CIRCULAR_BUFFER_DEQUEUE(rcli_send_buffer, (char*)cmd_ptr, *(zb_uint8_t*)(cmd_ptr - 1));
    cmd_ptr += *(zb_uint8_t*)(cmd_ptr - 1);

    ZB_ZCL_FINISH_PACKET(param, cmd_ptr)
    ZB_ZCL_SEND_COMMAND_SHORT(param,
                              addr,
                              ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                              endpoint,
                              1,
                              ZB_AF_HA_PROFILE_ID,
                              TR_ZCL_CLUSTER_REMOTE_CLI_ID,
                              remote_cli_server_send_response_cb);
}

// get output headed for the local stdout and put it in the remote cli send buffer
void remote_cli_server_add_output(const zb_uint8_t *buf,
                                  zb_uint8_t       len)
{
    static zb_bool_t                stripping_color = ZB_FALSE;
    tr_remote_cli_cli_status_attr_t rcli_status;

    // check and see if rcli is enabled
    rcli_status.value = ZB_ZCL_GET_ATTRIBUTE_VAL_8(rcli_status_attr_desc);

    if (rcli_status.bits.remote_enable || rcli_tmp_enable)
    {
        // we need to remove the color settings from the output before sending it
        // all color strings start with '\033' and end with 'm'
        if ((!stripping_color) && (buf[0] == COLOR_START_CHAR))
        {
            // this is the start of a color string, discard it
            stripping_color = ZB_TRUE;
            return;
        }
        else if (stripping_color && (buf[0] == COLOR_END_CHAR))
        {
            // this is the end of a color string, discard it
            stripping_color = ZB_FALSE;
            return;
        }
        else if (stripping_color)
        {
            // throw away everything while stripping color
            return;
        }

        // copy bytes to the send buffer
        TR_CIRCULAR_BUFFER_ENQUEUE(rcli_send_buffer, buf, len);

        // kick off the packet send event if not already active
        ZB_SCHEDULE_ALARM_CANCEL(remote_cli_server_send_response, ZB_ALARM_ANY_PARAM);
        ZB_SCHEDULE_ALARM(remote_cli_server_send_response, 0, ZB_MILLISECONDS_TO_BEACON_INTERVAL(TR_REMOTE_CLI_SEND_PACKET_DELAY_MS));
    }
}

// Check the value of an attribute
static zb_ret_t remote_cli_server_check_value(zb_uint16_t attr_id,
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

// process remote cli server attribute write commands
static void remote_cli_server_write_attr_hook(zb_uint8_t  endpoint,
                                              zb_uint16_t attr_id,
                                              zb_uint8_t  *new_value,
                                              zb_uint16_t manuf_code)
{
    tr_remote_cli_server_printf("RX:(%s) Write Attribute Cmd, ATTR: %04X, EP: %02X\n",
                                PLUGIN_NAME,
                                attr_id,
                                endpoint);
    tr_remote_cli_server_write_attr_cb(endpoint, attr_id, new_value, manuf_code);
}

// handler for the remote cli cli command command
static void remote_cli_server_cli_command(zb_uint8_t param)
{
    zb_zcl_parsed_hdr_t cmd_info;
    zb_uint8_t          *payload = zb_buf_begin(param);
    zb_uint8_t          cmd_len  = payload[0];
    zb_uint8_t          x;

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);

    // save the sender's short address and endpoint for the response
    short_addr = cmd_info.addr_data.common_data.source.u.short_addr;
    endpoint   = cmd_info.addr_data.common_data.src_endpoint;

    rcli_tmp_enable = ZB_TRUE;

    // send the characters to the cli
    for (x = 1 ; x <= cmd_len ; x++)
    {
        tr_cli_char_received(payload[x]);
    }
    tr_cli_char_received('\n');
    rcli_tmp_enable = ZB_FALSE;
}

// handler for the remote cli enable command
static void remote_cli_server_enable(zb_uint8_t param)
{
    zb_zcl_parsed_hdr_t cmd_info;
    // zb_ret_t            result   = RET_OK;
    zb_uint8_t *payload = zb_buf_begin(param);

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);

    short_addr = cmd_info.addr_data.common_data.source.u.short_addr;
    endpoint   = cmd_info.addr_data.common_data.src_endpoint;

    // set the enabled attribute according to the arg passed
    zb_zcl_set_attr_val_manuf(cmd_info.addr_data.common_data.dst_endpoint,
                              TR_ZCL_CLUSTER_REMOTE_CLI_ID,
                              TR_ZCL_CLUSTER_SERVER_ROLE,
                              TR_ZCL_ATTR_REMOTE_CLI_CLI_STATUS_ID,
                              REMOTE_CLI_MFG_ID,
                              (zb_uint8_t*)&payload[0],
                              ZB_FALSE);

    // TODO: LCD 11/18/24 - handle the poll rate argument and set the poll rate accordingly. This makes sleepy devices responsive.
    // set the polling rate according to the arg passed
}

// handler for the remote cli server commands received
static zb_bool_t remote_cli_server_cluster_handler(zb_uint8_t param)
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
            ZCL_CTX().zb_zcl_cluster_cmd_list = &gs_remote_cli_server_cmd_list;
            return ZB_TRUE;
        }

        switch (cmd_info.cmd_id)
        {
            case TR_ZCL_CMD_CLI_COMMAND_ID:
                tr_remote_cli_server_printf("RX:(%s) CLI command: %02X\n",
                                            PLUGIN_NAME,
                                            ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
                ZB_SCHEDULE_CALLBACK(remote_cli_server_cli_command, param);
                status = RET_OK;
                break;

            case TR_ZCL_CMD_ENABLE_REMOTE_CLI_ID:
                tr_remote_cli_server_printf("RX:(%s) CLI enable: %02X\n",
                                            PLUGIN_NAME,
                                            ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
                ZB_SCHEDULE_CALLBACK(remote_cli_server_enable, param);
                status = RET_OK;
                break;

            default:
                tr_remote_cli_server_printf("RX:(%s) Unknown Cmd ID: %02X, EP: %02X\n",
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

// Remote CLI server cluster plugin init
void tr_remote_cli_server_init(void)
{
    zb_zcl_add_cluster_handlers(TR_ZCL_CLUSTER_REMOTE_CLI_ID,
                                TR_ZCL_CLUSTER_SERVER_ROLE,
                                remote_cli_server_check_value,
                                remote_cli_server_write_attr_hook,
                                remote_cli_server_cluster_handler);

    // setup the attribute descriptor for the rcli status attribute
    rcli_status_attr_desc = zb_zcl_get_attr_desc_manuf_a(1,
                                                         TR_ZCL_CLUSTER_REMOTE_CLI_ID,
                                                         TR_ZCL_CLUSTER_SERVER_ROLE,
                                                         TR_ZCL_ATTR_REMOTE_CLI_CLI_STATUS_ID,
                                                         REMOTE_CLI_MFG_ID);

    ZB_ASSERT(rcli_status_attr_desc);
    tr_remote_cli_server_init_cb();
}
