/// ****************************************************************************
/// @file tr_power_configuration_client.h
///
/// @brief ZCL POWER CONFIGURATION cluster client implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_POWER_CONFIGURATION_CLIENT_H
#define TR_POWER_CONFIGURATION_CLIENT_H

#include "tr_af.h"
#include "zb_zcl_power_config.h"

/// ****************************************************************************
///                                 debug prints
/// ****************************************************************************
#if defined(TR_POWER_CONFIGURATION_CLIENT_PLUGIN_PRINT_ENABLE) && \
    (TR_POWER_CONFIGURATION_CLIENT_PLUGIN_PRINT_ENABLE == 1)
#define tr_power_configuration_client_printf(...)  tr_zcl_printf(__VA_ARGS__)
#define tr_power_configuration_client_println(...) tr_zcl_println(__VA_ARGS__)
#else
#define tr_power_configuration_client_printf(...)
#define tr_power_configuration_client_println(...)
#endif

/// ****************************************************************************
/// @defgroup zcl_power_config_client_cb Power Configuration Client Callbacks
/// @ingroup zcl_app_callbacks
/// @{
/// ****************************************************************************

/// @brief Callback fires when power configuration client cluster is initialized
void tr_power_configuration_client_init_cb(void);

/// @} // end of zcl_app_callbacks

/// ****************************************************************************
/// function prototypes consumed by framework
/// ****************************************************************************
void tr_power_configuration_client_init(void);

#endif // TR_POWER_CONFIGURATION_CLIENT_H
