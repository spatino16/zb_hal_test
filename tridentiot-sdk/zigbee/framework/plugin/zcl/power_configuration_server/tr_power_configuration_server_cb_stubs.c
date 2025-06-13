/// ****************************************************************************
/// @file tr_power_configuration_server_cb_stubs.c
///
/// @brief weakly defined application callback stubs
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_power_configuration_server.h"

ZB_WEAK void tr_power_configuration_server_init_cb(void)
{
}

ZB_WEAK void tr_power_configuration_server_write_attr_cb(zb_uint8_t  endpoint,
                                                         zb_uint16_t attr_id,
                                                         zb_uint8_t  *new_value,
                                                         zb_uint16_t manuf_code)
{
    ZVUNUSED(endpoint);
    ZVUNUSED(attr_id);
    ZVUNUSED(new_value);
    ZVUNUSED(manuf_code);
}

ZB_WEAK void tr_power_configuration_server_battery_alarm_state_changed_cb(zb_uint8_t  endpoint,
                                                                          zb_uint32_t batt_alarm_state)
{
    ZVUNUSED(endpoint);
    ZVUNUSED(batt_alarm_state);
}

ZB_WEAK zb_bool_t tr_power_configuration_server_pre_alarm_send_cb(zb_uint8_t endpoint,
                                                                  zb_uint8_t alarm_code)
{
    ZVUNUSED(endpoint);
    ZVUNUSED(alarm_code);
    return ZB_FALSE;
}
