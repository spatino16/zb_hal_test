/// ****************************************************************************
/// @file tr_door_lock_server.c
///
/// @brief ZCL DOOR LOCK cluster server implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_door_lock_server.h"

#define PLUGIN_NAME (char*)("Door Lock Server")

#ifdef DOOR_LOCK_SERVER_SUPPORTED_COMMANDS_RECEIVE
static uint8_t gs_door_lock_server_received_commands[] =
{
    DOOR_LOCK_SERVER_SUPPORTED_COMMANDS_RECEIVE
};
#endif

#ifdef DOOR_LOCK_SERVER_SUPPORTED_COMMANDS_SEND
static uint8_t gs_door_lock_server_generated_commands[] =
{
    DOOR_LOCK_SERVER_SUPPORTED_COMMANDS_SEND
};
#endif

static zb_discover_cmd_list_t gs_door_lock_server_cmd_list =
{
#ifdef DOOR_LOCK_SERVER_SUPPORTED_COMMANDS_RECEIVE
    sizeof(gs_door_lock_server_received_commands),  gs_door_lock_server_received_commands,
#else
    0,                                              NULL,
#endif
#ifdef DOOR_LOCK_SERVER_SUPPORTED_COMMANDS_SEND
    sizeof(gs_door_lock_server_generated_commands), gs_door_lock_server_generated_commands
#else
    0,                                              NULL
#endif
};

static void send_op_event_notification_cb(zb_bufid_t bufid)
{
    zb_buf_free(bufid);
}

// check for APS security enabled
static zb_bool_t check_lock_security_level(zb_uint8_t endpoint)
{
    zb_zcl_attr_t *attr_desc;

    // see if the cluster requires APS encryption
    attr_desc = zb_zcl_get_attr_desc_a(
        endpoint,
        TR_ZCL_CLUSTER_DOOR_LOCK_ID,
        TR_ZCL_CLUSTER_SERVER_ROLE,
        TR_ZCL_ATTR_DOOR_LOCK_ZIGBEE_SECURITY_LEVEL_ID);

    if (attr_desc != NULL)
    {
        if ((*(zb_uint8_t*)attr_desc->data_p & 0x01) == 1)
        {
            // APS encryption is required for all commands
            return ZB_TRUE;
        }
    }
    return ZB_FALSE;
}

// API for getting the auto-relock time
zb_uint32_t tr_get_auto_relock_time_seconds(zb_uint8_t endpoint)
{
    zb_zcl_attr_t *attr_desc;

    // see if the cluster requires APS encryption
    attr_desc = zb_zcl_get_attr_desc_a(
        endpoint,
        TR_ZCL_CLUSTER_DOOR_LOCK_ID,
        TR_ZCL_CLUSTER_SERVER_ROLE,
        TR_ZCL_ATTR_DOOR_LOCK_AUTO_RELOCK_TIME_ID);

    if (attr_desc != NULL)
    {
        return *(zb_uint32_t*)attr_desc->data_p;
    }
    return 0;
}

// API for sending a door lock operation event notification
void tr_door_lock_server_send_op_event_notification(zb_uint8_t  endpoint,
                                                    zb_uint8_t  source,
                                                    zb_uint8_t  code,
                                                    zb_uint16_t user,
                                                    zb_uint8_t  *pin,
                                                    zb_uint8_t  mask_bit)
{
    zb_uint16_t   mask = 0;
    zb_zcl_attr_t *attr_desc;
    zb_bufid_t    param = zb_buf_get_out();

    // get the proper operation event mask attribute
    attr_desc = zb_zcl_get_attr_desc_a(
        endpoint,
        TR_ZCL_CLUSTER_DOOR_LOCK_ID,
        TR_ZCL_CLUSTER_SERVER_ROLE,
        TR_ZCL_ATTR_DOOR_LOCK_KEYPAD_OPERATION_EVENT_MASK_ID + source);

    // get the mask value
    if (attr_desc != NULL)
    {
        mask = *(zb_uint16_t*)attr_desc->data_p;
    }

    // if not masked (non-zero) send the operation event notification
    if (mask & mask_bit)
    {
        // create the operation event notification and send it via bindings
        tr_door_lock_server_printf("Sending op event notification: ep %d, src %d, code 0x%x\n", endpoint, source, code);
        zb_uint8_t *ptr = ZB_ZCL_START_PACKET(param);
        zb_addr_u  addr;
        addr.addr_short = 0; // force send via a binding

        ZB_ZCL_CONSTRUCT_SPECIFIC_COMMAND_RES_FRAME_CONTROL(ptr);
        ZB_ZCL_CONSTRUCT_COMMAND_HEADER(ptr, ZB_ZCL_GET_SEQ_NUM(), TR_ZCL_CMD_OPERATION_EVENT_NOTIFICATION_ID);
        ZB_ZCL_PACKET_PUT_DATA8(ptr, source);
        ZB_ZCL_PACKET_PUT_DATA8(ptr, code);
        ZB_ZCL_PACKET_PUT_DATA16_VAL(ptr, user);

        // if pin is null, put in a single 0 for the length byte
        if (pin == NULL)
        {
            ZB_ZCL_PACKET_PUT_DATA8(ptr, 0);
        }
        else
        {
            // put in pin, len byte first
            ZB_ZCL_PACKET_PUT_DATA_N(ptr, pin, pin[0] + 1);

        }
        ZB_ZCL_PACKET_PUT_DATA32_VAL(ptr, 0xFFFFFFFF); // zigbee time
        ZB_ZCL_FINISH_N_SEND_PACKET_NEW(param,
                                        ptr,
                                        addr,
                                        ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT,
                                        0,
                                        endpoint,
                                        ZB_AF_HA_PROFILE_ID,
                                        TR_ZCL_CLUSTER_DOOR_LOCK_ID,
                                        send_op_event_notification_cb,
                                        check_lock_security_level(endpoint),
                                        ZB_FALSE,
                                        0);
    }
}

// API used to update the lock state after an actuation completes
void tr_door_lock_server_update_lock_state(zb_uint8_t  endpoint,
                                           zb_uint8_t  lock_state,
                                           zb_uint8_t  source,
                                           zb_uint16_t user,
                                           zb_uint8_t  *pin_code)
{
    // set the lock state attribute
    zb_zcl_set_attr_val(
        endpoint,
        TR_ZCL_CLUSTER_DOOR_LOCK_ID,
        TR_ZCL_CLUSTER_SERVER_ROLE,
        TR_ZCL_ATTR_DOOR_LOCK_LOCK_STATE_ID,
        &lock_state,
        ZB_FALSE);

    if (lock_state == TR_ZCL_DOOR_LOCK_STATE_LOCKED)
    {
        // use lock event source indeterminate to mean auto relock
        if (source == TR_ZCL_DOOR_LOCK_EVENT_SOURCE_INDETERMINATE)
        {
            tr_door_lock_server_send_op_event_notification(endpoint,
                                                           TR_ZCL_DOOR_LOCK_EVENT_SOURCE_MANUAL,
                                                           TR_ZCL_DOOR_LOCK_OPERATION_EVENT_CODE_AUTO_LOCK,
                                                           user,
                                                           pin_code,
                                                           (1 << 1));
        }
        else
        {
            tr_door_lock_server_send_op_event_notification(endpoint,
                                                           source,
                                                           TR_ZCL_DOOR_LOCK_OPERATION_EVENT_CODE_LOCK,
                                                           user,
                                                           pin_code,
                                                           (1 << 1));
        }
    }
    else if (lock_state == TR_ZCL_DOOR_LOCK_STATE_UNLOCKED)
    {
        tr_door_lock_server_send_op_event_notification(endpoint,
                                                       source,
                                                       TR_ZCL_DOOR_LOCK_OPERATION_EVENT_CODE_UNLOCK,
                                                       user,
                                                       pin_code,
                                                       (1 << 2));
    }
}

// Check the value of an attribute
static zb_ret_t door_lock_server_check_value(zb_uint16_t attr_id,
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

// process door_lock server attribute write commands
static void door_lock_server_write_attr_hook(zb_uint8_t  endpoint,
                                             zb_uint16_t attr_id,
                                             zb_uint8_t  *new_value,
                                             zb_uint16_t manuf_code)
{
    tr_door_lock_server_printf("RX:(%s) Write Attribute Cmd, ATTR: %04X, EP: %02X\n",
                               PLUGIN_NAME,
                               attr_id,
                               endpoint);
    tr_door_lock_server_write_attr_cb(endpoint, attr_id, new_value, manuf_code);
}

/**************************************************************************/
/*                         command handlers                               */
/**************************************************************************/

// function used for sending door lock responses
static void send_door_lock_response(zb_bufid_t  buffer,
                                    zb_uint16_t addr,
                                    zb_uint8_t  dst_addr_mode,
                                    zb_uint8_t  dst_ep,
                                    zb_uint8_t  ep,
                                    zb_uint16_t prfl_id,
                                    zb_uint8_t  seq_num,
                                    zb_uint8_t  cmd,
                                    zb_uint8_t  payload_len,
                                    zb_uint8_t  *payload,
                                    zb_uint8_t  aps_secured)
{
    zb_uint8_t *ptr = ZB_ZCL_START_PACKET(buffer);

    ZB_ZCL_CONSTRUCT_SPECIFIC_COMMAND_RES_FRAME_CONTROL(ptr);
    ZB_ZCL_CONSTRUCT_COMMAND_HEADER(ptr, seq_num, cmd);
    ZB_ZCL_PACKET_PUT_DATA_N(ptr, payload, payload_len);
    ZB_ZCL_FINISH_N_SEND_PACKET_NEW(buffer,
                                    ptr,
                                    addr,
                                    dst_addr_mode,
                                    dst_ep,
                                    ep,
                                    prfl_id,
                                    TR_ZCL_CLUSTER_DOOR_LOCK_ID,
                                    NULL,
                                    aps_secured,
                                    ZB_FALSE,
                                    0);
}

static void door_lock_server_lock_door(zb_uint8_t          param,
                                       zb_zcl_parsed_hdr_t *cmd_info)
{
    zb_uint8_t *pin_code = zb_buf_begin(param);
    zb_uint8_t status    = TR_ZCL_STATUS_SUCCESS;

    // invoke application callback
    if (tr_door_lock_server_lock_door_cb(cmd_info, pin_code) == ZB_FALSE)
    {
        // send the lock door response
        send_door_lock_response(param,
                                cmd_info->addr_data.common_data.source.u.short_addr,
                                ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                cmd_info->addr_data.common_data.src_endpoint,
                                cmd_info->addr_data.common_data.dst_endpoint,
                                cmd_info->profile_id,
                                cmd_info->seq_number,
                                TR_ZCL_CMD_LOCK_DOOR_RESPONSE_ID,
                                1,
                                &status,
                                (zb_bool_t)ZB_APS_FC_GET_SECURITY(cmd_info->addr_data.common_data.fc));
    }
}

static void door_lock_server_unlock_door(zb_uint8_t          param,
                                         zb_zcl_parsed_hdr_t *cmd_info)
{
    zb_uint8_t *pin_code = zb_buf_begin(param);
    zb_uint8_t status    = TR_ZCL_STATUS_SUCCESS;

    // invoke application callback
    if (tr_door_lock_server_unlock_door_cb(cmd_info, pin_code) == ZB_FALSE)
    {
        // send the unlock door response
        send_door_lock_response(param,
                                cmd_info->addr_data.common_data.source.u.short_addr,
                                ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                cmd_info->addr_data.common_data.src_endpoint,
                                cmd_info->addr_data.common_data.dst_endpoint,
                                cmd_info->profile_id,
                                cmd_info->seq_number,
                                TR_ZCL_CMD_UNLOCK_DOOR_RESPONSE_ID,
                                1,
                                &status,
                                (zb_bool_t)ZB_APS_FC_GET_SECURITY(cmd_info->addr_data.common_data.fc));
    }
}

// set a pin code
static void door_lock_server_set_pin(zb_uint8_t          param,
                                     zb_zcl_parsed_hdr_t *cmd_info)
{
    tr_door_lock_set_pin_or_id_status_t status;
    tr_door_lock_server_pin_code_arg_t  *args = zb_buf_begin(param);

    // invoke application callback
    if (tr_door_lock_server_set_pin_cb(cmd_info,
                                       args->user_id,
                                       args->user_status,
                                       args->user_type,
                                       args->pin_code) == ZB_FALSE)
    {
        // put this pin code into the table
        status = tr_door_lock_server_add_user(args->user_id,
                                              (tr_door_lock_user_status_t)args->user_status,
                                              (tr_door_lock_user_type_t)args->user_type,
                                              (zb_char_t*)args->pin_code);
        // send the set pin response
        send_door_lock_response(param,
                                cmd_info->addr_data.common_data.source.u.short_addr,
                                ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                cmd_info->addr_data.common_data.src_endpoint,
                                cmd_info->addr_data.common_data.dst_endpoint,
                                cmd_info->profile_id,
                                cmd_info->seq_number,
                                TR_ZCL_CMD_SET_PIN_RESPONSE_ID,
                                1,
                                (zb_uint8_t*)&status,
                                (zb_bool_t)ZB_APS_FC_GET_SECURITY(cmd_info->addr_data.common_data.fc));
    }
}

// get a pin code
static zb_ret_t door_lock_server_get_pin(zb_uint8_t          param,
                                         zb_zcl_parsed_hdr_t *cmd_info)
{
    zb_ret_t                           ret_val = RET_BUSY;
    zb_bool_t                          status;
    tr_door_lock_server_pin_code_arg_t user_info;
    zb_uint16_t                        user_id = *(zb_uint16_t*)zb_buf_begin(param);

    // invoke application callback
    if (tr_door_lock_server_get_pin_cb(cmd_info, user_id) == ZB_FALSE)
    {
        // get the pin code from the table
        status = tr_door_lock_server_get_user(user_id, &user_info);

        if (status)
        {
            // send the get pin response
            send_door_lock_response(param,
                                    cmd_info->addr_data.common_data.source.u.short_addr,
                                    ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                    cmd_info->addr_data.common_data.src_endpoint,
                                    cmd_info->addr_data.common_data.dst_endpoint,
                                    cmd_info->profile_id,
                                    cmd_info->seq_number,
                                    TR_ZCL_CMD_GET_PIN_RESPONSE_ID,
                                    4 + 1 + user_info.pin_code[0],
                                    (zb_uint8_t*)&user_info,
                                    (zb_bool_t)ZB_APS_FC_GET_SECURITY(cmd_info->addr_data.common_data.fc));
        }
        else
        {
            // send default response with error
            ret_val = RET_ERROR;
        }
    }
    return ret_val;
}

// clear a pin code
static zb_ret_t door_lock_server_clear_pin(zb_uint8_t          param,
                                           zb_zcl_parsed_hdr_t *cmd_info)
{
    zb_ret_t    ret_val = RET_BUSY;
    zb_bool_t   status;
    zb_uint16_t user_id = *(zb_uint16_t*)zb_buf_begin(param);

    // invoke application callback
    if (tr_door_lock_server_clear_pin_cb(cmd_info, user_id) == ZB_FALSE)
    {
        // delete this pin code from the table
        status = tr_door_lock_server_delete_user(user_id);

        // strange, but the ZCL8 says to send a response with 0 for success, 1 for failure
        // no further definitions or information provided
        if (status)
        {
            status = 0;
        }
        else
        {
            status = 1;
        }

        // send the set pin response
        send_door_lock_response(param,
                                cmd_info->addr_data.common_data.source.u.short_addr,
                                ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                cmd_info->addr_data.common_data.src_endpoint,
                                cmd_info->addr_data.common_data.dst_endpoint,
                                cmd_info->profile_id,
                                cmd_info->seq_number,
                                TR_ZCL_CMD_CLEAR_PIN_RESPONSE_ID,
                                1,
                                (zb_uint8_t*)&status,
                                (zb_bool_t)ZB_APS_FC_GET_SECURITY(cmd_info->addr_data.common_data.fc));
    }
    return ret_val;
}

// clear all pin codes
static zb_ret_t door_lock_server_clear_all_pins(zb_uint8_t          param,
                                                zb_zcl_parsed_hdr_t *cmd_info)
{
    zb_ret_t  ret_val = RET_BUSY;
    zb_bool_t status  = 0;

    // invoke application callback
    if (tr_door_lock_server_clear_all_pins_cb(cmd_info) == ZB_FALSE)
    {
        // delete all pin codes from the table
        tr_door_lock_server_delete_all_pin_users();

        // send the set pin response
        send_door_lock_response(param,
                                cmd_info->addr_data.common_data.source.u.short_addr,
                                ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                cmd_info->addr_data.common_data.src_endpoint,
                                cmd_info->addr_data.common_data.dst_endpoint,
                                cmd_info->profile_id,
                                cmd_info->seq_number,
                                TR_ZCL_CMD_CLEAR_ALL_PINS_RESPONSE_ID,
                                1,
                                (zb_uint8_t*)&status,
                                (zb_bool_t)ZB_APS_FC_GET_SECURITY(cmd_info->addr_data.common_data.fc));
    }
    return ret_val;
}

// set user status
static zb_ret_t door_lock_server_set_user_status(zb_uint8_t          param,
                                                 zb_zcl_parsed_hdr_t *cmd_info)
{
    zb_ret_t                              ret_val = RET_BUSY;
    tr_door_lock_server_user_status_arg_t *args   = zb_buf_begin(param);
    zb_bool_t                             status  = 0;

    // invoke application callback
    if (tr_door_lock_server_set_user_status_cb(cmd_info, args->user_id, args->user_status) == ZB_FALSE)
    {
        // set the user status
        status = tr_door_lock_server_set_user_status(args->user_id, args->user_status);

        // strange, but the ZCL8 says to send a response with 0 for success, 1 for failure
        // no further definitions or information provided
        if (status)
        {
            status = 0;
        }
        else
        {
            status = 1;
        }

        // send the set pin response
        send_door_lock_response(param,
                                cmd_info->addr_data.common_data.source.u.short_addr,
                                ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                cmd_info->addr_data.common_data.src_endpoint,
                                cmd_info->addr_data.common_data.dst_endpoint,
                                cmd_info->profile_id,
                                cmd_info->seq_number,
                                TR_ZCL_CMD_SET_USER_STATUS_RESPONSE_ID,
                                1,
                                (zb_uint8_t*)&status,
                                (zb_bool_t)ZB_APS_FC_GET_SECURITY(cmd_info->addr_data.common_data.fc));
    }
    return ret_val;
}

// get user status
static zb_ret_t door_lock_server_get_user_status(zb_uint8_t          param,
                                                 zb_zcl_parsed_hdr_t *cmd_info)
{
    zb_ret_t                              ret_val = RET_BUSY;
    zb_bool_t                             status;
    tr_door_lock_server_user_status_arg_t user_info;
    zb_uint16_t                           user_id = *(zb_uint16_t*)zb_buf_begin(param);

    // invoke application callback
    if (tr_door_lock_server_get_user_status_cb(cmd_info, user_id) == ZB_FALSE)
    {
        // get the user status from the table
        status = tr_door_lock_server_get_user_status(user_id, &user_info);

        if (status)
        {
            // send the get pin response
            send_door_lock_response(param,
                                    cmd_info->addr_data.common_data.source.u.short_addr,
                                    ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                    cmd_info->addr_data.common_data.src_endpoint,
                                    cmd_info->addr_data.common_data.dst_endpoint,
                                    cmd_info->profile_id,
                                    cmd_info->seq_number,
                                    TR_ZCL_CMD_GET_USER_STATUS_RESPONSE_ID,
                                    3,
                                    (zb_uint8_t*)&user_info,
                                    (zb_bool_t)ZB_APS_FC_GET_SECURITY(cmd_info->addr_data.common_data.fc));
        }
        else
        {
            // send default response with error
            ret_val = RET_ERROR;
        }
    }
    return ret_val;
}

// set user type
static zb_ret_t door_lock_server_set_user_type(zb_uint8_t          param,
                                               zb_zcl_parsed_hdr_t *cmd_info)
{
    zb_ret_t                            ret_val = RET_BUSY;
    tr_door_lock_server_user_type_arg_t *args   = zb_buf_begin(param);
    zb_bool_t                           status  = 0;

    // invoke application callback
    if (tr_door_lock_server_set_user_type_cb(cmd_info, args->user_id, args->user_type) == ZB_FALSE)
    {
        // set the user type
        status = tr_door_lock_server_set_user_type(args->user_id, args->user_type);

        // strange, but the ZCL8 says to send a response with 0 for success, 1 for failure
        // no further definitions or information provided
        if (status)
        {
            status = 0;
        }
        else
        {
            status = 1;
        }

        // send the set pin response
        send_door_lock_response(param,
                                cmd_info->addr_data.common_data.source.u.short_addr,
                                ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                cmd_info->addr_data.common_data.src_endpoint,
                                cmd_info->addr_data.common_data.dst_endpoint,
                                cmd_info->profile_id,
                                cmd_info->seq_number,
                                TR_ZCL_CMD_SET_USER_TYPE_RESPONSE_ID,
                                1,
                                (zb_uint8_t*)&status,
                                (zb_bool_t)ZB_APS_FC_GET_SECURITY(cmd_info->addr_data.common_data.fc));
    }
    return ret_val;
}

// get user type
static zb_ret_t door_lock_server_get_user_type(zb_uint8_t          param,
                                               zb_zcl_parsed_hdr_t *cmd_info)
{
    zb_ret_t                            ret_val = RET_BUSY;
    zb_bool_t                           status;
    tr_door_lock_server_user_type_arg_t user_info;
    zb_uint16_t                         user_id = *(zb_uint16_t*)zb_buf_begin(param);

    // invoke application callback
    if (tr_door_lock_server_get_user_type_cb(cmd_info, user_id) == ZB_FALSE)
    {
        // get the user type from the table
        status = tr_door_lock_server_get_user_type(user_id, &user_info);

        if (status)
        {
            // send the get pin response
            send_door_lock_response(param,
                                    cmd_info->addr_data.common_data.source.u.short_addr,
                                    ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                    cmd_info->addr_data.common_data.src_endpoint,
                                    cmd_info->addr_data.common_data.dst_endpoint,
                                    cmd_info->profile_id,
                                    cmd_info->seq_number,
                                    TR_ZCL_CMD_GET_USER_TYPE_RESPONSE_ID,
                                    3,
                                    (zb_uint8_t*)&user_info,
                                    (zb_bool_t)ZB_APS_FC_GET_SECURITY(cmd_info->addr_data.common_data.fc));
        }
        else
        {
            // send default response with error
            ret_val = RET_ERROR;
        }
    }
    return ret_val;
}

static zb_bool_t door_lock_server_cluster_handler(zb_uint8_t param)
{
    zb_bool_t           processed = ZB_TRUE;
    zb_zcl_parsed_hdr_t cmd_info;
    zb_ret_t            status = RET_OK;

    if (ZB_ZCL_GENERAL_GET_CMD_LISTS_PARAM == param)
    {
        ZCL_CTX().zb_zcl_cluster_cmd_list = &gs_door_lock_server_cmd_list;
        return ZB_TRUE;
    }

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);

    // is APS security required?
    if (check_lock_security_level(cmd_info.addr_data.common_data.dst_endpoint))
    {
        // APS security is required, was this a secure frame?
        if (ZB_APS_FC_IS_SECURE(cmd_info.addr_data.common_data.fc) == ZB_FALSE)
        {
            // the command was not sent with APS security, send default response with error?
            // status = RET_ERROR;

            // just quietly drop the command?
            zb_buf_free(param);
            return ZB_TRUE;
        }
    }

    switch (cmd_info.cmd_id)
    {
        case TR_ZCL_CMD_LOCK_DOOR_ID:
            tr_door_lock_server_printf("RX:(%s) Lock Door Cmd, EP: %02X\n",
                                       PLUGIN_NAME,
                                       ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
            door_lock_server_lock_door(param, &cmd_info);
            status = RET_BUSY;
            break;

        case TR_ZCL_CMD_UNLOCK_DOOR_ID:
            tr_door_lock_server_printf("RX:(%s) Unlock Door Cmd, EP: %02X\n",
                                       PLUGIN_NAME,
                                       ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
            door_lock_server_unlock_door(param, &cmd_info);
            status = RET_BUSY;
            break;

        case TR_ZCL_CMD_SET_PIN_ID:
            tr_door_lock_server_printf("RX:(%s) Set PIN Cmd, EP: %02X\n",
                                       PLUGIN_NAME,
                                       ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
            door_lock_server_set_pin(param, &cmd_info);
            status = RET_BUSY;
            break;

        case TR_ZCL_CMD_GET_PIN_ID:
            tr_door_lock_server_printf("RX:(%s) Get PIN Cmd, EP: %02X\n",
                                       PLUGIN_NAME,
                                       ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
            status = door_lock_server_get_pin(param, &cmd_info);
            break;

        case TR_ZCL_CMD_CLEAR_PIN_ID:
            tr_door_lock_server_printf("RX:(%s) Clear PIN Cmd, EP: %02X\n",
                                       PLUGIN_NAME,
                                       ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
            status = door_lock_server_clear_pin(param, &cmd_info);
            break;

        case TR_ZCL_CMD_CLEAR_ALL_PINS_ID:
            tr_door_lock_server_printf("RX:(%s) Clear All PINs Cmd, EP: %02X\n",
                                       PLUGIN_NAME,
                                       ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
            door_lock_server_clear_all_pins(param, &cmd_info);
            status = RET_BUSY;
            break;

        case TR_ZCL_CMD_SET_USER_STATUS_ID:
            tr_door_lock_server_printf("RX:(%s) Set User Status Cmd, EP: %02X\n",
                                       PLUGIN_NAME,
                                       ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
            status = door_lock_server_set_user_status(param, &cmd_info);
            break;

        case TR_ZCL_CMD_GET_USER_STATUS_ID:
            tr_door_lock_server_printf("RX:(%s) Get User Status Cmd, EP: %02X\n",
                                       PLUGIN_NAME,
                                       ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
            status = door_lock_server_get_user_status(param, &cmd_info);
            break;

        case TR_ZCL_CMD_SET_USER_TYPE_ID:
            tr_door_lock_server_printf("RX:(%s) Set User Type Cmd, EP: %02X\n",
                                       PLUGIN_NAME,
                                       ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
            status = door_lock_server_set_user_type(param, &cmd_info);
            break;

        case TR_ZCL_CMD_GET_USER_TYPE_ID:
            tr_door_lock_server_printf("RX:(%s) Get User Type Cmd, EP: %02X\n",
                                       PLUGIN_NAME,
                                       ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
            status = door_lock_server_get_user_type(param, &cmd_info);
            break;

        default:
            tr_door_lock_server_printf("RX:(%s) Unknown Cmd ID: %02X, EP: %02X\n",
                                       PLUGIN_NAME,
                                       cmd_info.cmd_id,
                                       ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
            processed = ZB_FALSE;
            break;
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
                                     TR_ZCL_CLUSTER_DOOR_LOCK_ID,
                                     cmd_info.seq_number,
                                     cmd_info.cmd_id,
                                     status == RET_OK ? TR_ZCL_STATUS_SUCCESS : TR_ZCL_STATUS_INVALID_FIELD);
        }
    }
    return processed;
}

// Door lock server cluster plugin init
void tr_door_lock_server_init(void)
{
    zb_zcl_add_cluster_handlers(TR_ZCL_CLUSTER_DOOR_LOCK_ID,
                                TR_ZCL_CLUSTER_SERVER_ROLE,
                                door_lock_server_check_value,
                                door_lock_server_write_attr_hook,
                                (zb_zcl_cluster_handler_t)door_lock_server_cluster_handler);

    // init the door lock server users
    tr_door_lock_server_users_init();

    // init the door lock server schedules
    // TODO: LCD 2/6/25 Add schedules

    // init the door lock server logging
    // TODO: LCD 2/6/25 Add logging

    // init the door lock server
    tr_door_lock_server_init_cb();
}
