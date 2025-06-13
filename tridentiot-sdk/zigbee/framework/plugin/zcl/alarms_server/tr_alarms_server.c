/// ****************************************************************************
/// @file tr_alarms_server.c
///
/// @brief ZCL ALARMS cluster server implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_alarms_server.h"

#define PLUGIN_NAME (zb_char_t*)("Alarms Server")

#ifdef ALARMS_SERVER_SUPPORTED_COMMANDS_RECEIVE
static zb_uint8_t gs_alarms_server_received_commands[] =
{
    ALARMS_SERVER_SUPPORTED_COMMANDS_RECEIVE
};
#endif

#ifdef ALARMS_SERVER_SUPPORTED_COMMANDS_SEND
static zb_uint8_t gs_alarms_server_generated_commands[] =
{
    ALARMS_SERVER_SUPPORTED_COMMANDS_SEND
};
#endif

static zb_discover_cmd_list_t gs_alarms_server_cmd_list =
{
#ifdef ALARMS_SERVER_SUPPORTED_COMMANDS_RECEIVE
    sizeof(gs_alarms_server_received_commands),  gs_alarms_server_received_commands,
#else
    0,                                           NULL,
#endif
#ifdef ALARMS_SERVER_SUPPORTED_COMMANDS_SEND
    sizeof(gs_alarms_server_generated_commands), gs_alarms_server_generated_commands
#else
    0,                                           NULL
#endif
};

static void alarms_server_reset_alarm(zb_uint8_t param)
{
    zb_zcl_parsed_hdr_t cmd_info;
    zb_ret_t            result = RET_OK;

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);

    // invoke application callback
    if (tr_alarms_server_reset_alarm_cb(&cmd_info) == ZB_FALSE)
    {
        // at some point we may support an alarm table. if so, reset the alarm in the table here
    }

    ZB_ZCL_PROCESS_COMMAND_FINISH(param,
                                  &cmd_info,
                                  result ==
                                  RET_OK ? TR_ZCL_STATUS_SUCCESS : (zb_zcl_get_backward_compatible_statuses_mode() ==
                                                                    ZB_ZCL_STATUSES_ZCL8_MODE) ? TR_ZCL_STATUS_FAILURE : TR_ZCL_STATUS_HARDWARE_FAILURE);
}

static void alarms_server_reset_all_alarms(zb_uint8_t param)
{
    zb_zcl_parsed_hdr_t cmd_info;
    zb_ret_t            result = RET_OK;

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);

    // invoke application callback
    if (tr_alarms_server_reset_all_alarms_cb(&cmd_info) == ZB_FALSE)
    {
        // at some point we may support an alarm table. if so, reset all alarms in the table here
    }

    ZB_ZCL_PROCESS_COMMAND_FINISH(param,
                                  &cmd_info,
                                  result ==
                                  RET_OK ? TR_ZCL_STATUS_SUCCESS : (zb_zcl_get_backward_compatible_statuses_mode() ==
                                                                    ZB_ZCL_STATUSES_ZCL8_MODE) ? TR_ZCL_STATUS_FAILURE : TR_ZCL_STATUS_HARDWARE_FAILURE);
}

static zb_bool_t alarms_server_cluster_handler(zb_uint8_t param)
{
    zb_bool_t           processed = ZB_TRUE;
    zb_zcl_parsed_hdr_t cmd_info;
    zb_ret_t            status = RET_OK;

    if (ZB_ZCL_GENERAL_GET_CMD_LISTS_PARAM == param)
    {
        ZCL_CTX().zb_zcl_cluster_cmd_list = &gs_alarms_server_cmd_list;
        return ZB_TRUE;
    }

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);

    switch (cmd_info.cmd_id)
    {
        case TR_ZCL_CMD_RESET_ALARM_ID:
            tr_alarms_server_printf("RX:(%s) Reset Alarm Cmd, EP: %02X\n",
                                    PLUGIN_NAME,
                                    ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
            ZB_SCHEDULE_CALLBACK(alarms_server_reset_alarm, param);
            status = RET_BUSY;
            break;

        case TR_ZCL_CMD_RESET_ALL_ALARMS_ID:
            tr_alarms_server_printf("RX:(%s) Reset All Alarms Cmd, EP: %02X\n",
                                    PLUGIN_NAME,
                                    ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
            ZB_SCHEDULE_CALLBACK(alarms_server_reset_all_alarms, param);
            status = RET_BUSY;
            break;

        default:
            tr_alarms_server_printf("RX:(%s) Unknown Cmd ID: %02X, EP: %02X\n",
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
                                     TR_ZCL_CLUSTER_ALARMS_ID,
                                     cmd_info.seq_number,
                                     cmd_info.cmd_id,
                                     status == RET_OK ? TR_ZCL_STATUS_SUCCESS : TR_ZCL_STATUS_INVALID_FIELD);
        }
    }
    return processed;
}

// Check the value of an attribute
static zb_ret_t alarms_server_check_value(zb_uint16_t attr_id,
                                          zb_uint8_t  endpoint,
                                          zb_uint8_t  *value)
{
    ZVUNUSED(attr_id);
    ZVUNUSED(value);
    ZVUNUSED(endpoint);

    // the only attribute is AlarmCount, it is read only and is currently not supported
    return RET_OK;
}

// process alarms server attribute write commands
static void alarms_server_write_attr_hook(zb_uint8_t  endpoint,
                                          zb_uint16_t attr_id,
                                          zb_uint8_t  *new_value,
                                          zb_uint16_t manuf_code)
{
    // the only attribute is AlarmCount, it is read only and is currently not supported

    tr_alarms_server_printf("RX:(%s) Write Attribute Cmd, ATTR: %04X, EP: %02X\n",
                            PLUGIN_NAME,
                            attr_id,
                            endpoint);
    tr_alarms_server_write_attr_cb(endpoint, attr_id, new_value, manuf_code);
}

// Alarms cluster server plugin init
void tr_alarms_server_init(void)
{
    zb_zcl_add_cluster_handlers(TR_ZCL_CLUSTER_ALARMS_ID,
                                TR_ZCL_CLUSTER_SERVER_ROLE,
                                alarms_server_check_value,
                                alarms_server_write_attr_hook,
                                alarms_server_cluster_handler);
    tr_alarms_server_init_cb();
}
