/// ****************************************************************************
/// @file tr_identify_server_cb_stubs.c
///
/// @brief weakly defined application callback stubs
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_identify_server.h"

ZB_WEAK void tr_identify_server_init_cb(void)
{
}

ZB_WEAK void tr_identify_server_identify_start_cb(zb_uint8_t  endpoint,
                                                  zb_uint16_t timeout_sec)
{
    ZVUNUSED(endpoint);
    ZVUNUSED(timeout_sec);
}

ZB_WEAK void tr_identify_server_identify_stop_cb(zb_uint8_t endpoint)
{
    ZVUNUSED(endpoint);
}

ZB_WEAK zb_ret_t tr_identify_server_trigger_effect_cb(zb_zcl_identify_effect_user_app_schedule_t *invoke_data)
{
    ZVUNUSED(invoke_data);
    return RET_OK;
}

ZB_WEAK void tr_identify_server_write_attr_cb(zb_uint8_t  endpoint,
                                              zb_uint16_t attr_id,
                                              zb_uint8_t  *new_value,
                                              zb_uint16_t manuf_code)
{
    ZVUNUSED(endpoint);
    ZVUNUSED(attr_id);
    ZVUNUSED(new_value);
    ZVUNUSED(manuf_code);
}
