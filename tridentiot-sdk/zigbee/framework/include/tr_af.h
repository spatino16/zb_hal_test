/// ****************************************************************************
/// @file tr_af.h
///
/// @brief Trident application framework include
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_AF_H
#define TR_AF_H
#include "zb_common.h"
#include "tr_debug_print.h"
#include "tr_zcl_common.h"
#include "tr_mfg_tokens.h"

typedef enum
{
    /* Zigbee framework running and device is not part of a network */
    TR_CONN_STATE_NO_NETWORK,
    /* BDB NWK Steering attempt completed successfully --> NWK Join successful */
    TR_CONN_STATE_NWK_STEERING_ATTEMPT_SUCCESS,
    /* BDB NWK Steering attempt completed unsuccessfully --> NWK Join fail */
    TR_CONN_STATE_NWK_STEERING_ATTEMPT_FAILURE,
    /* Fires when devices joins previously known network */
    TR_CONN_STATE_JOINED_NETWORK,
    /* Fires when devices recognizes that the network parent is gone */
    TR_CONN_STATE_JOINED_NETWORK_NO_PARENT,
    /* Unknown state */
    TR_CONN_STATE_UNKNOWN = 0xFF
}tr_conn_state_e;

#include "tr_zcl_endpoint_config.h"
#ifdef TR_IS_SLEEPY_ZED
#include "tr_sleep.h"
#endif

#ifdef TR_NETWORK_REJOIN_PLUGIN_ENABLE
#include "tr_network_rejoin.h"
#endif

#ifndef TR_PRIMARY_CHANNEL_MASK
#define TR_PRIMARY_CHANNEL_MASK 0x02108800
#endif
#ifndef TR_SECONDARY_CHANNEL_MASK
#define TR_SECONDARY_CHANNEL_MASK 0x05EF7000
#endif

#ifdef ZB_ED_FUNC
#ifndef TR_ED_AGING_TIMEOUT
#define TR_ED_AGING_TIMEOUT ED_AGING_TIMEOUT_64MIN
#endif
#ifndef TR_KEEPALIVE_INTERVAL_MS
#define TR_KEEPALIVE_INTERVAL_MS 300000
#endif
#ifndef TR_KEEPALIVE_METHOD
#define TR_KEEPALIVE_METHOD ED_KEEPALIVE_DISABLED
#endif
#endif


/// ****************************************************************************
/// @defgroup zb_app_framework_cb Application Framework Callbacks
/// @ingroup common_app_callbacks
/// @{
/// ****************************************************************************

/// @brief Callback that the user can declare to handle external attribute storage
/// @param endpoint
/// @param cluster_id
/// @param cluster_role
/// @param attr_id
/// @param manuf_code
/// @return pointer to data if it is handled by the application, NULL otherwise
zb_uint8_t *tr_zcl_external_attribute_read_cb(zb_uint8_t  endpoint,
                                              zb_uint16_t cluster_id,
                                              zb_uint8_t  cluster_role,
                                              zb_uint16_t attr_id,
                                              zb_uint16_t manuf_code);

/// @brief Callback that fires when ZBOSS framework (scheduler, buffer pool, etc.)
/// has started, but no join / rejoin / formation / BDB initialization has been done yet.
void tr_app_init_cb(void);

/// @brief Callback that fires when network connection state changes
/// @param conn_state connection state defined in typedef enum tr_conn_state_e
void tr_connection_state_cb(tr_conn_state_e conn_state);

/// @brief Callback that can be defined in the user application to set the EUI64
/// @param eui64 pointer to 8 byte buffer that contains the user EUI64
/// @return ZB_TRUE to override the eui64, ZB_FALSE to ignore this callback
zb_bool_t tr_eui64_user_override_cb(zb_uint8_t *eui64);

/// @} // end of common_app_callbacks

/// ****************************************************************************
/// @defgroup common_api_app_framework Application Framework API Reference
/// @ingroup common_api_references
/// @{
/// ****************************************************************************

/// @brief Initialize Trident application framework
void tr_af_init(void);

/// @brief Function for getting the last known connection state of the device. Optional
/// callback tr_connection_state_cb() is fired when state changes.
/// @return Last known connection state passed into tr_connection_state_cb()
tr_conn_state_e tr_get_connection_state(void);

/// @brief Function for initiating network rejoin attempts
/// @param channel_mask channel mask used for the rejoin
/// @param secure true for secure rejoin, false for trust center (unsecure) rejoin
void tr_network_rejoin(zb_uint32_t channel_mask,
                       zb_bool_t   secure);

/// @} // end of common_api_references

#endif /* TR_AF_H */
