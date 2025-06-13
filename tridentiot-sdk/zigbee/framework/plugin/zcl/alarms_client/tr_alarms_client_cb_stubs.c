/// ****************************************************************************
/// @file tr_alarms_client_cb_stubs.c
///
/// @brief weakly defined application callback stubs
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_alarms_client.h"

ZB_WEAK void tr_alarms_client_init_cb(void)
{
}

ZB_WEAK zb_bool_t tr_alarms_client_alarm_cb(zb_zcl_parsed_hdr_t *cmd_info)
{
    ZVUNUSED(cmd_info);
    // can be declared in user application to init app specific functionality
    // return ZB_FALSE to allow framework to continue processing the command
    return ZB_FALSE;
}
