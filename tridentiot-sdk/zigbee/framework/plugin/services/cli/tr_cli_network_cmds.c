/// ****************************************************************************
/// @file tr_cli_network_cmds.c
///
/// @brief CLI network commands
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include <string.h>
#include "tr_af.h"
#include "tr_cli_argument_parser.h"

// network join command
zb_int_t cli_network_join(zb_int_t  argc,
                          zb_char_t *argv[])
{
    // scan primary and secondary channel sets once
    zb_zdo_set_nwk_scan_attempts(1);

    if (bdb_start_top_level_commissioning(ZB_BDB_NETWORK_STEERING))
    {
        tr_core_printf("Network Steering Start Success\n");
    }
    else
    {
        tr_core_printf("Network Steering Start Error\n");
    }

    return 0;
}

// network rejoin command
zb_int_t cli_network_rejoin(zb_int_t  argc,
                            zb_char_t *argv[])
{
    zb_uint32_t channel_mask = 0; // 0 means use current channel
    zb_bool_t   secure       = ZB_TRUE;
    zb_char_t   *option_argument;

    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        tr_core_printf("usage: network rejoin [-c channel mask <default 0 (current channel)>] [-t tc rejoin <default secure rejoin>]\n");
        return 0;
    }

    if (tr_cli_get_option(argc, argv, "t", &option_argument))
    {
        // do a trust center (unsecure) rejoin
        secure = ZB_FALSE;
    }

    if (tr_cli_get_option(argc, argv, "c:", &option_argument))
    {
        // get the channel mask
        channel_mask = (zb_uint32_t)tr_dec_or_hex_string_to_int(option_argument);
    }

    tr_network_rejoin(channel_mask, secure);
    tr_print_cli_cmd_status("Rejoin", 0);

    return 0;
}

// network leave command
zb_int_t cli_network_leave(zb_int_t  argc,
                           zb_char_t *argv[])
{
    zb_int_t   ret_val = ZB_TRUE;
    zb_char_t  *option_argument;
    zb_bufid_t buffer;

    // if the -r option is specified perform a reset to factory defaults
    if (tr_cli_get_option(argc, argv, "r", &option_argument))
    {

        tr_core_printf("Reset to factory defaults not implemented yet\n");
    }

    // if the -h option is specified, just print the usage string
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val == ZB_TRUE)
    {
        buffer = zb_buf_get_out();
        zdo_commissioning_leave(buffer, ZB_FALSE, ZB_FALSE);
        tr_print_cli_cmd_status("Leave", 0);
    }
    else
    {
        // at least 1 arg is wrong or -h was specified, print usage
        tr_core_printf("usage: leave [-r also reset to factory defaults]\n");
    }

    return 0;
}

// network form command
zb_int_t cli_network_form(zb_int_t  argc,
                          zb_char_t *argv[])
{
    zb_int_t  ret_val = ZB_TRUE;
    zb_char_t *option_argument;

    // if the -h option is specified, just print the usage string
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    // TODO: It is unclear if ZBOSS supports manual network formation if BDB is enabled.
    if (ret_val == ZB_TRUE)
    {
        if (1)
        {
            tr_core_printf("Network formation command coming soon\n");
        }
        // tr_print_cli_cmd_status("Form", status);
    }
    else
    {
        // at least 1 arg is wrong or -h was specified, print usage
        tr_core_printf("usage: form -p pan id -c channel -p tx power\n");
    }

    return 0;
}

// TODO LCD 5/5/25 The intent was to be able to switch between r23 and r22 behavior on the fly.
// The projects default to r23 behavior and the dynamic switch to r22 seems to work, but switching
// back to r23 doesn't seem to work at this time.
#if 0

// network use r22 behavior command
zb_int_t cli_network_use_r22(zb_int_t  argc,
                             zb_char_t *argv[])
{
    zb_int_t  ret_val = ZB_TRUE;
    zb_char_t *option_argument;

    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val == ZB_TRUE)
    {
        zboss_use_r22_behavior();
    }
    else
    {
        tr_core_printf("usage: network use_r22\n");
    }

    return 0;
}

// network use r23 behavior command
zb_int_t cli_network_use_r23(zb_int_t  argc,
                             zb_char_t *argv[])
{
    zb_int_t  ret_val = ZB_TRUE;
    zb_char_t *option_argument;

    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val == ZB_TRUE)
    {
        zboss_use_r23_behavior();
    }
    else
    {
        tr_core_printf("usage: network use_r23\n");
    }

    return 0;
}

#endif /* if 0 */

// Global commands table
TR_CLI_COMMAND_TABLE(network_commands) =
{
    { "join",   cli_network_join,   "Make a single attempt to join a network" },
    { "rejoin", cli_network_rejoin, "Perform secure or TC (unsecure) rejoin"  },
    { "leave",  cli_network_leave,  "Leave the current network"               },
    { "form",   cli_network_form,   "Form a network"                          },
    // { "use_r22", cli_network_use_r22, "Set stack for Zigbee r22 behavior"       },
    // { "use_r23", cli_network_use_r23, "Set stack for Zigbee r23 behavior"       },
    TR_CLI_COMMAND_TABLE_END
};
