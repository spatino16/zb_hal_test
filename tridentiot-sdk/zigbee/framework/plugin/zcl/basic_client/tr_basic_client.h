/// ****************************************************************************
/// @file tr_basic_client.h
///
/// @brief ZCL BASIC cluster client implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_BASIC_CLIENT_H
#define TR_BASIC_CLIENT_H

#include "tr_af.h"
#include "zb_zcl_basic.h"

/// ****************************************************************************
///                                 debug prints
/// ****************************************************************************
#if defined(TR_BASIC_CLIENT_PLUGIN_PRINT_ENABLE) && (TR_BASIC_CLIENT_PLUGIN_PRINT_ENABLE == 1)
#define tr_basic_client_printf(...)  tr_zcl_printf(__VA_ARGS__)
#define tr_basic_client_println(...) tr_zcl_println(__VA_ARGS__)
#else
#define tr_basic_client_printf(...)
#define tr_basic_client_println(...)
#endif

/// ****************************************************************************
/// @defgroup zcl_basic_client_cb Basic Client Callbacks
/// @ingroup zcl_app_callbacks
/// @{
/// ****************************************************************************

/// @brief Callback fires when basic client cluster is initialized
void tr_basic_client_init_cb(void);

/// @} // end of zcl_app_callbacks

/// ****************************************************************************
/// function prototypes consumed by framework
/// ****************************************************************************
void tr_basic_client_init(void);

#endif // TR_BASIC_SERVER_H
