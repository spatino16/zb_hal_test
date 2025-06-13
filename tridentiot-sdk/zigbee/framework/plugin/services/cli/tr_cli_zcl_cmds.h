/// ****************************************************************************
/// @file tr_cli_zcl_cmds.h
///
/// @brief TODO - documentation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_CLI_ZCL_COMMANDS_H
#define TR_CLI_ZCL_COMMANDS_H

typedef struct
{
    zb_bufid_t    buffer;
    zb_uint8_t    *cmd_ptr;
    zb_uint16_t   prof_id;
    zb_uint16_t   cluster_id;
    zb_callback_t cb;
} tr_cli_zcl_cmd_creation_s;

// this is where zcl commands
extern tr_cli_zcl_cmd_creation_s g_cli_zcl_cmd_creation_s;

#endif // TR_CLI_ZCL_COMMANDS_H
