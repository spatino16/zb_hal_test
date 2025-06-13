/// ****************************************************************************
/// @file tr_zcl_common_cb_stubs.c
///
/// @brief weakly defined application callback stubs
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "zb_common.h"

ZB_WEAK zb_uint8_t tr_zcl_command_cb(zb_uint8_t param)
{
    ZVUNUSED(param);
    return 0;
}
