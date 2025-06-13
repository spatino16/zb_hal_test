/// ****************************************************************************
/// @file tr_network_rejoin_cb_stubs.c
///
/// @brief weakly defined application callback stubs
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_af.h"

/// @brief User callback to indicate a rejoin attempt is about to be made
/// @param channel_mask channel(s) to be used for the rejoin
/// @param secure ZB_TRUE for secure rejoin attempt, ZB_FALSE for tc (unsecure) rejoin
/// @return ZB_TRUE to allow rejoin attemp to be made, ZB_FALSE to stop it
ZB_WEAK zb_bool_t tr_network_rejoin_attempt_cb(zb_uint32_t channel_mask,
                                               zb_bool_t   secure)
{
    (void)channel_mask;
    (void)secure;
    return ZB_TRUE;
}
