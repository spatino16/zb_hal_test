/// ****************************************************************************
/// @file tr_groups_server_cli.h
///
/// @brief Contains CLI commands specific to the GROUPS server cluster
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_groups_server.h"
#include "tr_cli_argument_parser.h"


/// ****************************************************************************
/// function prototypes consumed by framework
/// ****************************************************************************
zb_int_t cli_cmd_groups_server_print_table(zb_int_t  argc,
                                           zb_char_t *argv[]);
