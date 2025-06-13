/// ****************************************************************************
/// @file tr_identify_server.c
///
/// @brief ZCL IDENTIFY cluster server implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_identify_server.h"
#include "tr_cli_argument_parser.h"

#define PLUGIN_NAME (zb_char_t*)("Identify Server")

#ifdef IDENTIFY_SERVER_SUPPORTED_COMMANDS_SEND
static zb_uint8_t gs_identify_server_generated_commands[] =
{
    IDENTIFY_SERVER_SUPPORTED_COMMANDS_SEND
};
#endif
#ifdef IDENTIFY_SERVER_SUPPORTED_COMMANDS_RECEIVE
static zb_uint8_t gs_identify_server_received_commands[] =
{
    IDENTIFY_SERVER_SUPPORTED_COMMANDS_RECEIVE
};
#endif

zb_discover_cmd_list_t gs_identify_server_cmd_list =
{
#ifdef IDENTIFY_SERVER_SUPPORTED_COMMANDS_SEND
    sizeof(gs_identify_server_received_commands),  gs_identify_server_received_commands,
#else
    0,                                             NULL,
#endif
#ifdef IDENTIFY_SERVER_SUPPORTED_COMMANDS_SEND
    sizeof(gs_identify_server_generated_commands), gs_identify_server_generated_commands
#else
    0,                                             NULL,
#endif
};

static zb_ret_t identify_server_check_value(zb_uint16_t attr_id,
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

// process identify server attribute write commands
static void identify_server_write_attr_hook(zb_uint8_t  endpoint,
                                            zb_uint16_t attr_id,
                                            zb_uint8_t  *new_value,
                                            zb_uint16_t manuf_code)
{
    zb_uint16_t val = 0;
    ZB_ASSIGN_UINT16(&val, new_value);

    tr_identify_server_write_attr_cb(endpoint, attr_id, new_value, manuf_code);

    /* ZCL9, 3.5.2.2.1 IdentifyTime attribute:
     * If this attribute is set to a value other than 0x0000 then the device SHALL enter its
     * identification procedure, in order to indicate to an observer which of several devices it is. It
     * is recommended that this procedure consists of flashing a light with a period of 0.5
     * seconds. The IdentifyTime attribute SHALL be decremented every second. */
    if (attr_id == TR_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID)
    {
        if (val != 0)
        {
            zb_zcl_start_identifying(endpoint, val);
        }
        else
        {
            zb_zcl_stop_identifying(endpoint);
        }
    }
}

static zb_uint8_t zb_zcl_identify_cmd_handler(zb_uint8_t  endpoint,
                                              zb_uint16_t timeout)
{
    zb_uint8_t    status = TR_ZCL_STATUS_SUCCESS;
    zb_zcl_attr_t *attr_desc;

    attr_desc = zb_zcl_get_attr_desc_a(
        endpoint,
        TR_ZCL_CLUSTER_IDENTIFY_ID,
        TR_ZCL_CLUSTER_SERVER_ROLE,
        TR_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID);

    if (!attr_desc || !attr_desc->data_p)
    {
        status = TR_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
    }
    else
    {
        zb_zcl_set_attr_val(
            endpoint,
            TR_ZCL_CLUSTER_IDENTIFY_ID,
            TR_ZCL_CLUSTER_SERVER_ROLE,
            TR_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID,
            (zb_uint8_t*)&timeout,
            ZB_FALSE);
    }

    return status;
}

static zb_bool_t identify_server_cluster_handler(zb_uint8_t param)
{
    zb_bool_t                    processed = ZB_TRUE;
    zb_uint8_t                   status    = TR_ZCL_STATUS_SUCCESS;
    zb_zcl_parsed_hdr_t          cmd_info;
    zb_zcl_attr_t                *attr_desc;
    zb_zcl_parse_status_t        parse_status = ZB_ZCL_PARSE_STATUS_FAILURE;
    zb_zcl_identify_req_t        payload;
    zb_zcl_identify_effect_req_t payload_effect;

    ZB_MEMSET(&payload, 0, sizeof(zb_zcl_identify_req_t));
    ZB_MEMSET(&payload_effect, 0, sizeof(zb_zcl_identify_effect_req_t));

    if (ZB_ZCL_GENERAL_GET_CMD_LISTS_PARAM == param)
    {
        ZCL_CTX().zb_zcl_cluster_cmd_list = &gs_identify_server_cmd_list;
        return ZB_TRUE;
    }

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);
    ZB_ASSERT(TR_ZCL_CLUSTER_IDENTIFY_ID == cmd_info.cluster_id);

    // if not client --> server then ignore it
    if (cmd_info.cmd_direction != ZB_ZCL_FRAME_DIRECTION_TO_SRV)
    {
        return ZB_FALSE;
    }

    switch (cmd_info.cmd_id)
    {
        case TR_ZCL_CMD_IDENTIFY_ID:
            ZB_ZCL_IDENTIFY_GET_IDENTIFY_REQ(&payload, param, parse_status);

            tr_identify_server_printf("RX:(%s) Identify Cmd, EP: %02X, TIMEOUT: %d\n",
                                      PLUGIN_NAME,
                                      ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
                                      payload.timeout);

            if (parse_status != ZB_ZCL_PARSE_STATUS_SUCCESS)
            {
                status = TR_ZCL_STATUS_MALFORMED_COMMAND;
            }
            else
            {
                zb_zcl_identify_cmd_handler(ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint, payload.timeout);
            }

            ZB_ZCL_PROCESS_COMMAND_FINISH(param, &cmd_info, status);
            break;

        case TR_ZCL_CMD_IDENTIFY_QUERY_ID:
            tr_identify_server_printf("RX:(%s) Identify Query Cmd, EP: %02X\n",
                                      PLUGIN_NAME,
                                      ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);

            if (zb_zcl_is_identifying(ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint))
            {
                attr_desc = zb_zcl_get_attr_desc_a(
                    ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
                    TR_ZCL_CLUSTER_IDENTIFY_ID,
                    TR_ZCL_CLUSTER_SERVER_ROLE,
                    TR_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID);

                ZB_ZCL_IDENTIFY_SEND_IDENTIFY_QUERY_RES(
                    param,
                    *(zb_uint16_t*)attr_desc->data_p,
                    ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).source.u.short_addr,
                    ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                    ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).src_endpoint,
                    ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
                    cmd_info.profile_id,
                    cmd_info.seq_number,
                    (zb_bool_t)ZB_APS_FC_GET_SECURITY(cmd_info.addr_data.common_data.fc));
            }
            else
            {
                status = TR_ZCL_STATUS_SUCCESS;
                ZB_ZCL_PROCESS_COMMAND_FINISH_NEW(param, &cmd_info, status);
            }
            break;

        case TR_ZCL_CMD_TRIGGER_EFFECT_ID:
            ZB_ZCL_IDENTIFY_GET_TRIGGER_VARIANT_REQ(&payload_effect, param, status);

            tr_identify_server_printf("RX:(%s) Identify Trigger Effect Cmd, EP: %02X, ID: %%02X, VAR: %02X\n",
                                      PLUGIN_NAME,
                                      ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
                                      payload_effect.effect_id,
                                      payload_effect.effect_variant);

            if (status != ZB_ZCL_PARSE_STATUS_SUCCESS)
            {
                ZB_ZCL_SEND_DEFAULT_RESP(
                    param,
                    ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).source.u.short_addr,
                    ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                    ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).src_endpoint,
                    ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
                    cmd_info.profile_id,
                    TR_ZCL_CLUSTER_IDENTIFY_ID,
                    cmd_info.seq_number,
                    TR_ZCL_CMD_TRIGGER_EFFECT_ID,
                    TR_ZCL_STATUS_MALFORMED_COMMAND);
            }
            else
            {
                ZB_ZCL_IDENTIFY_EFFECT_SCHEDULE_USER_APP(param,
                                                         &cmd_info,
                                                         payload_effect.effect_id,
                                                         payload_effect.effect_variant);
            }
            break;

        default:
            tr_identify_server_printf("RX:(%s) Unknown Cmd, CMD: %02X, EP: %02X\n",
                                      PLUGIN_NAME,
                                      cmd_info.cmd_id,
                                      ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
            processed = ZB_FALSE;
            break;
    }

    return processed;
}

/* Assumes param contains an endpoint number */
static void zb_zcl_identify_time_handler(zb_uint8_t param)
{
    zb_uint16_t   identify_time;
    zb_zcl_attr_t *attr_desc;

    attr_desc = zb_zcl_get_attr_desc_a(
        param,
        TR_ZCL_CLUSTER_IDENTIFY_ID,
        TR_ZCL_CLUSTER_SERVER_ROLE,
        TR_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID);

    if (attr_desc && attr_desc->data_p)
    {
        identify_time = *(zb_uint16_t*)attr_desc->data_p;

        if (identify_time)
        {
            identify_time--;
            zb_zcl_set_attr_val(
                param,
                TR_ZCL_CLUSTER_IDENTIFY_ID,
                TR_ZCL_CLUSTER_SERVER_ROLE,
                TR_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID,
                (zb_uint8_t*)&identify_time,
                false);

            ZB_SCHEDULE_ALARM(zb_zcl_identify_time_handler, param, ZB_TIME_ONE_SECOND);
            tr_identify_server_printf("identify time remaining: %d seconds\n", identify_time);
        }
    }
}

void tr_identify_server_init(void)
{
    zb_zcl_add_cluster_handlers(TR_ZCL_CLUSTER_IDENTIFY_ID,
                                TR_ZCL_CLUSTER_SERVER_ROLE,
                                identify_server_check_value,
                                identify_server_write_attr_hook,
                                identify_server_cluster_handler);

    tr_identify_server_init_cb();
}

void zb_zcl_identify_effect_invoke_user_app(zb_uint8_t param)
{
    zb_zcl_identify_effect_user_app_schedule_t *invoke_data = ZB_BUF_GET_PARAM(param, zb_zcl_identify_effect_user_app_schedule_t);
    zb_zcl_parsed_hdr_t                        cmd_info;
    zb_ret_t                                   result = RET_OK;

    ZB_MEMCPY(&cmd_info, &(invoke_data->cmd_info), sizeof(zb_zcl_parsed_hdr_t));

    result = tr_identify_server_trigger_effect_cb(invoke_data);

    ZB_ZCL_PROCESS_COMMAND_FINISH(param,
                                  &cmd_info,
                                  result == RET_OK ? TR_ZCL_STATUS_SUCCESS :
                                  (zb_zcl_get_backward_compatible_statuses_mode() == ZB_ZCL_STATUSES_ZCL8_MODE) ?
                                  TR_ZCL_STATUS_FAILURE : TR_ZCL_STATUS_HARDWARE_FAILURE);

}

zb_uint8_t zb_zcl_start_identifying(zb_uint8_t  endpoint,
                                    zb_uint16_t timeout)
{
    zb_uint8_t    status = TR_ZCL_STATUS_SUCCESS;
    zb_zcl_attr_t *attr_desc;
    zb_uint16_t   old_timeout = 0;

    attr_desc = zb_zcl_get_attr_desc_a(
        endpoint,
        TR_ZCL_CLUSTER_IDENTIFY_ID,
        TR_ZCL_CLUSTER_SERVER_ROLE,
        TR_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID);

    if (!attr_desc || !attr_desc->data_p)
    {
        status = TR_ZCL_STATUS_UNSUPPORTED_ATTRIBUTE;
    }
    else
    {
        old_timeout = *(zb_uint16_t*)attr_desc->data_p;
        ZB_ZCL_SET_DIRECTLY_ATTR_VAL16(attr_desc, timeout);

        if (timeout)
        {
            if (!old_timeout)
            {
                tr_identify_server_printf("Identify Start, EP: %02X, TIMEOUT: %d\n",
                                          endpoint,
                                          timeout);
                ZB_SCHEDULE_ALARM_CANCEL(zb_zcl_identify_time_handler, endpoint);
                ZB_SCHEDULE_ALARM(zb_zcl_identify_time_handler, endpoint, ZB_TIME_ONE_SECOND);
                ZB_SCHEDULE_CALLBACK2(tr_identify_server_identify_start_cb, endpoint, timeout);
            }
        }
    }

    return status;
}

zb_uint8_t zb_zcl_is_identifying(zb_uint8_t endpoint)
{
    zb_uint8_t    result = ZB_FALSE;
    zb_zcl_attr_t *attr_desc;

    if (endpoint)
    {
        attr_desc = zb_zcl_get_attr_desc_a(
            endpoint,
            TR_ZCL_CLUSTER_IDENTIFY_ID,
            TR_ZCL_CLUSTER_SERVER_ROLE,
            TR_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID);

        if (attr_desc)
        {
            result = (0 != (*(zb_uint16_t*)attr_desc->data_p));
        }
        else
        {
            tr_identify_server_printf("Attribute not found\n");
        }
    }

    return result;
}

void zb_zcl_stop_identifying(zb_uint8_t endpoint)
{
    zb_zcl_attr_t *attr_desc;

    tr_identify_server_printf("Identify Stop, EP: %02X\n",
                              endpoint);

    attr_desc = zb_zcl_get_attr_desc_a(
        endpoint,
        TR_ZCL_CLUSTER_IDENTIFY_ID,
        TR_ZCL_CLUSTER_SERVER_ROLE,
        TR_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID);

    if (attr_desc)
    {
        ZB_SCHEDULE_ALARM_CANCEL(zb_zcl_identify_time_handler, endpoint);
        ZB_ZCL_SET_DIRECTLY_ATTR_VAL16(attr_desc, 0);
        ZB_SCHEDULE_CALLBACK(tr_identify_server_identify_stop_cb, endpoint);
    }
    else
    {
        tr_identify_server_printf("Attribute not found\n");
    }
}
