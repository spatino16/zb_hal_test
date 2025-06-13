/// ****************************************************************************
/// @file tr_cli_zcl_global_cmds.c
///
/// @brief CLI for ZCL global commands
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include <string.h>
#include "tr_af.h"
#include "tr_cli_argument_parser.h"
#include "tr_cli_zcl_cmds.h"

tr_cli_zcl_cmd_creation_s g_cli_zcl_cmd_creation_s;

zb_int_t cli_set_default_response_policy(zb_int_t  argc,
                                         zb_char_t *argv[])
{
    zb_uint8_t ret_val = ZB_FALSE;
    zb_char_t  *option_argument;

    if (tr_cli_get_option(argc, argv, "e", &option_argument))
    {
        tr_global_default_response_policy = ZB_ZCL_ENABLE_DEFAULT_RESPONSE;
        ret_val                           = ZB_TRUE;
    }

    if (tr_cli_get_option(argc, argv, "d", &option_argument))
    {
        tr_global_default_response_policy = ZB_ZCL_DISABLE_DEFAULT_RESPONSE;
        ret_val                           = ZB_TRUE;
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ZB_TRUE == ret_val)
    {
        if (ZB_ZCL_DISABLE_DEFAULT_RESPONSE == tr_global_default_response_policy)
        {
            tr_core_printf("Default response policy set to disabled\n");
        }
        else
        {
            tr_core_printf("Default response policy set to enabled\n");
        }
    }
    else
    {
        tr_core_printf("usage: set_default_resp -e enable or -d disable\n");
    }
    return 0;
}

// build the zcl read attribute command
zb_int_t cli_attr_read(zb_int_t  argc,
                       zb_char_t *argv[])
{
    zb_int_t           i;
    zb_int_t           ret_val = ZB_TRUE;
    zb_char_t          *option_argument;
    tr_standard_args_s arg_struct;

    // parse the args passed by the user
    tr_cli_parse_standard_args(&arg_struct, argc, argv);

    // check mandatory args
    if (arg_struct.num_cluster_ids != 1)
    {
        tr_core_printf("A single cluster id is required\n");
        ret_val = ZB_FALSE;
    }

    if (arg_struct.num_attr_ids == 0)
    {
        tr_core_printf("At least one attribute id is required\n");
        ret_val = ZB_FALSE;
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    // args are good, build the read command
    if (ret_val)
    {
        // get a buffer and build the command
        if (g_cli_zcl_cmd_creation_s.buffer == 0)
        {
            g_cli_zcl_cmd_creation_s.buffer = zb_buf_get_out();
        }

        ZB_ZCL_GENERAL_INIT_READ_ATTR_REQ_A((g_cli_zcl_cmd_creation_s.buffer),
                                            g_cli_zcl_cmd_creation_s.cmd_ptr,
                                            arg_struct.cmd_dir,
                                            tr_global_default_response_policy);

        for (i = 0 ; i < arg_struct.num_attr_ids ; i++)
        {
            ZB_ZCL_GENERAL_ADD_ID_READ_ATTR_REQ(g_cli_zcl_cmd_creation_s.cmd_ptr, (arg_struct.attr_ids[i]));

        }

        // save the profile and cluster for use by the send command
        g_cli_zcl_cmd_creation_s.prof_id    = ZB_AF_HA_PROFILE_ID;
        g_cli_zcl_cmd_creation_s.cluster_id = arg_struct.cluster_ids[0];
        g_cli_zcl_cmd_creation_s.cb         = NULL;

        // print the command buffer
        tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);
    }
    else
    {
        // at least 1 arg is wrong, print usage
        tr_core_printf(
            "usage: read -c cluster_id -a attr_id [-a attr_id] [-m mfg_id <default non-mfg specific>] [-d dir <default 0 (c->s>)] [-e aps encrypt]\n");
    }
    return ret_val;
}

// build the zcl write attribute command
zb_int_t cli_attr_write(zb_int_t  argc,
                        zb_char_t *argv[])
{
    zb_int_t           ret_val = ZB_TRUE;
    zb_char_t          *option_argument;
    tr_standard_args_s arg_struct;

    // parse the args passed by the user
    tr_cli_parse_standard_args(&arg_struct, argc, argv);

    // check mandatory args
    if (arg_struct.num_cluster_ids != 1)
    {
        tr_core_printf("A single cluster id is required\n");
        ret_val = ZB_FALSE;
    }

    if (arg_struct.num_attr_ids != 1)
    {
        tr_core_printf("A single attribute id is required\n");
        ret_val = ZB_FALSE;
    }

    if (arg_struct.data_type == 0xff)
    {
        // no data type was provided, see if we can figure it out from our own clusters
        zb_zcl_attr_t *attr_info;
        attr_info =  zb_zcl_get_attr_desc_manuf_a(
            ZCL_CTX().device_ctx->ep_desc_list[0]->ep_id,
            arg_struct.cluster_ids[0],
            TR_ZCL_CLUSTER_SERVER_ROLE,
            arg_struct.attr_ids[0],
            arg_struct.mfg_id);

        if (attr_info != NULL)
        {
            arg_struct.data_type = attr_info->type;
            // tr_core_printf("Deduced attr type is 0x%2.2x\n", arg_struct.data_type);
        }
        else
        {
            tr_core_printf("No valid data type entered\n");
            ret_val = ZB_FALSE;
        }
    }

    if (!arg_struct.value_entered)
    {
        tr_core_printf("A value is required\n");
        ret_val = ZB_FALSE;
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    // args are good, build the write command
    if (ret_val)
    {
        zb_uint64_t write_value;

        // get a buffer and build the command
        if (g_cli_zcl_cmd_creation_s.buffer == 0)
        {
            g_cli_zcl_cmd_creation_s.buffer = zb_buf_get_out();
        }

        ZB_ZCL_GENERAL_INIT_WRITE_ATTR_REQ_A(g_cli_zcl_cmd_creation_s.buffer,
                                             g_cli_zcl_cmd_creation_s.cmd_ptr,
                                             arg_struct.cmd_dir,
                                             tr_global_default_response_policy)

        // convert the value string to the proper type of data
        switch (arg_struct.data_type)
        {
            case TR_ZCL_OCTET_STRING_ATTR_TYPE:
            case TR_ZCL_CHAR_STRING_ATTR_TYPE:
            {
                zb_uint8_t len = strlen(arg_struct.value);
                zb_uint8_t str[33];

                // make sure entered string fits
                if (len > sizeof(str) - 1)
                {
                    tr_core_printf("\nValue string must be 32 characters or less\n");
                }
                else
                {
                    // for octet string, char string put a 1 byte length at the beginning
                    memcpy(&str[1], arg_struct.value, len);
                    str[0]       = len;
                    str[len + 1] = 0;
                    ZB_ZCL_GENERAL_ADD_VALUE_WRITE_ATTR_REQ(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                                            arg_struct.attr_ids[0],
                                                            arg_struct.data_type,
                                                            str)
                }
                break;
            }

            case TR_ZCL_LONG_OCTET_STRING_ATTR_TYPE:
            case TR_ZCL_LONG_CHAR_STRING_ATTR_TYPE:
                // for long octet string and long char string put a 2 byte length at the beginning
                tr_core_printf("\nLong octet (0x43) and long char (0x44) string types not supported\n");
                break;

            default:
                // just convert the value string to a number
                write_value = tr_dec_or_hex_string_to_int(arg_struct.value);
                ZB_ZCL_GENERAL_ADD_VALUE_WRITE_ATTR_REQ(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                                        arg_struct.attr_ids[0],
                                                        arg_struct.data_type,
                                                        (zb_uint8_t*)&write_value)
                break;
        }

        // save the profile and cluster for use by the send command
        g_cli_zcl_cmd_creation_s.prof_id    = ZB_AF_HA_PROFILE_ID;
        g_cli_zcl_cmd_creation_s.cluster_id = arg_struct.cluster_ids[0];
        g_cli_zcl_cmd_creation_s.cb         = NULL;

        // print the command buffer
        tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);
    }
    else
    {
        // at least 1 arg is missing, print usage
        tr_core_printf(
            "usage: write -c cluster_id -a attr_id -t data_type -v value [-m mfg_id <default non-mfg specific>] [-d dir <default 0 (c->s)>] [-e aps encrypt]\n");
    }
    return ret_val;
}

// Global commands table
TR_CLI_COMMAND_TABLE(zcl_global_commands) =
{
    { "read",             cli_attr_read,                   "read an attribute"                                                },
    { "write",            cli_attr_write,                  "write an attribute"                                               },
    { "set_default_resp", cli_set_default_response_policy, "Enable or disable the global default response"                    },
    TR_CLI_COMMAND_TABLE_END
};
