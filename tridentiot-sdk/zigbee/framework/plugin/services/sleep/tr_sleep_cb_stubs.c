/// ****************************************************************************
/// @file tr_sleep_cb_stubs.c
///
/// @brief weakly defined application callback stubs
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "zb_common.h"

ZB_WEAK zb_bool_t tr_pre_sleep_cb(zb_uint32_t sleep_duration_ms)
{
    ZVUNUSED(sleep_duration_ms);
    return ZB_TRUE;
}

ZB_WEAK void tr_post_wake_cb(void)
{
}
