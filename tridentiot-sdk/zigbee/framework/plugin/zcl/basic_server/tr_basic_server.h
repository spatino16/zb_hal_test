/// ****************************************************************************
/// @file tr_basic_server.h
///
/// @brief ZCL BASIC cluster server implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_BASIC_SERVER_H
#define TR_BASIC_SERVER_H

#include "tr_af.h"
#include "zb_zcl_basic.h"

/// ****************************************************************************
///                                 debug prints
/// ****************************************************************************
#if defined(TR_BASIC_SERVER_PLUGIN_PRINT_ENABLE) && (TR_BASIC_SERVER_PLUGIN_PRINT_ENABLE == 1)
#define tr_basic_server_printf(...)  tr_zcl_printf(__VA_ARGS__)
#define tr_basic_server_println(...) tr_zcl_println(__VA_ARGS__)
#else
#define tr_basic_server_printf(...)
#define tr_basic_server_println(...)
#endif

/// ****************************************************************************
/// @defgroup zcl_basic_server_cb Basic Server Callbacks
/// @ingroup zcl_app_callbacks
/// @{
/// ****************************************************************************

/// @brief Callback fires when basic server cluster is initialized
void tr_basic_server_init_cb(void);

/// @brief Callback that user can declare to handle basic cluster reset to factory defaults command
/// @param cmd_info struct that contains zcl header info
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_basic_server_reset_to_factory_defaults_cb(zb_zcl_parsed_hdr_t *cmd_info);

/// @brief Callback fires when a basic server attribute is about to be written
/// @param endpoint   device endpoint
/// @param attr_id    ZCL attribute id
/// @param new_value  pointer to the new attribute value
/// @param manuf_code manufacturer specific code
void tr_basic_server_write_attr_cb(zb_uint8_t  endpoint,
                                   zb_uint16_t attr_id,
                                   zb_uint8_t  *new_value,
                                   zb_uint16_t manuf_code);

/// @} // end of zcl_app_callbacks

/// ****************************************************************************
/// function prototypes consumed by framework
/// ****************************************************************************
void tr_basic_server_init(void);

#endif // TR_BASIC_SERVER_H
