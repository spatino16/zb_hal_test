/// ****************************************************************************
/// @file find_and_bind_cli.c
///
/// @brief CLI for the find and bind plugin. This handles both initiator and target
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_af.h"
#include "tr_cli_argument_parser.h"

static uint16_t f_b_cluster;

bool_t tr_finding_and_binding_cb(int16_t        status,
                                 zb_ieee_addr_t ieee_address,
                                 uint8_t        ep,
                                 uint16_t       cluster)
{
    if (status == ZB_BDB_COMM_BIND_ASK_USER)
    {
        // check the cluster
        if (f_b_cluster == cluster)
        {
            tr_core_printf("Binding to eui 0x");
            tr_print_eui64(ieee_address);
            tr_core_printf(" ep %d, cluster 0x%4.4x\n", ep, cluster);

            // returning true tells the ZBOSS f&b handler to create the binding
            return TRUE;
        }
    }

    // returning false tells the ZBOSS f&b handler NOT to create the binding
    return FALSE;
}

int cli_find_and_bind_ini_start_command(int  argc,
                                        char *argv[])
{
    uint8_t  ret_val = TRUE;
    zb_ret_t status;
    uint8_t  endpoint = 1;
    char     *option_argument;

    // set the matching cluster to invalid
    f_b_cluster = 0xFFFF;

    // get the endpoint
    if (tr_cli_get_option(argc, argv, "e:", &option_argument))
    {
        endpoint = (uint8_t)tr_dec_or_hex_string_to_int(option_argument);
    }

    // get the cluster
    if (tr_cli_get_option(argc, argv, "c:", &option_argument))
    {
        f_b_cluster = (uint16_t)tr_dec_or_hex_string_to_int(option_argument);
    }

    // check for help
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = FALSE;
    }

    if (ret_val)
    {
        status = zb_bdb_finding_binding_initiator(endpoint, (zb_bdb_comm_binding_callback_t)tr_finding_and_binding_cb);
        tr_core_printf("find-and-bind initiator start status = 0x%x\n", status);
    }
    else
    {
        tr_core_printf("usage: find-and-bind initiator start [-e endpoint (default=1)] [-c cluster (default=0xFFFF, don't match any clusters)]\n");
    }

    ZVUNUSED(status);

    return 0;
}

int cli_find_and_bind_ini_stop_command(int  argc,
                                       char *argv[])
{
    uint8_t ret_val = TRUE;
    char    *option_argument;

    // set the matching cluster to invalid
    f_b_cluster = 0xFFFF;

    // check for help
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = FALSE;
    }

    if (ret_val)
    {
        zb_bdb_finding_binding_initiator_cancel();
    }
    else
    {
        tr_core_printf("usage: find-and-bind initiator stop\n");
    }

    return 0;
}

int cli_find_and_bind_tar_start_command(int  argc,
                                        char *argv[])
{
    uint8_t  ret_val = TRUE;
    zb_ret_t status;
    uint8_t  endpoint      = 1;
    uint16_t comm_time_sec = ZB_BDBC_MIN_COMMISSIONING_TIME_S;
    char     *option_argument;

    // get the endpoint
    if (tr_cli_get_option(argc, argv, "e:", &option_argument))
    {
        endpoint = (uint8_t)tr_dec_or_hex_string_to_int(option_argument);
    }

    // get the commissioning time. ZBOSS does not allow anything less than 180 seconds
    if (tr_cli_get_option(argc, argv, "t:", &option_argument))
    {
        comm_time_sec = (uint16_t)tr_dec_or_hex_string_to_int(option_argument);
    }

    // check for help
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = FALSE;
    }

    if (ret_val)
    {
        status = zb_bdb_finding_binding_target_ext(endpoint, comm_time_sec);
        tr_core_printf("find-and-bind target start status = 0x%x\n", status);
    }
    else
    {
        tr_core_printf("usage: find-and-bind target start [-e endpoint (default=1)] [-t commissioning time seconds, must be >=180 (default=180)]\n");
    }

    ZVUNUSED(status);

    return 0;
}

int cli_find_and_bind_tar_stop_command(int  argc,
                                       char *argv[])
{
    uint8_t ret_val = TRUE;
    char    *option_argument;

    // check for help
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = FALSE;
    }

    if (ret_val)
    {
        zb_bdb_finding_binding_target_cancel();
    }
    else
    {
        tr_core_printf("usage: find-and-bind target stop\n");
    }

    return 0;
}

TR_CLI_COMMAND_TABLE(plugin_find_and_bind_initiator_commands) =
{
    { "start", cli_find_and_bind_ini_start_command, "start initiator finding and binding" },
    { "stop",  cli_find_and_bind_ini_stop_command,  "stop initiator finding and binding"  },
    TR_CLI_COMMAND_TABLE_END
};

TR_CLI_COMMAND_TABLE(plugin_find_and_bind_target_commands) =
{
    { "start", cli_find_and_bind_tar_start_command, "start target finding and binding" },
    { "stop",  cli_find_and_bind_tar_stop_command,  "stop target finding and binding"  },
    TR_CLI_COMMAND_TABLE_END
};

TR_CLI_COMMAND_TABLE(plugin_find_and_bind_commands) =
{
    { "initiator", TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(plugin_find_and_bind_initiator_commands) },
    { "target",    TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(plugin_find_and_bind_target_commands)    },
    TR_CLI_COMMAND_TABLE_END
};
