/// ****************************************************************************
/// @file tr_af_cb_stubs.c
///
/// @brief weakly defined application callback stubs
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "zb_common.h"
#include "tr_af.h"

ZB_WEAK zb_uint8_t *tr_zcl_external_attribute_read_cb(zb_uint8_t  endpoint,
                                                      zb_uint16_t cluster_id,
                                                      zb_uint8_t  cluster_role,
                                                      zb_uint16_t attr_id,
                                                      zb_uint16_t manuf_code)
{
    ZVUNUSED(endpoint);
    ZVUNUSED(cluster_id);
    ZVUNUSED(cluster_role);
    ZVUNUSED(attr_id);
    ZVUNUSED(manuf_code);
    return NULL;
}

ZB_WEAK void tr_app_init_cb(void)
{
}

ZB_WEAK void tr_connection_state_cb(tr_conn_state_e conn_state)
{
    ZVUNUSED(conn_state);
}
