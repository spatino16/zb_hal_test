/// ****************************************************************************
/// @file tr_remote_cli_client_cli.c
///
/// @brief Contains CLI commands specific to the REMOTE CLI client cluster
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_cli_argument_parser.h"
#include "tr_cli_zcl_cmds.h"
#include "tr_remote_cli_client.h"

// send the remote cli cli command to a remote device
zb_int_t cli_remote_cli_send(zb_int_t  argc,
                             zb_char_t *argv[])
{
    zb_int_t           ret_val = ZB_TRUE;
    zb_char_t          *option_argument;
    static zb_uint16_t short_addr = 0;
    static zb_uint8_t  endpoint   = 1;

    // get the destination short addr
    if (tr_cli_get_option(argc, argv, "a:", &option_argument))
    {
        short_addr = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
    }

    // get the endpoint
    if (tr_cli_get_option(argc, argv, "d:", &option_argument))
    {
        endpoint = (zb_uint8_t)tr_dec_or_hex_string_to_int(option_argument);
    }

    // is this a -h for help?
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    // get the command to send
    // do this last so option_argument points to the string to send
    if (!tr_cli_get_option(argc, argv, "c:", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        tr_remote_cli_client_send_cli_command(option_argument, short_addr, endpoint);
    }
    else
    {
        tr_core_printf("usage: send -c command string [-a short addr (default 0x0000 or last used)] [-d dest endpoint (default 1 or last used)]\n");
        ret_val = ZB_FALSE;
    }

    return ret_val;
}

// send the remote cli enable command to a remote device
zb_int_t cli_remote_cli_enable(zb_int_t  argc,
                               zb_char_t *argv[])
{
    zb_int_t                       ret_val = ZB_TRUE;
    zb_char_t                      *option_argument;
    zb_uint16_t                    poll_rate = 0;
    tr_remote_cli_cli_status_arg_t cli_enable;
    static zb_uint16_t             short_addr = 0;
    static zb_uint8_t              endpoint   = 1;

    cli_enable.value = 0;

    // get the destination short addr
    if (tr_cli_get_option(argc, argv, "a:", &option_argument))
    {
        short_addr = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
    }

    // get the endpoint
    if (tr_cli_get_option(argc, argv, "d:", &option_argument))
    {
        endpoint = (zb_uint8_t)tr_dec_or_hex_string_to_int(option_argument);
    }

    // get the local enable
    if (tr_cli_get_option(argc, argv, "l:", &option_argument))
    {
        cli_enable.bits.local_enable = tr_dec_or_hex_string_to_int(option_argument);
    }

    // get the remote enable
    if (tr_cli_get_option(argc, argv, "r:", &option_argument))
    {
        cli_enable.bits.remote_enable = tr_dec_or_hex_string_to_int(option_argument);
    }

    // is this a -h for help?
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        tr_remote_cli_client_send_cli_enable(cli_enable, poll_rate, short_addr, endpoint);
    }
    else
    {
        tr_core_printf(
            "usage: enable [-l 1/0 local enable] [-r 1/0 remote enable] [-a short_addr (default 0x0000 or last used)] [-d dest_endpoint (default 1 or last used)]\n");
        ret_val = ZB_FALSE;
    }

    return ret_val;
}

TR_CLI_COMMAND_TABLE(zcl_remote_cli_c_cluster_commands) =
{
    { "send",   cli_remote_cli_send,   "send a cli command to a remote device"                                                                         },
    { "enable", cli_remote_cli_enable, "enable/disable remote interactive mode"                                                                        },
    TR_CLI_COMMAND_TABLE_END
};
