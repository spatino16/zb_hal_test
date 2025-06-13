/// ****************************************************************************
/// @file tr_basic_client_cli.c
///
/// @brief Contains CLI commands specific to the BASIC client cluster
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_basic_client.h"
#include "tr_cli_argument_parser.h"
#include "tr_cli_zcl_cmds.h"

zb_int_t cli_basic_rtfd(zb_int_t  argc,
                        zb_char_t *argv[])
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
                                                                           TR_ZCL_CMD_RESET_TO_FACTORY_DEFAULTS_ID);

    // save the profile and cluster for use by the send command
    g_cli_zcl_cmd_creation_s.prof_id    = ZB_AF_HA_PROFILE_ID;
    g_cli_zcl_cmd_creation_s.cluster_id = TR_ZCL_CLUSTER_BASIC_ID;
    g_cli_zcl_cmd_creation_s.cb         = NULL;

    tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);

    return ZB_TRUE;
}

TR_CLI_COMMAND_TABLE(zcl_basic_c_cluster_commands) =
{
    { "rtfd", cli_basic_rtfd, "reset to factory defaults" },
    TR_CLI_COMMAND_TABLE_END
};
