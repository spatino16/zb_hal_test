/// ****************************************************************************
/// @file tr_alarms_client_cli.c
///
/// @brief Contains CLI commands specific to the ALARMS client cluster
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_alarms_client.h"
#include "tr_cli_argument_parser.h"
#include "tr_cli_zcl_cmds.h"

zb_int_t cli_alarms_reset_alarm(zb_int_t  argc,
                                zb_char_t *argv[])
{
    zb_bool_t   ret_val       = ZB_TRUE;
    zb_uint8_t  alarm_code    = 0;
    zb_uint16_t alarm_cluster = 0;
    zb_char_t   *option_argument;

    // get the alarm code
    if (tr_cli_get_option(argc, argv, "a:", &option_argument))
    {
        alarm_code = tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // get the alarm cluster
    if (tr_cli_get_option(argc, argv, "c:", &option_argument))
    {
        alarm_cluster = tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // check for the -h option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        if (g_cli_zcl_cmd_creation_s.buffer == 0)
        {
            g_cli_zcl_cmd_creation_s.buffer = zb_buf_get_out();
        }

        g_cli_zcl_cmd_creation_s.cmd_ptr = ZB_ZCL_START_PACKET_REQ(g_cli_zcl_cmd_creation_s.buffer)
                                           ZB_ZCL_CONSTRUCT_SPECIFIC_COMMAND_REQ_FRAME_CONTROL(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                                                                               tr_global_default_response_policy)
                                           ZB_ZCL_CONSTRUCT_COMMAND_HEADER_REQ(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                                                               ZB_ZCL_GET_SEQ_NUM(),
                                                                               TR_ZCL_CMD_RESET_ALARM_ID);

        ZB_ZCL_PACKET_PUT_DATA8(g_cli_zcl_cmd_creation_s.cmd_ptr, alarm_code);
        ZB_ZCL_PACKET_PUT_DATA16_VAL(g_cli_zcl_cmd_creation_s.cmd_ptr, alarm_cluster);

        // save the profile and cluster for use by the send command
        g_cli_zcl_cmd_creation_s.prof_id    = ZB_AF_HA_PROFILE_ID;
        g_cli_zcl_cmd_creation_s.cluster_id = TR_ZCL_CLUSTER_ALARMS_ID;
        g_cli_zcl_cmd_creation_s.cb         = NULL;

        tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);
    }
    else
    {
        tr_core_printf("usage: reset -a alarm code -c alarm cluster\n");
    }

    return ZB_TRUE;
}

zb_int_t cli_alarms_reset_all_alarms(zb_int_t  argc,
                                     zb_char_t *argv[])
{
    zb_bool_t ret_val = ZB_TRUE;
    zb_char_t *option_argument;

    // check for the -h option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        if (g_cli_zcl_cmd_creation_s.buffer == 0)
        {
            g_cli_zcl_cmd_creation_s.buffer = zb_buf_get_out();
        }

        g_cli_zcl_cmd_creation_s.cmd_ptr = ZB_ZCL_START_PACKET_REQ(g_cli_zcl_cmd_creation_s.buffer)
                                           ZB_ZCL_CONSTRUCT_SPECIFIC_COMMAND_REQ_FRAME_CONTROL(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                                                                               tr_global_default_response_policy)
                                           ZB_ZCL_CONSTRUCT_COMMAND_HEADER_REQ(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                                                               ZB_ZCL_GET_SEQ_NUM(),
                                                                               TR_ZCL_CMD_RESET_ALL_ALARMS_ID);

        // save the profile and cluster for use by the send command
        g_cli_zcl_cmd_creation_s.prof_id    = ZB_AF_HA_PROFILE_ID;
        g_cli_zcl_cmd_creation_s.cluster_id = TR_ZCL_CLUSTER_ALARMS_ID;
        g_cli_zcl_cmd_creation_s.cb         = NULL;

        tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);
    }
    else
    {
        tr_core_printf("usage: rst_all\n");
    }

    return ZB_TRUE;
}

TR_CLI_COMMAND_TABLE(zcl_alarms_c_cluster_commands) =
{
    { "reset",   cli_alarms_reset_alarm,      "reset a single alarm" },
    { "rst_all", cli_alarms_reset_all_alarms, "reset all alarms"     },
    TR_CLI_COMMAND_TABLE_END
};
