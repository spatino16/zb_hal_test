/// ****************************************************************************
/// @file tr_alarms_client.h
///
/// @brief ZCL ALARMS cluster client implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_ALARMS_CLIENT_H
#define TR_ALARMS_CLIENT_H

#include "tr_af.h"
#include "zb_zcl_alarms.h"

/// ****************************************************************************
///                                 debug prints
/// ****************************************************************************
#if defined(TR_ALARMS_CLIENT_PLUGIN_PRINT_ENABLE) && (TR_ALARMS_CLIENT_PLUGIN_PRINT_ENABLE == 1)
#define tr_alarms_client_printf(...)  tr_zcl_printf(__VA_ARGS__)
#define tr_alarms_client_println(...) tr_zcl_println(__VA_ARGS__)
#else
#define tr_alarms_client_printf(...)
#define tr_alarms_client_println(...)
#endif

/// ****************************************************************************
/// @defgroup zcl_alarms_client_cb Alarms Client Callbacks
/// @ingroup zcl_app_callbacks
/// @{
/// ****************************************************************************

/// @brief Callback fires when alarms client cluster is initialized
void tr_alarms_client_init_cb(void);

/// @brief Callback that user can declare to handle the alarms cluster alarm command
/// @param cmd_info struct that contains zcl header info
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_alarms_client_alarm_cb(zb_zcl_parsed_hdr_t *cmd_info);

/// @} // end of zcl_app_callbacks

/// ****************************************************************************
/// function prototypes consumed by framework
/// ****************************************************************************
void tr_alarms_client_init(void);

#endif // TR_ALARMS_SERVER_H
