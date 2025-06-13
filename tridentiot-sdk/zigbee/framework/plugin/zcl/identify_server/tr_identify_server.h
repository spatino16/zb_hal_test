/// ****************************************************************************
/// @file tr_identify_server.h
///
/// @brief ZCL IDENTIFY cluster server implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_IDENTIFY_SERVER_H
#define TR_IDENTIFY_SERVER_H

#include "tr_af.h"
#include "zb_zcl_identify.h"
#include "zb_aps.h"

#define GET_IDENTIFY_HANDLER(endpoint) (zb_af_get_endpoint_desc((endpoint))->identify_handler)

/// ****************************************************************************
///                                 debug prints
/// ****************************************************************************
#if defined(TR_IDENTIFY_SERVER_PLUGIN_PRINT_ENABLE) && (TR_IDENTIFY_SERVER_PLUGIN_PRINT_ENABLE == 1)
#define tr_identify_server_printf(...)  tr_zcl_printf(__VA_ARGS__)
#define tr_identify_server_println(...) tr_zcl_println(__VA_ARGS__)
#else
#define tr_identify_server_printf(...)
#define tr_identify_server_println(...)
#endif

/// ****************************************************************************
/// @defgroup zcl_identify_server_cb Identify Server Callbacks
/// @ingroup zcl_app_callbacks
/// @{
/// ****************************************************************************

/// @brief Callback fires when identify server cluster is initialized
void tr_identify_server_init_cb(void);

/// @brief Callback fires when identify behavior is started
/// @param endpoint    device endpoint
/// @param timeout_sec identify duration in seconds
void tr_identify_server_identify_start_cb(zb_uint8_t  endpoint,
                                          zb_uint16_t timeout_sec);

/// @brief Callback fires when identify behavior stops
/// @param endpoint device endpoint
void tr_identify_server_identify_stop_cb(zb_uint8_t endpoint);

/// @brief Callback fires when trigger effect command is received
/// @param invoke_data struct with zcl header info and command parameters
/// @return RET_OK if successful, else send response command with error
zb_ret_t tr_identify_server_trigger_effect_cb(zb_zcl_identify_effect_user_app_schedule_t *invoke_data);

/// @brief Callback fires when an identify server attribute is about to be written
/// @param endpoint   device endpoint
/// @param attr_id    ZCL attribute id
/// @param new_value  pointer to the new attribute value
/// @param manuf_code manufacturer specific code
void tr_identify_server_write_attr_cb(zb_uint8_t  endpoint,
                                      zb_uint16_t attr_id,
                                      zb_uint8_t  *new_value,
                                      zb_uint16_t manuf_code);

/// @} // end of zcl_app_callbacks

/// ****************************************************************************
/// function prototypes consumed by framework
/// ****************************************************************************
void tr_identify_server_init(void);
void zb_zcl_identify_effect_invoke_user_app(zb_uint8_t param);

#endif // TR_IDENTIFY_SERVER_H
