/// ****************************************************************************
/// @file tr_remote_cli_server_cb_stubs.c
///
/// @brief weakly defined application callback stubs
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_remote_cli_server.h"

ZB_WEAK void tr_remote_cli_server_init_cb(void)
{
}

ZB_WEAK void tr_remote_cli_server_write_attr_cb(zb_uint8_t  endpoint,
                                                zb_uint16_t attr_id,
                                                zb_uint8_t  *new_value,
                                                zb_uint16_t manuf_code)
{
    ZVUNUSED(endpoint);
    ZVUNUSED(attr_id);
    ZVUNUSED(new_value);
    ZVUNUSED(manuf_code);
}
