/// ****************************************************************************
/// @file tr_remote_cli_common.h
///
/// @brief TODO - documentation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_REMOTE_CLI_COMMON_H
#define TR_REMOTE_CLI_COMMON_H

#define REMOTE_CLI_MFG_ID 0x1570

// Note: the cli status attribute is the same as the cli status argument
typedef union
{
    struct
    {
        zb_uint8_t local_enable : 1;
        zb_uint8_t remote_enable : 1;
        zb_uint8_t unused : 6;
    } bits;

    zb_uint8_t value;
}tr_remote_cli_cli_status_arg_t, tr_remote_cli_cli_status_attr_t;

#endif // ifndef TR_REMOTE_CLI_COMMON_H
