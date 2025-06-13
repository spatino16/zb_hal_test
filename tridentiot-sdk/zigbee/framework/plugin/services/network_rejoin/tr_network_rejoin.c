/// ****************************************************************************
/// @file tr_network_rejoin.c
///
/// @brief This plugin handles network rejoins for end devices, including backoffs
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_af.h"

static tr_network_rejoin_params_t g_nwk_rejoin_params;

static void tr_initiate_rejoin(zb_uint8_t param);

zb_bool_t tr_network_rejoin_backoff_active(void)
{
    return g_nwk_rejoin_params.active;
}

void tr_network_rejoin_reset_backoff_delay(void)
{
    zb_time_t timeout = 0;

    g_nwk_rejoin_params.delay_sec = g_nwk_rejoin_params.initial_delay_sec;

    if (tr_network_rejoin_backoff_active())
    {
        ZB_SCHEDULE_GET_ALARM_TIME(tr_initiate_rejoin, ZB_ALARM_ANY_PARAM, &timeout);

        if (timeout > 0)
        {
            ZB_SCHEDULE_APP_ALARM_CANCEL(tr_initiate_rejoin, ZB_ALARM_ANY_PARAM);
        }
        tr_initiate_rejoin(0);
    }
}

static void tr_initiate_rejoin(zb_uint8_t param)
{
    if (zb_zdo_is_rejoin_active())
    {
        // rejoin attempt is active, come back later
        ZB_SCHEDULE_APP_ALARM(tr_initiate_rejoin, 0, ZB_MILLISECONDS_TO_BEACON_INTERVAL(100));
        return;
    }

    switch (g_nwk_rejoin_params.rejoin_type)
    {
        case TR_REJOIN_SECURE_CURRENT_CHANNEL:
            // do a secure rejoin on the current channel
            if (tr_network_rejoin_attempt_cb(0, ZB_TRUE))
            {
                tr_network_rejoin_printf("Attempting secure rejoin on current channel\n");
                tr_network_rejoin(0, ZB_TRUE);
                // next rejoin will be unsecure on current channel
                g_nwk_rejoin_params.rejoin_type = TR_REJOIN_TC_CURRENT_CHANNEL;
            }
            break;

        case TR_REJOIN_TC_CURRENT_CHANNEL:
            // do a tc (unsecure) rejoin on the current channel
            if (tr_network_rejoin_attempt_cb(0, ZB_FALSE))
            {
                tr_network_rejoin_printf("Attempting tc (unsecure) rejoin on current channel\n");
                tr_network_rejoin(0, ZB_FALSE);
                // next rejoin will be secure on primary channels
                g_nwk_rejoin_params.rejoin_type = TR_REJOIN_SECURE_PRIMARY_CHANNELS;
            }
            break;

        case TR_REJOIN_SECURE_PRIMARY_CHANNELS:
            // do a secure rejoin on the primary channels
            if (tr_network_rejoin_attempt_cb(zb_get_bdb_primary_channel_set(), ZB_TRUE))
            {
                tr_network_rejoin_printf("Attempting secure rejoin on primary channels\n");
                tr_network_rejoin(zb_get_bdb_primary_channel_set(), ZB_TRUE);
                // next rejoin will be unsecure on secondary channels
                g_nwk_rejoin_params.rejoin_type = TR_REJOIN_TC_PRIMARY_CHANNELS;
            }
            break;

        case TR_REJOIN_TC_PRIMARY_CHANNELS:
            // do a tc (unsecure) rejoin on the primary channels
            if (tr_network_rejoin_attempt_cb(zb_get_bdb_primary_channel_set(), ZB_FALSE))
            {
                tr_network_rejoin_printf("Attempting tc (unsecure) rejoin on primary channels\n");
                tr_network_rejoin(zb_get_bdb_primary_channel_set(), ZB_FALSE);
                // next rejoin will be secure on secondary channels
                g_nwk_rejoin_params.rejoin_type = TR_REJOIN_SECURE_SECONDARY_CHANNELS;
            }
            break;

        case TR_REJOIN_SECURE_SECONDARY_CHANNELS:
            // do a secure rejoin on the secondary channels
            if (tr_network_rejoin_attempt_cb(zb_get_bdb_secondary_channel_set(), ZB_TRUE))
            {
                tr_network_rejoin_printf("Attempting secure rejoin on secondary channels\n");
                tr_network_rejoin(zb_get_bdb_secondary_channel_set(), ZB_TRUE);
                // next rejoin will be unsecure on secondary channels
                g_nwk_rejoin_params.rejoin_type = TR_REJOIN_TC_SECONDARY_CHANNELS;
            }
            break;

        case TR_REJOIN_TC_SECONDARY_CHANNELS:
            // do a tc (unsecure) rejoin on the secondary channels
            if (tr_network_rejoin_attempt_cb(zb_get_bdb_secondary_channel_set(), ZB_FALSE))
            {
                tr_network_rejoin_printf("Attempting tc (unsecure) rejoin on secondary channels\n");
                tr_network_rejoin(zb_get_bdb_secondary_channel_set(), ZB_FALSE);
                // next rejoin will be unsecure on secondary channels
                g_nwk_rejoin_params.rejoin_type = TR_REJOIN_DONE;
            }
            break;

        case TR_REJOIN_DONE:
            // all of the rejoin types have been done, stop for now
            tr_network_rejoin_printf("All rejoin types have been tried and failed\n");
            break;
    }
}

static void tr_network_rejoin_backoff_start(zb_bool_t enable)
{
    if (enable)
    {
        g_nwk_rejoin_params.active = ZB_TRUE;
        // kick off the next type of rejoin
        tr_initiate_rejoin(0);
    }
    else
    {
        g_nwk_rejoin_params.active = ZB_FALSE;
        tr_network_rejoin_reset_backoff_delay();
    }
}

static zb_time_t tr_network_rejoin_get_backoff_delay_ms(void)
{
    if (g_nwk_rejoin_params.delay_sec < g_nwk_rejoin_params.initial_delay_sec)
    {
        g_nwk_rejoin_params.delay_sec = g_nwk_rejoin_params.initial_delay_sec;
    }
    else if (g_nwk_rejoin_params.delay_sec < g_nwk_rejoin_params.max_delay_sec)
    {
        if (g_nwk_rejoin_params.max_delay_sec > g_nwk_rejoin_params.delay_sec * g_nwk_rejoin_params.delay_multiplier)
        {
            g_nwk_rejoin_params.delay_sec = g_nwk_rejoin_params.delay_sec * g_nwk_rejoin_params.delay_multiplier;
        }
        else
        {
            g_nwk_rejoin_params.delay_sec = g_nwk_rejoin_params.max_delay_sec;
        }
    }
    else
    {
        g_nwk_rejoin_params.delay_sec = g_nwk_rejoin_params.max_delay_sec;
    }

    tr_network_rejoin_printf("NWK Rejoin Backoff Delay: %d seconds\n", g_nwk_rejoin_params.delay_sec);

    return ZB_SECONDS_TO_BEACON_INTERVAL(g_nwk_rejoin_params.delay_sec);
}

void tr_network_rejoin_init(void)
{
    // configure nwk rejoin backoff
    memset(&g_nwk_rejoin_params, 0, sizeof(tr_network_rejoin_params_t));
    g_nwk_rejoin_params.delay_multiplier  = TR_NWK_REJOIN_DELAY_MULTIPLIER;
    g_nwk_rejoin_params.initial_delay_sec = TR_NWK_REJOIN_INITIAL_DELAY_SEC;
    g_nwk_rejoin_params.max_delay_sec     = TR_NWK_REJOIN_MAX_DELAY_SEC;
    g_nwk_rejoin_params.rejoin_type       = TR_REJOIN_SECURE_CURRENT_CHANNEL;
}

void tr_network_rejoin_connection_state_cb(tr_conn_state_e conn_state)
{
    switch (conn_state)
    {
        case TR_CONN_STATE_NWK_STEERING_ATTEMPT_SUCCESS:
        case TR_CONN_STATE_JOINED_NETWORK:
            tr_network_rejoin_printf("TR_CONN_STATE_JOINED_NETWORK\n");

            // we have joined, reset the rejoin method to be secure on the current channel
            g_nwk_rejoin_params.rejoin_type = TR_REJOIN_SECURE_CURRENT_CHANNEL;

            // stop the rejoin backoff if active
            if (tr_network_rejoin_backoff_active())
            {
                tr_network_rejoin_backoff_start(ZB_FALSE);
            }
            else
            {
                // if not network rejoin, then probably joining for the first time
            }
            break;

        case TR_CONN_STATE_JOINED_NETWORK_NO_PARENT:
            tr_network_rejoin_printf("TR_CONN_STATE_JOINED_NETWORK_NO_PARENT\n");

            if (!tr_network_rejoin_backoff_active())
            {
                tr_network_rejoin_backoff_start(ZB_TRUE);
            }
            else
            {
                zb_time_t timeout = 0;
                ZB_SCHEDULE_GET_ALARM_TIME(tr_initiate_rejoin, ZB_ALARM_ANY_PARAM, &timeout);

                if (timeout == 0)
                {
                    if (TR_REJOIN_DONE != g_nwk_rejoin_params.rejoin_type)
                    {
                        // we have not cycled through all rejoin types yet, do the next rejoin NOW!
                        tr_initiate_rejoin(0);
                    }
                    else
                    {
                        // reset the rejoin type
                        g_nwk_rejoin_params.rejoin_type = TR_REJOIN_SECURE_CURRENT_CHANNEL;
                        ZB_SCHEDULE_APP_ALARM(tr_initiate_rejoin, 0, tr_network_rejoin_get_backoff_delay_ms());
                    }
                }
            }
            break;

        case TR_CONN_STATE_UNKNOWN:
        default:
            break;
    }
}
