/// ****************************************************************************
/// @file tr_on_off_switch_configuration_client.h
///
/// @brief ZCL ON/OFF SWITCH CONFIG cluster client implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_ON_OFF_SWITCH_CONFIGURATION_CLIENT_H
#define TR_ON_OFF_SWITCH_CONFIGURATION_CLIENT_H

#include "tr_af.h"
#include "zb_zcl_on_off_switch_conf.h"

/// ****************************************************************************
///                                 debug prints
/// ****************************************************************************
#if defined(TR_ON_OFF_SWITCH_CONFIGURATION_CLIENT_PLUGIN_PRINT_ENABLE) && \
    (TR_ON_OFF_SWITCH_CONFIGURATION_CLIENT_PLUGIN_PRINT_ENABLE == 1)
#define tr_on_off_switch_configuration_client_printf(...)  tr_zcl_printf(__VA_ARGS__)
#define tr_on_off_switch_configuration_client_println(...) tr_zcl_println(__VA_ARGS__)
#else
#define tr_on_off_switch_configuration_client_printf(...)
#define tr_on_off_switch_configuration_client_println(...)
#endif

/// ****************************************************************************
/// @defgroup zcl_on_off_switch_config_client_cb On/Off Switch Configuration Client Callbacks
/// @ingroup zcl_app_callbacks
/// @{
/// ****************************************************************************

/// @brief Callback fires when on off switch configuration client cluster is initialized
void tr_on_off_switch_configuration_client_init_cb(void);

/// @} // end of zcl_app_callbacks

/// ****************************************************************************
/// function prototypes consumed by framework
/// ****************************************************************************
void tr_on_off_switch_configuration_client_init(void);

#endif // TR_ON_OFF_SWITCH_CONFIGURATION_CLIENT_H
