/// ****************************************************************************
/// @file tr_identify_client_cli.c
///
/// @brief Contains CLI commands specific to the IDENTIFY client cluster
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_identify_client.h"
#include "tr_cli_argument_parser.h"
#include "tr_cli_zcl_cmds.h"

zb_int_t cli_identify_command(zb_int_t  argc,
                              zb_char_t *argv[])
{
    zb_uint16_t timeout = 0;
    zb_int_t    ret_val = ZB_TRUE;
    zb_char_t   *option_argument;

    // get timeout value
    if (tr_cli_get_option(argc, argv, "t:", &option_argument))
    {
        timeout = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
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
                                                                                               tr_global_default_response_policy)
                                           ZB_ZCL_CONSTRUCT_COMMAND_HEADER_REQ(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                                                               ZB_ZCL_GET_SEQ_NUM(),
                                                                               TR_ZCL_CMD_IDENTIFY_ID);

        ZB_ZCL_PACKET_PUT_DATA16_VAL(g_cli_zcl_cmd_creation_s.cmd_ptr, timeout);

        // save the profile and cluster for use by the send command
        g_cli_zcl_cmd_creation_s.prof_id    = ZB_AF_HA_PROFILE_ID;
        g_cli_zcl_cmd_creation_s.cluster_id = TR_ZCL_CLUSTER_IDENTIFY_ID;
        g_cli_zcl_cmd_creation_s.cb         = NULL;

        tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);
    }
    else
    {
        tr_core_printf("usage: identify -t timeout\n");
    }

    return ret_val;
}

zb_int_t cli_query_command(zb_int_t  argc,
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
                                                                           TR_ZCL_CMD_IDENTIFY_QUERY_ID);

    // save the profile and cluster for use by the send command
    g_cli_zcl_cmd_creation_s.prof_id    = ZB_AF_HA_PROFILE_ID;
    g_cli_zcl_cmd_creation_s.cluster_id = TR_ZCL_CLUSTER_IDENTIFY_ID;
    g_cli_zcl_cmd_creation_s.cb         = NULL;

    tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);

    return ZB_TRUE;
}

zb_int_t cli_trigger_command(zb_int_t  argc,
                             zb_char_t *argv[])
{
    zb_uint8_t effect_id      = 0;
    zb_uint8_t effect_variant = 0;
    zb_int_t   ret_val        = ZB_TRUE;
    zb_char_t  *option_argument;

    // get effect id and variant
    if (tr_cli_get_option(argc, argv, "i:", &option_argument))
    {
        effect_id = (zb_uint8_t)tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

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
                                                                                               tr_global_default_response_policy)
                                           ZB_ZCL_CONSTRUCT_COMMAND_HEADER_REQ(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                                                               ZB_ZCL_GET_SEQ_NUM(),
                                                                               TR_ZCL_CMD_TRIGGER_EFFECT_ID);

        ZB_ZCL_PACKET_PUT_DATA8(g_cli_zcl_cmd_creation_s.cmd_ptr, effect_id);
        ZB_ZCL_PACKET_PUT_DATA8(g_cli_zcl_cmd_creation_s.cmd_ptr, effect_variant);

        // save the profile and cluster for use by the send command
        g_cli_zcl_cmd_creation_s.prof_id    = ZB_AF_HA_PROFILE_ID;
        g_cli_zcl_cmd_creation_s.cluster_id = TR_ZCL_CLUSTER_IDENTIFY_ID;
        g_cli_zcl_cmd_creation_s.cb         = NULL;

        tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);
    }
    else
    {
        tr_core_printf("usage: trigger -i id -v variant\n");
    }

    return ret_val;
}

zb_int_t cli_identify_on(zb_int_t  argc,
                         zb_char_t *argv[])
{
    zb_uint16_t timeout = 0;
    zb_int_t    ret_val = ZB_TRUE;
    zb_char_t   *option_argument;

    // get timeout value
    if (tr_cli_get_option(argc, argv, "t:", &option_argument))
    {
        timeout = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
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

        ZB_ZCL_GENERAL_INIT_WRITE_ATTR_REQ_A(g_cli_zcl_cmd_creation_s.buffer,
                                             g_cli_zcl_cmd_creation_s.cmd_ptr,
                                             ZB_ZCL_FRAME_DIRECTION_TO_SRV,
                                             tr_global_default_response_policy)

        ZB_ZCL_GENERAL_ADD_VALUE_WRITE_ATTR_REQ(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                                TR_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID,
                                                TR_ZCL_INT16U_ATTR_TYPE,
                                                (zb_uint8_t*)&timeout)

        // save the profile and cluster for use by the send command
        g_cli_zcl_cmd_creation_s.prof_id    = ZB_AF_HA_PROFILE_ID;
        g_cli_zcl_cmd_creation_s.cluster_id = TR_ZCL_CLUSTER_IDENTIFY_ID;
        g_cli_zcl_cmd_creation_s.cb         = NULL;

        tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);
    }
    else
    {
        tr_core_printf("usage: on -t timeout\n");
    }

    return ret_val;
}

zb_int_t cli_identify_off(zb_int_t  argc,
                          zb_char_t *argv[])
{
    zb_uint16_t timeout = 0;

    if (g_cli_zcl_cmd_creation_s.buffer == 0)
    {
        g_cli_zcl_cmd_creation_s.buffer = zb_buf_get_out();
    }

    ZB_ZCL_GENERAL_INIT_WRITE_ATTR_REQ_A(g_cli_zcl_cmd_creation_s.buffer,
                                         g_cli_zcl_cmd_creation_s.cmd_ptr,
                                         ZB_ZCL_FRAME_DIRECTION_TO_SRV,
                                         tr_global_default_response_policy)

    ZB_ZCL_GENERAL_ADD_VALUE_WRITE_ATTR_REQ(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                            TR_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID,
                                            TR_ZCL_INT16U_ATTR_TYPE,
                                            (zb_uint8_t*)&timeout)

    // save the profile and cluster for use by the send command
    g_cli_zcl_cmd_creation_s.prof_id    = ZB_AF_HA_PROFILE_ID;
    g_cli_zcl_cmd_creation_s.cluster_id = TR_ZCL_CLUSTER_IDENTIFY_ID;
    g_cli_zcl_cmd_creation_s.cb         = NULL;

    tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);

    return ZB_TRUE;
}

TR_CLI_COMMAND_TABLE(zcl_identify_c_cluster_commands) =
{
    { "identify", cli_identify_command, "identify command"                      },
    { "query",    cli_query_command,    "identify query command"                },
    { "trigger",  cli_trigger_command,  "trigger effect command"                },
    { "on",       cli_identify_on,      "write identify timeout attribute"      },
    { "off",      cli_identify_off,     "write identify timeout attribute to 0" },
    // ez mode?
    TR_CLI_COMMAND_TABLE_END
};
