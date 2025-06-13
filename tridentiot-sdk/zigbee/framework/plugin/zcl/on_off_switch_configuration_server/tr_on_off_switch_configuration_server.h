/// ****************************************************************************
/// @file tr_on_off_switch_configuration_server.h
///
/// @brief ZCL ON/OFF SWITCH CONFIG cluster server implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_ON_OFF_SWITCH_CONFIGURATION_SERVER_H
#define TR_ON_OFF_SWITCH_CONFIGURATION_SERVER_H

#include "tr_af.h"
#include "zb_zcl_on_off_switch_conf.h"

/// ****************************************************************************
///                                 debug prints
/// ****************************************************************************
#if defined(TR_ON_OFF_SWITCH_CONFIGURATION_SERVER_PLUGIN_PRINT_ENABLE) && \
    (TR_ON_OFF_SWITCH_CONFIGURATION_SERVER_PLUGIN_PRINT_ENABLE == 1)
#define tr_on_off_switch_configuration_server_printf(...)  tr_zcl_printf(__VA_ARGS__)
#define tr_on_off_switch_configuration_server_println(...) tr_zcl_println(__VA_ARGS__)
#else
#define tr_on_off_switch_configuration_server_printf(...)
#define tr_on_off_switch_configuration_server_println(...)
#endif

/// ****************************************************************************
/// @defgroup zcl_on_off_switch_config_server_cb On/Off Switch Configuration Server Callbacks
/// @ingroup zcl_app_callbacks
/// @{
/// ****************************************************************************

/// @brief Callback fires when on off switch configuration server cluster is initialized
void tr_on_off_switch_configuration_server_init_cb(void);

/// @brief Callback fires when a on off switch configuration server attribute is about to be written
/// @param endpoint   device endpoint
/// @param attr_id    ZCL attribute id
/// @param new_value  pointer to the new attribute value
/// @param manuf_code manufacturer specific code
void tr_on_off_switch_configuration_server_write_attr_cb(zb_uint8_t  endpoint,
                                                         zb_uint16_t attr_id,
                                                         zb_uint8_t  *new_value,
                                                         zb_uint16_t manuf_code);

/// @} // end of zcl_app_callbacks

/// ****************************************************************************
/// function prototypes consumed by framework
/// ****************************************************************************
void tr_on_off_switch_configuration_server_init(void);

#endif // TR_ON_OFF_SWITCH_CONFIGURATION_SERVER_H
