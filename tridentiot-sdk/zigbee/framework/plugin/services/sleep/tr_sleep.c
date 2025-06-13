/// ****************************************************************************
/// @file tr_sleep.c
///
/// @brief plugin for allowing sleep and getting alerted about pre/post sleep
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include <stdint.h>
#include "tr_af.h"

#ifdef TR_IS_SLEEPY_ZED

static zb_bool_t tr_sleep_ok = ZB_TRUE;

// this is an application call that be used to allow or block sleep at runtime
void tr_allow_sleep(zb_bool_t allow)
{
    tr_sleep_ok = allow;
}

zb_bool_t tr_check_for_sleep(zb_uint32_t sleep_time_ms)
{
    if (!tr_sleep_ok)
    {
        return ZB_FALSE;
    }

#ifdef TR_STAY_AWAKE_WHEN_NOT_JOINED

    // don't sleep if we are not joined
    if (tr_get_connection_state() != TR_CONN_STATE_JOINED_NETWORK)
    {
        return ZB_FALSE;
    }
#endif

    // allow the user callback to determine if sleep is ok
    return tr_pre_sleep_cb(sleep_time_ms);
}

#endif /* ifdef TR_IS_SLEEPY_ZED */
