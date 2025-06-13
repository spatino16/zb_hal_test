/// ****************************************************************************
/// @file tr_groups_client_cli.c
///
/// @brief Contains CLI commands specific to the GROUPS client cluster
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include <unistd.h>
#include <stdlib.h>
#include "tr_cli_argument_parser.h"
#include "tr_cli_zcl_cmds.h"
#include "tr_groups_client.h"
#include "tr_groups_server_cli.h"

zb_int_t cli_groups_add(zb_int_t  argc,
                        zb_char_t *argv[])
{
    zb_uint16_t group_id       = 0;
    zb_uint8_t  group_name_len = 0;
    zb_char_t   group_name[18] = { 0 };
    zb_int_t    ret_val        = ZB_TRUE;
    zb_char_t   *option_argument;

    // get group id
    if (tr_cli_get_option(argc, argv, "i:", &option_argument))
    {
        group_id = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // get optional group name
    if (tr_cli_get_option(argc, argv, "n:", &option_argument))
    {
        // NOTE: max group name size is 16
        for (group_name_len = 0 ; group_name_len < 16 ; group_name_len++)
        {
            if (group_name_len < strlen(option_argument))
            {
                group_name[group_name_len + 1] = option_argument[group_name_len];
            }
            else
            {
                break;
            }
        }
        group_name[0] = group_name_len;
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
                                                                               TR_ZCL_CMD_ADD_GROUP_ID);

        ZB_ZCL_PACKET_PUT_DATA16_VAL(g_cli_zcl_cmd_creation_s.cmd_ptr, group_id);

        // populate group name if it exists
        if (group_name[0] > 0)
        {
            ZB_ZCL_PACKET_PUT_DATA_N(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                     group_name,
                                     group_name[0] + 1);
        }
        else
        {
            ZB_ZCL_PACKET_PUT_DATA8(g_cli_zcl_cmd_creation_s.cmd_ptr, ZB_ZCL_NULL_STRING);
        }

        // save the profile and cluster for use by the send command
        g_cli_zcl_cmd_creation_s.prof_id    = ZB_AF_HA_PROFILE_ID;
        g_cli_zcl_cmd_creation_s.cluster_id = TR_ZCL_CLUSTER_GROUPS_ID;
        g_cli_zcl_cmd_creation_s.cb         = NULL;

        // print the command buffer
        tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);
    }
    else
    {
        tr_core_printf("usage: add -i group-id [-n \"group-name\"]\n");
    }

    return ret_val;
}

zb_int_t cli_groups_view(zb_int_t  argc,
                         zb_char_t *argv[])
{
    zb_uint16_t group_id = 0;
    zb_int_t    ret_val  = ZB_TRUE;
    zb_char_t   *option_argument;

    // get group id
    if (tr_cli_get_option(argc, argv, "i:", &option_argument))
    {
        group_id = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
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
                                                                               TR_ZCL_CMD_VIEW_GROUP_ID);

        ZB_ZCL_PACKET_PUT_DATA16_VAL(g_cli_zcl_cmd_creation_s.cmd_ptr, group_id);

        // save the profile and cluster for use by the send command
        g_cli_zcl_cmd_creation_s.prof_id    = ZB_AF_HA_PROFILE_ID;
        g_cli_zcl_cmd_creation_s.cluster_id = TR_ZCL_CLUSTER_GROUPS_ID;
        g_cli_zcl_cmd_creation_s.cb         = NULL;

        // print the command buffer
        tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);
    }
    else
    {
        tr_core_printf("usage: view -i group-id\n");
    }

    return ret_val;
}

zb_int_t cli_groups_get_membership(zb_int_t  argc,
                                   zb_char_t *argv[])
{
    zb_int_t    opt             = 0;
    zb_uint8_t  group_count     = 0;
    zb_uint8_t  arg_group_count = 0;
    zb_uint16_t group_list[32]  = { 0 };
    zb_int_t    ret_val         = ZB_TRUE;
    zb_char_t   *option_argument;

    // get group count
    if (tr_cli_get_option(argc, argv, "c:", &option_argument))
    {
        group_count = (zb_uint8_t)tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    if (group_count != 0)
    {
        // get list of group ids
        while ((opt = getopt(argc, argv, "i:")) != -1)
        {
            if (opt == 'i')
            {
                group_list[arg_group_count++] = (zb_uint16_t)tr_dec_or_hex_string_to_int(optarg);
            }
        }
    }

    if (group_count != arg_group_count)
    {
        tr_core_printf("Incorrect number of group ids!\n");
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
                                                                               TR_ZCL_CMD_GET_GROUP_MEMBERSHIP_ID);

        ZB_ZCL_PACKET_PUT_DATA8(g_cli_zcl_cmd_creation_s.cmd_ptr, group_count);

        // populate group ids if they exist
        for (zb_uint8_t i = 0 ; i < group_count ; i++)
        {
            ZB_ZCL_PACKET_PUT_DATA16_VAL(g_cli_zcl_cmd_creation_s.cmd_ptr, group_list[i]);
        }

        // save the profile and cluster for use by the send command
        g_cli_zcl_cmd_creation_s.prof_id    = ZB_AF_HA_PROFILE_ID;
        g_cli_zcl_cmd_creation_s.cluster_id = TR_ZCL_CLUSTER_GROUPS_ID;
        g_cli_zcl_cmd_creation_s.cb         = NULL;

        // print the command buffer
        tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);
    }
    else
    {
        tr_core_printf("usage: get -c group-count -i group-id [-i group-id]\n");
    }

    return ret_val;
}

zb_int_t cli_groups_remove(zb_int_t  argc,
                           zb_char_t *argv[])
{
    zb_uint16_t group_id = 0;
    zb_int_t    ret_val  = ZB_TRUE;
    zb_char_t   *option_argument;

    // get group id
    if (tr_cli_get_option(argc, argv, "i:", &option_argument))
    {
        group_id = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
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
                                                                               TR_ZCL_CMD_REMOVE_GROUP_ID);

        ZB_ZCL_PACKET_PUT_DATA16_VAL(g_cli_zcl_cmd_creation_s.cmd_ptr, group_id);

        // save the profile and cluster for use by the send command
        g_cli_zcl_cmd_creation_s.prof_id    = ZB_AF_HA_PROFILE_ID;
        g_cli_zcl_cmd_creation_s.cluster_id = TR_ZCL_CLUSTER_GROUPS_ID;
        g_cli_zcl_cmd_creation_s.cb         = NULL;

        // print the command buffer
        tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);
    }
    else
    {
        tr_core_printf("usage: remove -i group-id\n");
    }

    return ret_val;
}

zb_int_t cli_groups_remove_all(zb_int_t  argc,
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
                                                                           TR_ZCL_CMD_REMOVE_ALL_GROUPS_ID);

    // save the profile and cluster for use by the send command
    g_cli_zcl_cmd_creation_s.prof_id    = ZB_AF_HA_PROFILE_ID;
    g_cli_zcl_cmd_creation_s.cluster_id = TR_ZCL_CLUSTER_GROUPS_ID;
    g_cli_zcl_cmd_creation_s.cb         = NULL;

    // print the command buffer
    tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);

    return ZB_TRUE;
}

zb_int_t cli_groups_add_if_identifying(zb_int_t  argc,
                                       zb_char_t *argv[])
{
    zb_uint16_t group_id       = 0;
    zb_uint8_t  group_name_len = 0;
    zb_char_t   group_name[18] = { 0 };
    zb_int_t    ret_val        = ZB_TRUE;
    zb_char_t   *option_argument;

    // get group id
    if (tr_cli_get_option(argc, argv, "i:", &option_argument))
    {
        group_id = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // get optional group name
    if (tr_cli_get_option(argc, argv, "n:", &option_argument))
    {
        // NOTE: max group name size is 16
        for (group_name_len = 0 ; group_name_len < 16 ; group_name_len++)
        {
            if (group_name_len < strlen(option_argument))
            {
                group_name[group_name_len + 1] = option_argument[group_name_len];
            }
            else
            {
                break;
            }
        }
        group_name[0] = group_name_len;
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
                                                                               TR_ZCL_CMD_ADD_GROUP_IF_IDENTIFYING_ID);

        ZB_ZCL_PACKET_PUT_DATA16_VAL(g_cli_zcl_cmd_creation_s.cmd_ptr, group_id);

        // populate group name if it exists
        if (group_name[0] > 0)
        {
            ZB_ZCL_PACKET_PUT_DATA_N(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                     group_name,
                                     group_name[0] + 1);
        }
        else
        {
            ZB_ZCL_PACKET_PUT_DATA8(g_cli_zcl_cmd_creation_s.cmd_ptr, ZB_ZCL_NULL_STRING);
        }

        // save the profile and cluster for use by the send command
        g_cli_zcl_cmd_creation_s.prof_id    = ZB_AF_HA_PROFILE_ID;
        g_cli_zcl_cmd_creation_s.cluster_id = TR_ZCL_CLUSTER_GROUPS_ID;
        g_cli_zcl_cmd_creation_s.cb         = NULL;

        // print the command buffer
        tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);
    }
    else
    {
        tr_core_printf("usage: id-add -i group-id [-n \"group-name\"]\n");
    }

    return ret_val;
}

TR_CLI_COMMAND_TABLE(zcl_groups_c_cluster_commands) =
{
    { "add",    cli_groups_add,                    "add group command"                },
    { "view",   cli_groups_view,                   "view group command"               },
    { "get",    cli_groups_get_membership,         "get group membership command"     },
    { "remove", cli_groups_remove,                 "remove group command"             },
    { "rmall",  cli_groups_remove_all,             "remove all groups command"        },
    { "id-add", cli_groups_add_if_identifying,     "add group if identifying command" },
#ifdef TR_GROUPS_SERVER_CLI_ENABLE
    { "print",  cli_cmd_groups_server_print_table, "print groups table (server)"      },
#endif
    TR_CLI_COMMAND_TABLE_END
};
