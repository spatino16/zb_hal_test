/// ****************************************************************************
/// @file tr_identify_client_cb_stubs.c
///
/// @brief weakly defined application callback stubs
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_identify_client.h"

ZB_WEAK void tr_identify_client_init_cb(void)
{
}

ZB_WEAK void tr_identify_client_identify_query_resp_cb(zb_zcl_parsed_hdr_t *cmd_info)
{
    ZVUNUSED(cmd_info);
}
