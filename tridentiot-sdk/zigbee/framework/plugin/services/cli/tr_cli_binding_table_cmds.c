/// ****************************************************************************
/// @file find_and_bind_cli.c
///
/// @brief CLI for manipulating binding table entries
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_af.h"
#include "tr_cli_argument_parser.h"

extern int cli_cmd_print_bind(int  argc,
                              char *argv[]);

int cli_add_binding_table_entry_command(int  argc,
                                        char *argv[])
{
    tr_core_printf("coming soon\n");
    return 0;
}

int cli_delete_binding_table_entry_command(int  argc,
                                           char *argv[])
{
    tr_core_printf("coming soon\n");
    return 0;
}

int cli_clear_binding_table_command(int  argc,
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
        zb_apsme_unbind_all(0);
    }
    else
    {
        tr_core_printf("usage: binding-table clear\n");
    }

    return 0;
}

int cli_print_binding_table_command(int  argc,
                                    char *argv[])
{
    cli_cmd_print_bind(argc, argv);
    return 0;
}

TR_CLI_COMMAND_TABLE(plugin_binding_table_commands) =
{
    { "add",    cli_add_binding_table_entry_command,    "add an entry to the binding table"        },
    { "delete", cli_delete_binding_table_entry_command, "delete an entry from the binding table"   },
    { "clear",  cli_clear_binding_table_command,        "clear all entries from the binding table" },
    { "print",  cli_print_binding_table_command,        "print the binding table"                  },
    TR_CLI_COMMAND_TABLE_END
};
