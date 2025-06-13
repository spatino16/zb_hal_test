/// ****************************************************************************
/// @file tr_cli_command_parser.h
///
/// @brief common CLI utility for parsing incoming commands
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_CLI_COMMAND_PARSER_H
#define TR_CLI_COMMAND_PARSER_H

#include <stdint.h>
#include "tr_cli_command_table.h"

#define TR_ARGUMENT_PARSER_MAX_ARGS (15)

void tr_cli_parse_command(const tr_command_s *cmd_table,
                          int                argc,
                          char               **argv);

uint8_t tr_cli_get_option(int  argc,
                          char *argv[],
                          char *opt_string,
                          char **ret_arg);

uint64_t tr_dec_or_hex_string_to_int(const char *number_string);

#endif // TR_CLI_COMMAND_PARSER_H
