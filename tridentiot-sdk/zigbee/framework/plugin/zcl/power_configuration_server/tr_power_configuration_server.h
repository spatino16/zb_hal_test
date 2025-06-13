/// ****************************************************************************
/// @file tr_power_configuration_server.h
///
/// @brief ZCL POWER CONFIGURATION cluster server implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_POWER_CONFIGURATION_SERVER_H
#define TR_POWER_CONFIGURATION_SERVER_H

#include "tr_af.h"
#include "zb_zcl_power_config.h"

typedef enum
{
    TR_POWER_CONFIGURATION_BATTERY_SOURCE_1 = 0,
    TR_POWER_CONFIGURATION_BATTERY_SOURCE_2 = 1,
    TR_POWER_CONFIGURATION_BATTERY_SOURCE_3 = 2
}tr_power_configuration_server_battery_source_t;

/// ****************************************************************************
///                                 debug prints
/// ****************************************************************************
#if defined(TR_POWER_CONFIGURATION_SERVER_PLUGIN_PRINT_ENABLE) && \
    (TR_POWER_CONFIGURATION_SERVER_PLUGIN_PRINT_ENABLE == 1)
#define tr_power_configuration_server_printf(...)  tr_zcl_printf(__VA_ARGS__)
#define tr_power_configuration_server_println(...) tr_zcl_println(__VA_ARGS__)
#else
#define tr_power_configuration_server_printf(...)
#define tr_power_configuration_server_println(...)
#endif

/// ****************************************************************************
/// @defgroup zcl_power_config_server_cb Power Configuration Server Callbacks
/// @ingroup zcl_app_callbacks
/// @{
/// ****************************************************************************

/// @brief Callback fires when power configuration server cluster is initialized
void tr_power_configuration_server_init_cb(void);

/// @brief Callback fires when a power configuration server attribute is about to be written
/// @param endpoint   device endpoint
/// @param attr_id    ZCL attribute id
/// @param new_value  pointer to the new attribute value
/// @param manuf_code manufacturer specific code
void tr_power_configuration_server_write_attr_cb(zb_uint8_t  endpoint,
                                                 zb_uint16_t attr_id,
                                                 zb_uint8_t  *new_value,
                                                 zb_uint16_t manuf_code);

/// @brief Callback fires when the power configuration server battery alarm state attribute is changes
/// @param endpoint         device endpoint
/// @param batt_alarm_state new battery alarm state value
void tr_power_configuration_server_battery_alarm_state_changed_cb(zb_uint8_t  endpoint,
                                                                  zb_uint32_t batt_alarm_state);

/// @brief Callback fires when the power configuration server is about to send an alarm
/// @param endpoint   device endpoint
/// @param alarm_code alarm code to be sent
/// @return ZB_FALSE to allow framework to continue sending the alarm
zb_bool_t tr_power_configuration_server_pre_alarm_send_cb(zb_uint8_t endpoint,
                                                          zb_uint8_t alarm_code);

/// @} // end of zcl_app_callbacks

/// ****************************************************************************
/// @defgroup zcl_power_config_server_apis Power Configuration Server APIs
/// @ingroup zcl_api_references
/// @{
/// ****************************************************************************

/// @brief API for setting the mains voltage
/// @param endpoint device endpoint
/// @param voltage_100mv RMS voltage in units of 100mV (1173 = 117.3VAC)
/// @return ZB_ZCL_STATUS_SUCCESS or error
zb_zcl_status_t tr_power_configuration_server_set_mains_voltage(zb_uint8_t  endpoint,
                                                                zb_uint16_t voltage_100mv);

/// @brief API for setting the mains frequency
/// @param endpoint device endpoint
/// @param frequency_hz mains frequency in units of 2 Hz (32 = 64Hz)
/// @return ZB_ZCL_STATUS_SUCCESS or error
zb_zcl_status_t tr_power_configuration_server_set_mains_frequency(zb_uint8_t endpoint,
                                                                  zb_uint8_t frequency_hz);

/// @brief API for setting the battery voltage
/// @param endpoint device endpoint
/// @param battery_source battery set (TR_POWER_CONFIGURATION_BATTERY_SOURCE_1, TR_POWER_CONFIGURATION_BATTERY_SOURCE_2, TR_POWER_CONFIGURATION_BATTERY_SOURCE_3)
/// @param voltage_100mv voltage in units of 100mV (33 = 3.3VDC)
/// @return ZB_ZCL_STATUS_SUCCESS or error
zb_zcl_status_t tr_power_configuration_server_set_battery_voltage(zb_uint8_t                                     endpoint,
                                                                  tr_power_configuration_server_battery_source_t battery_source,
                                                                  zb_uint8_t                                     voltage_100mv);

/// @brief API for setting the battery percentage remaining
/// @param endpoint device endpoint
/// @param battery_source battery set (TR_POWER_CONFIGURATION_BATTERY_SOURCE_1, TR_POWER_CONFIGURATION_BATTERY_SOURCE_2, TR_POWER_CONFIGURATION_BATTERY_SOURCE_3)
/// @param percentage_remaining remaining battery life in units of 0.5 percent (151 = 75.5%)
/// @return ZB_ZCL_STATUS_SUCCESS or error
zb_zcl_status_t tr_power_configuration_server_set_battery_percentage_remaining(zb_uint8_t                                     endpoint,
                                                                               tr_power_configuration_server_battery_source_t battery_source,
                                                                               zb_uint8_t                                     percentage_remaining);

/// @brief API to indicate that mains power has been lost
/// @param endpoint device endpoint
/// @param mains_power_lost ZB_TRUE if power is lost, ZB_FALSE if power is not lost
void tr_power_configuration_server_set_clear_mains_power_lost(zb_uint8_t endpoint,
                                                              zb_bool_t  mains_power_lost);

#ifdef TR_POWER_CONFIGURATION_SERVER_BATTERY_LATCHING
/// @brief API to unlatch a low battery condition
/// @param endpoint device endpoint
/// @param battery_source battery set (TR_POWER_CONFIGURATION_BATTERY_SOURCE_1, TR_POWER_CONFIGURATION_BATTERY_SOURCE_2, TR_POWER_CONFIGURATION_BATTERY_SOURCE_3)
void tr_power_configuration_server_unlatch_battery(zb_uint8_t                                     endpoint,
                                                   tr_power_configuration_server_battery_source_t battery_source);

#endif

/// @} // end of zcl_api_references

/// ****************************************************************************
/// function prototypes consumed by framework
/// ****************************************************************************
void tr_power_configuration_server_init(void);

#endif // TR_POWER_CONFIGURATION_SERVER_H
