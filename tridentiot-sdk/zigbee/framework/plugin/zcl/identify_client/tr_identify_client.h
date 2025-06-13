/// ****************************************************************************
/// @file tr_identify_client.h
///
/// @brief ZCL IDENTIFY cluster client implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_IDENTIFY_CLIENT_H
#define TR_IDENTIFY_CLIENT_H

#include "tr_af.h"
#include "zb_zcl_identify.h"

/// ****************************************************************************
///                                 debug prints
/// ****************************************************************************
#if defined(TR_IDENTIFY_CLIENT_PLUGIN_PRINT_ENABLE) && (TR_IDENTIFY_CLIENT_PLUGIN_PRINT_ENABLE == 1)
#define tr_identify_client_printf(...)  tr_zcl_printf(__VA_ARGS__)
#define tr_identify_client_println(...) tr_zcl_println(__VA_ARGS__)
#else
#define tr_identify_client_printf(...)
#define tr_identify_client_println(...)
#endif

/// ****************************************************************************
/// @defgroup zcl_identify_client_cb Identify Client Callbacks
/// @ingroup zcl_app_callbacks
/// @{
/// ****************************************************************************

/// @brief Callback fires when identify client cluster is initialized
void tr_identify_client_init_cb(void);

/// @brief Callback that fires on receipt of identify query response command
/// @param cmd_info contains zcl packet header information
void tr_identify_client_identify_query_resp_cb(zb_zcl_parsed_hdr_t *cmd_info);

/// @} // end of zcl_app_callbacks

/// ****************************************************************************
/// function prototypes consumed by framework
/// ****************************************************************************
void tr_identify_client_init(void);

#endif // TR_IDENTIFY_CLIENT_H
