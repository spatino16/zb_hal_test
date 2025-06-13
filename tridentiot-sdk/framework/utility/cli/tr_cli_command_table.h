/// ****************************************************************************
/// @file tr_cli_command_table.h
///
/// @brief common CLI command table structure
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_CLI_COMMAND_TABLE_H
#define TR_CLI_COMMAND_TABLE_H

// structure used for each cli command table entry
typedef struct
{
    const char *command;
    int (*handler)(int  argc,
                   char *argv[]);
    const char *help;
} tr_command_s;

// macros for creating cli command tables
#define TR_CLI_COMMAND_TABLE(table) const tr_command_s table[]
#define TR_CLI_COMMAND_TABLE_END { "", NULL, "" }
#define TR_CLI_SUB_COMMANDS      NULL
#define TR_CLI_SUB_COMMAND_TABLE (const char*)


#endif // TR_CLI_COMMAND_TABLE_H
