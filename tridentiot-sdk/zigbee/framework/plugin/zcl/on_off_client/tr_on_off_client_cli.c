/// ****************************************************************************
/// @file tr_on_off_client_cli.c
///
/// @brief Contains CLI commands specific to the ON/OFF client cluster
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_on_off_client.h"
#include "tr_cli_argument_parser.h"
#include "tr_cli_zcl_cmds.h"

zb_int_t cli_on_off_off(zb_int_t  argc,
                        zb_char_t *argv[])
{
    if (g_cli_zcl_cmd_creation_s.buffer == 0)
    {
        g_cli_zcl_cmd_creation_s.buffer = zb_buf_get_out();
    }

    g_cli_zcl_cmd_creation_s.cmd_ptr = ZB_ZCL_START_PACKET_REQ(g_cli_zcl_cmd_creation_s.buffer)
                                       ZB_ZCL_CONSTRUCT_SPECIFIC_COMMAND_REQ_FRAME_CONTROL(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                                                                           TR_GLOBAL_RESPONSE_POLICY)
                                       ZB_ZCL_CONSTRUCT_COMMAND_HEADER_REQ(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                                                           ZB_ZCL_GET_SEQ_NUM(),
                                                                           TR_ZCL_CMD_OFF_ID);

    // save the profile and cluster for use by the send command
    g_cli_zcl_cmd_creation_s.prof_id    = ZB_AF_HA_PROFILE_ID;
    g_cli_zcl_cmd_creation_s.cluster_id = TR_ZCL_CLUSTER_ON_OFF_ID;
    g_cli_zcl_cmd_creation_s.cb         = NULL;

    tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);

    return ZB_TRUE;
}

zb_int_t cli_on_off_on(zb_int_t  argc,
                       zb_char_t *argv[])
{
    if (g_cli_zcl_cmd_creation_s.buffer == 0)
    {
        g_cli_zcl_cmd_creation_s.buffer = zb_buf_get_out();
    }

    g_cli_zcl_cmd_creation_s.cmd_ptr = ZB_ZCL_START_PACKET_REQ(g_cli_zcl_cmd_creation_s.buffer)
                                       ZB_ZCL_CONSTRUCT_SPECIFIC_COMMAND_REQ_FRAME_CONTROL(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                                                                           TR_GLOBAL_RESPONSE_POLICY)
                                       ZB_ZCL_CONSTRUCT_COMMAND_HEADER_REQ(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                                                           ZB_ZCL_GET_SEQ_NUM(),
                                                                           TR_ZCL_CMD_ON_ID);

    // save the profile and cluster for use by the send command
    g_cli_zcl_cmd_creation_s.prof_id    = ZB_AF_HA_PROFILE_ID;
    g_cli_zcl_cmd_creation_s.cluster_id = TR_ZCL_CLUSTER_ON_OFF_ID;
    g_cli_zcl_cmd_creation_s.cb         = NULL;

    tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);

    return ZB_TRUE;
}

zb_int_t cli_on_off_toggle(zb_int_t  argc,
                           zb_char_t *argv[])
{
    if (g_cli_zcl_cmd_creation_s.buffer == 0)
    {
        g_cli_zcl_cmd_creation_s.buffer = zb_buf_get_out();
    }

    g_cli_zcl_cmd_creation_s.cmd_ptr = ZB_ZCL_START_PACKET_REQ(g_cli_zcl_cmd_creation_s.buffer)
                                       ZB_ZCL_CONSTRUCT_SPECIFIC_COMMAND_REQ_FRAME_CONTROL(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                                                                           TR_GLOBAL_RESPONSE_POLICY)
                                       ZB_ZCL_CONSTRUCT_COMMAND_HEADER_REQ(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                                                           ZB_ZCL_GET_SEQ_NUM(),
                                                                           TR_ZCL_CMD_TOGGLE_ID);

    // save the profile and cluster for use by the send command
    g_cli_zcl_cmd_creation_s.prof_id    = ZB_AF_HA_PROFILE_ID;
    g_cli_zcl_cmd_creation_s.cluster_id = TR_ZCL_CLUSTER_ON_OFF_ID;
    g_cli_zcl_cmd_creation_s.cb         = NULL;

    tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);

    return ZB_TRUE;
}

zb_int_t cli_on_off_effect(zb_int_t  argc,
                           zb_char_t *argv[])
{
    zb_uint8_t effect_id      = 0;
    zb_uint8_t effect_variant = 0;
    zb_int_t   ret_val        = ZB_TRUE;
    zb_char_t  *option_argument;

    // get effect id
    if (tr_cli_get_option(argc, argv, "i:", &option_argument))
    {
        effect_id = (zb_uint8_t)tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // get effect variant
    if (tr_cli_get_option(argc, argv, "v:", &option_argument))
    {
        effect_variant = (zb_uint8_t)tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // help option
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
                                                                                               TR_GLOBAL_RESPONSE_POLICY)
                                           ZB_ZCL_CONSTRUCT_COMMAND_HEADER_REQ(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                                                               ZB_ZCL_GET_SEQ_NUM(),
                                                                               TR_ZCL_CMD_OFF_WITH_EFFECT_ID);

        ZB_ZCL_PACKET_PUT_DATA8(g_cli_zcl_cmd_creation_s.cmd_ptr, effect_id);
        ZB_ZCL_PACKET_PUT_DATA8(g_cli_zcl_cmd_creation_s.cmd_ptr, effect_variant);

        // save the profile and cluster for use by the send command
        g_cli_zcl_cmd_creation_s.prof_id    = ZB_AF_HA_PROFILE_ID;
        g_cli_zcl_cmd_creation_s.cluster_id = TR_ZCL_CLUSTER_ON_OFF_ID;
        g_cli_zcl_cmd_creation_s.cb         = NULL;

        tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);
    }
    else
    {
        tr_core_printf("usage: effect -i id -v variant\n");
    }

    return ret_val;
}

zb_int_t cli_on_off_recall(zb_int_t  argc,
                           zb_char_t *argv[])
{
    if (g_cli_zcl_cmd_creation_s.buffer == 0)
    {
        g_cli_zcl_cmd_creation_s.buffer = zb_buf_get_out();
    }

    g_cli_zcl_cmd_creation_s.cmd_ptr = ZB_ZCL_START_PACKET_REQ(g_cli_zcl_cmd_creation_s.buffer)
                                       ZB_ZCL_CONSTRUCT_SPECIFIC_COMMAND_REQ_FRAME_CONTROL(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                                                                           TR_GLOBAL_RESPONSE_POLICY)
                                       ZB_ZCL_CONSTRUCT_COMMAND_HEADER_REQ(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                                                           ZB_ZCL_GET_SEQ_NUM(),
                                                                           TR_ZCL_CMD_ON_WITH_RECALL_GLOBAL_SCENE_ID);

    // save the profile and cluster for use by the send command
    g_cli_zcl_cmd_creation_s.prof_id    = ZB_AF_HA_PROFILE_ID;
    g_cli_zcl_cmd_creation_s.cluster_id = TR_ZCL_CLUSTER_ON_OFF_ID;
    g_cli_zcl_cmd_creation_s.cb         = NULL;

    tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);

    return ZB_TRUE;
}

zb_int_t cli_on_off_timed(zb_int_t  argc,
                          zb_char_t *argv[])
{
    zb_uint8_t  control       = 0;
    zb_uint16_t on_time       = 0;
    zb_uint16_t off_wait_time = 0;
    zb_int_t    ret_val       = ZB_TRUE;
    zb_char_t   *option_argument;

    // get on/off control
    if (tr_cli_get_option(argc, argv, "c:", &option_argument))
    {
        control = (zb_uint8_t)tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // get on time
    if (tr_cli_get_option(argc, argv, "t:", &option_argument))
    {
        on_time = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // get off wait time
    if (tr_cli_get_option(argc, argv, "w:", &option_argument))
    {
        off_wait_time = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // help option
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
                                                                                               TR_GLOBAL_RESPONSE_POLICY)
                                           ZB_ZCL_CONSTRUCT_COMMAND_HEADER_REQ(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                                                               ZB_ZCL_GET_SEQ_NUM(),
                                                                               TR_ZCL_CMD_ON_WITH_TIMED_OFF_ID);

        ZB_ZCL_PACKET_PUT_DATA8(g_cli_zcl_cmd_creation_s.cmd_ptr, control);
        ZB_ZCL_PACKET_PUT_DATA16_VAL(g_cli_zcl_cmd_creation_s.cmd_ptr, on_time);
        ZB_ZCL_PACKET_PUT_DATA16_VAL(g_cli_zcl_cmd_creation_s.cmd_ptr, off_wait_time);

        // save the profile and cluster for use by the send command
        g_cli_zcl_cmd_creation_s.prof_id    = ZB_AF_HA_PROFILE_ID;
        g_cli_zcl_cmd_creation_s.cluster_id = TR_ZCL_CLUSTER_ON_OFF_ID;
        g_cli_zcl_cmd_creation_s.cb         = NULL;

        tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);
    }
    else
    {
        tr_core_printf("usage: timed -c control -t <on time> -w <off wait time>\n");
    }

    return ret_val;
}

TR_CLI_COMMAND_TABLE(zcl_on_off_c_cluster_commands) =
{
    { "off",    cli_on_off_off,    "off command"                         },
    { "on",     cli_on_off_on,     "on command"                          },
    { "toggle", cli_on_off_toggle, "toggle command"                      },
    { "effect", cli_on_off_effect, "off with effect command"             },
    { "recall", cli_on_off_recall, "on with recall global scene command" },
    { "timed",  cli_on_off_timed,  "on with timed off command"           },
    TR_CLI_COMMAND_TABLE_END
};
