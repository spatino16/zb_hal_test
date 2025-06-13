/// ****************************************************************************
/// @file tr_on_off_server.h
///
/// @brief ZCL ON/OFF cluster server implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_ON_OFF_SERVER_H
#define TR_ON_OFF_SERVER_H

#include "tr_af.h"
#include "zb_zcl_on_off.h"

/// ****************************************************************************
///                                 debug prints
/// ****************************************************************************
#if defined(TR_ON_OFF_SERVER_PLUGIN_PRINT_ENABLE) && (TR_ON_OFF_SERVER_PLUGIN_PRINT_ENABLE == 1)
#define tr_on_off_server_printf(...)  tr_zcl_printf(__VA_ARGS__)
#define tr_on_off_server_println(...) tr_zcl_println(__VA_ARGS__)
#else
#define tr_on_off_server_printf(...)
#define tr_on_off_server_println(...)
#endif

/// ****************************************************************************
/// @defgroup zcl_on_off_server_cb On/Off Server Callbacks
/// @ingroup zcl_app_callbacks
/// @{
/// ****************************************************************************


/// @} // end of zcl_app_callbacks

/// ****************************************************************************
/// function prototypes consumed by framework
/// ****************************************************************************
void tr_on_off_server_init(void);

#endif // TR_ON_OFF_SERVER_H
