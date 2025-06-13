/// ****************************************************************************
/// @file tr_alarms_client.c
///
/// @brief ZCL ALARMS cluster client implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_alarms_client.h"

#define PLUGIN_NAME (zb_char_t*)("Alarms Client")

#ifdef ALARMS_CLIENT_SUPPORTED_COMMANDS_RECEIVE
static zb_uint8_t gs_alarms_client_received_commands[] =
{
    ALARMS_CLIENT_SUPPORTED_COMMANDS_RECEIVE
};
#endif

#ifdef ALARMS_CLIENT_SUPPORTED_COMMANDS_SEND
static zb_uint8_t gs_alarms_client_generated_commands[] =
{
    ALARMS_CLIENT_SUPPORTED_COMMANDS_SEND
};
#endif

static zb_discover_cmd_list_t gs_alarms_client_cmd_list =
{
#ifdef ALARMS_CLIENT_SUPPORTED_COMMANDS_RECEIVE
    sizeof(gs_alarms_client_received_commands),  gs_alarms_client_received_commands,
#else
    0,                                           NULL,
#endif
#ifdef ALARMS_CLIENT_SUPPORTED_COMMANDS_SEND
    sizeof(gs_alarms_client_generated_commands), gs_alarms_client_generated_commands
#else
    0,                                           NULL
#endif
};

static void alarms_client_alarm(zb_uint8_t param)
{
    zb_zcl_parsed_hdr_t cmd_info;
    zb_ret_t            result = RET_OK;

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);

    // invoke application callback
    tr_alarms_client_alarm_cb(&cmd_info);

    ZB_ZCL_PROCESS_COMMAND_FINISH(param,
                                  &cmd_info,
                                  result ==
                                  RET_OK ? TR_ZCL_STATUS_SUCCESS : (zb_zcl_get_backward_compatible_statuses_mode() ==
                                                                    ZB_ZCL_STATUSES_ZCL8_MODE) ? TR_ZCL_STATUS_FAILURE : TR_ZCL_STATUS_HARDWARE_FAILURE);
}

static zb_bool_t alarms_client_cluster_handler(zb_uint8_t param)
{
    zb_bool_t           processed = ZB_TRUE;
    zb_zcl_parsed_hdr_t cmd_info;
    zb_ret_t            status = RET_OK;

    if (ZB_ZCL_GENERAL_GET_CMD_LISTS_PARAM == param)
    {
        ZCL_CTX().zb_zcl_cluster_cmd_list = &gs_alarms_client_cmd_list;
        return ZB_TRUE;
    }

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);

    switch (cmd_info.cmd_id)
    {
        case TR_ZCL_CMD_ALARM_ID:
            tr_alarms_client_printf("RX:(%s) Alarm Cmd, EP: %02X\n",
                                    PLUGIN_NAME,
                                    ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
            ZB_SCHEDULE_CALLBACK(alarms_client_alarm, param);
            status = RET_BUSY;
            break;

        default:
            tr_alarms_client_printf("RX:(%s) Unknown Cmd ID: %02X, EP: %02X\n",
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

void tr_alarms_client_init(void)
{
    zb_zcl_add_cluster_handlers(TR_ZCL_CLUSTER_ALARMS_ID,
                                TR_ZCL_CLUSTER_CLIENT_ROLE,
                                (zb_zcl_cluster_check_value_t)NULL,
                                (zb_zcl_cluster_write_attr_hook_t)NULL,
                                alarms_client_cluster_handler);

    tr_alarms_client_init_cb();
}
