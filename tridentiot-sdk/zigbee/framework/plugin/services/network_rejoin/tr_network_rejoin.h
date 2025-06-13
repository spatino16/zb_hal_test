/// ****************************************************************************
/// @file tr_network_rejoin.h
///
/// @brief This plugin handles network rejoins for end devices, including backoffs
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_NETWORK_REJOIN_H
#define TR_NETWORK_REJOIN_H

#include "tr_af.h" // TODO LCD 3/11/25 this shouldn't be here, but include order from tr_zcl_endpoint_config.c requires it

#ifndef TR_NWK_REJOIN_DELAY_MULTIPLIER
#define TR_NWK_REJOIN_DELAY_MULTIPLIER 2
#endif

#ifndef TR_NWK_REJOIN_INITIAL_DELAY_SEC
#define TR_NWK_REJOIN_INITIAL_DELAY_SEC 1
#endif

#ifndef TR_NWK_REJOIN_MAX_DELAY_SEC
#define TR_NWK_REJOIN_MAX_DELAY_SEC 900
#endif

typedef enum
{
    TR_REJOIN_SECURE_CURRENT_CHANNEL,
    TR_REJOIN_TC_CURRENT_CHANNEL,
    TR_REJOIN_SECURE_PRIMARY_CHANNELS,
    TR_REJOIN_TC_PRIMARY_CHANNELS,
    TR_REJOIN_SECURE_SECONDARY_CHANNELS,
    TR_REJOIN_TC_SECONDARY_CHANNELS,
    TR_REJOIN_DONE
}tr_network_rejoin_types_t;

typedef struct
{
    zb_bool_t                 active;
    zb_uint8_t                delay_multiplier;
    zb_uint32_t               initial_delay_sec;
    zb_uint32_t               delay_sec;
    zb_uint32_t               max_delay_sec;
    tr_network_rejoin_types_t rejoin_type;
}tr_network_rejoin_params_t;

/// ****************************************************************************
///                                 debug prints
/// ****************************************************************************
#if defined(TR_NETWORK_REJOIN_PLUGIN_PRINT_ENABLE) && (TR_NETWORK_REJOIN_PLUGIN_PRINT_ENABLE == 1)
#define tr_network_rejoin_printf(...)  tr_zcl_printf(__VA_ARGS__)
#define tr_network_rejoin_println(...) tr_zcl_println(__VA_ARGS__)
#else
#define tr_network_rejoin_printf(...)
#define tr_network_rejoin_println(...)
#endif

/// ****************************************************************************
/// @defgroup zb_network_rejoin_cb Network Rejoin Callbacks
/// @ingroup services_app_callbacks
/// @{
/// ****************************************************************************

/// @brief User callback to indicate a rejoin attempt is about to be made
/// @param channel_mask channel(s) to be used for the rejoin
/// @param secure ZB_TRUE for secure rejoin attempt, ZB_FALSE for tc (unsecure) rejoin
/// @return ZB_TRUE to allow rejoin attemp to be made, ZB_FALSE to stop it
zb_bool_t tr_network_rejoin_attempt_cb(zb_uint32_t channel_mask,
                                       zb_bool_t   secure);

/// @} // end of services_app_callbacks

/// ****************************************************************************
/// @defgroup services_api_network_rejoin Network Rejoin API References
/// @ingroup services_api_references
/// @{
/// ****************************************************************************

/// @brief API to check to see if the rejoin backoff is currently active
/// @return ZB_TRUE if it is active, ZB_FALSE if not
zb_bool_t tr_network_rejoin_backoff_active(void);

/// @brief API to reset the rejoin backoff delay to the starting value
void tr_network_rejoin_reset_backoff_delay(void);

/// @} // end of services_api_references

/// ****************************************************************************
/// function prototypes consumed by framework
/// ****************************************************************************
void tr_network_rejoin_init(void);
void tr_network_rejoin_connection_state_cb(tr_conn_state_e conn_state);

#endif // ifndef TR_NETWORK_REJOIN_H
