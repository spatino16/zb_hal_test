/// ****************************************************************************
/// @file tr_af.c
///
/// @brief Trident application framework
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_af.h"

static tr_conn_state_e g_conn_state = TR_CONN_STATE_UNKNOWN;

static void tr_set_connection_state(tr_conn_state_e conn_state);
static void tr_nlme_status_indication_handler(zb_zdo_signal_nlme_status_indication_params_t *nlme_params);
static void tr_initiate_secure_nwk_rejoin(zb_uint8_t param);

static zb_bool_t g_tr_install_code_valid = ZB_FALSE;

// inits the zigbee application, does not include hardware related things
void tr_af_init(void)
{
    tr_mfg_tok_type_cca_threshold cca_threshold;

    // Process the cca token before we init
    // get the token value
    tr_get_mfg_token(&cca_threshold, TR_MFG_TOKEN_CCA_THRESHOLD);
    tr_set_cca_threshold_value((zb_uint8_t)cca_threshold.value);

    // Global ZBOSS initialization
    ZB_INIT("trident iot");

    // allow written tokens to take effect
    tr_mfg_tokens_process();

    // set device behavior (rx on (non-sleepy) or off (sleepy) when idle)
#ifdef TR_IS_SLEEPY_ZED
    zb_set_rx_on_when_idle(ZB_FALSE);
#else
    zb_set_rx_on_when_idle(ZB_TRUE);
#endif

    // set stack behavior
#ifdef TR_ZIGBEE_R22
    zboss_use_r22_behavior();
#else
#ifdef TR_DONT_USE_ECDHE_CURVE_25519_HASH_SHA256
    zb_disable_key_neg_method(ZB_TLV_KEY_ECDHE_CURVE_25519_HASH_SHA256);
#endif /* TR_DONT_USE_ECDHE_CURVE_25519_HASH_SHA256 */

#ifdef TR_DONT_USE_ECDHE_CURVE_25519_HASH_AESMMO128
    zb_disable_key_neg_method(ZB_TLV_KEY_ECDHE_CURVE_25519_HASH_AESMMO128);
#endif /* TR_DONT_USE_ECDHE_CURVE_25519_HASH_AESMMO128 */
#endif /* TR_ZIGBEE_R22 */
}

tr_conn_state_e tr_get_connection_state(void)
{
    if (g_conn_state == TR_CONN_STATE_UNKNOWN)
    {
        if (zb_bdb_is_factory_new())
        {
            g_conn_state = TR_CONN_STATE_NO_NETWORK;
        }
        else
        {
            g_conn_state = TR_CONN_STATE_JOINED_NETWORK_NO_PARENT;
        }
    }

    if (g_conn_state == TR_CONN_STATE_NWK_STEERING_ATTEMPT_SUCCESS)
    {
        g_conn_state = TR_CONN_STATE_JOINED_NETWORK;
    }

    return g_conn_state;
}

void tr_network_rejoin(zb_uint32_t channel_mask,
                       zb_bool_t   secure)
{
    if (channel_mask == 0)
    {
        zb_aib_channel_page_list_set_2_4GHz_mask((1 << zb_get_current_channel()));
    }
    else
    {
        zb_aib_channel_page_list_set_2_4GHz_mask(channel_mask);
    }

    if (secure)
    {
        // initiate a secure rejoin
        tr_initiate_secure_nwk_rejoin(0);
    }
    else
    {
        // initiate a trust center rejoin
        zb_bdb_initiate_tc_rejoin(0);
    }
}

// Signal handler required by zboss stack
void zboss_signal_handler(zb_uint8_t param)
{
    zb_ret_t                          status    = ZB_GET_APP_SIGNAL_STATUS(param);
    zb_ret_t                          ic_status = -1;
    zb_zdo_app_signal_hdr_t           *sg_p     = NULL;
    zb_zdo_app_signal_type_t          sig       = zb_get_app_signal(param, &sg_p);
    tr_mfg_tok_type_installation_code install_code;
    zb_uint8_t                        ic_array[20];
    zb_uint8_t                        x;
#ifdef TR_ENABLE_JOINING_DISTRIBUTED_NETWORKS
    tr_mfg_tok_type_distributed_key dist_nwk_key;
#endif

    // skip print if we are trying to sleep, revisit this when implementing sleep functionality
    if (sig != ZB_COMMON_SIGNAL_CAN_SLEEP)
    {
        tr_app_printf("\nzboss_signal_handler - STATUS: %d, STATE: %d\n", status, sig);
    }

    switch (sig)
    {
        case ZB_ZDO_SIGNAL_DEFAULT_START:
            // NOTE: not using this since we want to control when comissioning begins
            break; /* ZB_ZDO_SIGNAL_DEFAULT_START */

        case ZB_ZDO_SIGNAL_SKIP_STARTUP:
            if (status == RET_OK)
            {
                tr_app_init_cb();
#ifdef TR_NETWORK_REJOIN_PLUGIN_ENABLE
                tr_network_rejoin_init();
#endif

                // TODO: install code setup goes here --> zb_secur_ic_str_set(g_installcode);
                tr_get_mfg_token(&install_code, TR_MFG_TOKEN_INSTALLATION_CODE);

                if (install_code.flags != 0xFFFF)
                {
                    tr_app_printf("Install Code:\n", install_code.flags);
                    tr_app_printf("   flags: 0x%4.4x\n", install_code.flags);
                    tr_app_printf("   code:  0x");

                    for (x = 0 ; x < 16 ; x++)
                    {
                        tr_app_printf("%2.2x", install_code.value[x]);
                    }
                    tr_app_printf("\n   crc:   0x%4.4x\n", install_code.crc);

                    switch (install_code.flags & 0x0F)
                    {
                        case 0x6:
                            memcpy(ic_array, install_code.value, 6);
                            *(zb_uint16_t*)&ic_array[6] = install_code.crc;
                            ic_status                   = zb_secur_ic_set(ZB_IC_TYPE_48, ic_array);
                            break;

                        case 0x8:
                            memcpy(ic_array, install_code.value, 8);
                            *(zb_uint16_t*)&ic_array[8] = install_code.crc;
                            ic_status                   = zb_secur_ic_set(ZB_IC_TYPE_64, ic_array);
                            break;

                        case 0xc:
                            memcpy(ic_array, install_code.value, 12);
                            *(zb_uint16_t*)&ic_array[12] = install_code.crc;
                            ic_status                    = zb_secur_ic_set(ZB_IC_TYPE_96, ic_array);
                            break;

                        case 0xf:
                            memcpy(ic_array, install_code.value, 16);
                            *(zb_uint16_t*)&ic_array[16] = install_code.crc;
                            ic_status                    = zb_secur_ic_set(ZB_IC_TYPE_128, ic_array);
                            break;

                        default:
                            tr_app_printf("   Install code flags invalid\n");
                            break;
                    }

                    if (ic_status == 0)
                    {
                        tr_app_printf("   Install code configured from token\n");
                        g_tr_install_code_valid = ZB_TRUE;
                    }
                    else
                    {
                        tr_app_printf("   Install code token invalid\n");
                        g_tr_install_code_valid = ZB_FALSE;
                    }
                }

#ifdef TR_ENABLE_JOINING_DISTRIBUTED_NETWORKS

                // configure the distributed key if there is one. If not the stack will use the default distributed key
                if (!tr_mfg_token_check_erased(TR_MFG_TOKEN_DISTRIBUTED_KEY))
                {
                    tr_get_mfg_token(&dist_nwk_key, TR_MFG_TOKEN_DISTRIBUTED_KEY);
                    zb_zdo_set_tc_standard_distributed_key(dist_nwk_key);
                    tr_app_printf("Distributed key: 0x");

                    for (x = 0 ; x < sizeof(dist_nwk_key) ; x++)
                    {
                        tr_app_printf("%2.2x", dist_nwk_key[x]);
                    }
                    tr_app_printf("\n");
                }
                zb_enable_joining_to_distributed_network();
#endif /* ifdef TR_ENABLE_JOINING_DISTRIBUTED_NETWORKS */

                if (zb_bdb_is_factory_new())
                {
                    tr_set_connection_state(TR_CONN_STATE_NO_NETWORK);

                    // NOTE: do not want to run zboss_start_continue() here because it will cause device
                    // to join open network automatically
                }
                else
                {
                    tr_set_connection_state(TR_CONN_STATE_JOINED_NETWORK_NO_PARENT);
                }
                // NOTE: thinking if/when we make a coordinator device type then we should make a tr_af_zc.c file to deal with that
            }
            break; /* ZB_ZDO_SIGNAL_SKIP_STARTUP */

        case ZB_ZDO_SIGNAL_DEVICE_ANNCE:
            if (status == RET_OK)
            {
                // TODO: hit when device announce command is received
            }
            break; /* ZB_ZDO_SIGNAL_DEVICE_ANNCE */

        case ZB_ZDO_SIGNAL_LEAVE:
            if (status == RET_OK)
            {
                zb_zdo_signal_leave_params_t *leave_params = ZB_ZDO_SIGNAL_GET_PARAMS(sg_p, zb_zdo_signal_leave_params_t);

                if (leave_params->leave_type == ZB_NWK_LEAVE_TYPE_RESET)
                {
                    tr_set_connection_state(TR_CONN_STATE_NO_NETWORK);
                }
                else if (leave_params->leave_type == ZB_NWK_LEAVE_TYPE_REJOIN)
                {
                    // NOTE: goes to ZB_BDB_SIGNAL_DEVICE_REBOOT state immediately after this.
                    // Connection state is updated there
                    tr_app_printf("LEAVE WITH REJOIN\n");
                }
            }
            break; /* ZB_ZDO_SIGNAL_LEAVE */

        case ZB_ZDO_SIGNAL_ERROR:
            // hit when incorrect buffer length detected by zb_get_app_signal
            break; /* ZB_ZDO_SIGNAL_ERROR */

        case ZB_BDB_SIGNAL_DEVICE_FIRST_START:
            // device started for first time after an NVRAM erase
            // currently not a very interesting state since we are not starting nwk steering automatically
            break; /* ZB_BDB_SIGNAL_DEVICE_FIRST_START */

        case ZB_BDB_SIGNAL_DEVICE_REBOOT:
            // runs when nwk rejoin attempt is made on device reboot
            // status RET_OK if successful rejoin using NWK information in NVRAM
            // status ERROR otherwise

            if (status != RET_OK)
            {
                if (zb_bdb_is_factory_new())
                {
                    /* Device tried to perform TC rejoin after reboot and lost its authentication flag.
                     * Do nothing here and wait for ZB_BDB_SIGNAL_TC_REJOIN_DONE to handle TC rejoin error */
                    tr_app_printf("Device lost authentication flag\n");
                }
                else
                {
                    tr_set_connection_state(TR_CONN_STATE_JOINED_NETWORK_NO_PARENT);
                }
            }
            else
            {
                tr_set_connection_state(TR_CONN_STATE_JOINED_NETWORK);
            }
            break; /* ZB_BDB_SIGNAL_DEVICE_REBOOT */

#ifdef ZB_ENABLE_ZLL
        case ZB_BDB_SIGNAL_TOUCHLINK_NWK_STARTED:
            break; /* ZB_BDB_SIGNAL_TOUCHLINK_NWK_STARTED */

        case ZB_BDB_SIGNAL_TOUCHLINK_NWK_JOINED_ROUTER:
            break; /* ZB_BDB_SIGNAL_TOUCHLINK_NWK_JOINED_ROUTER */

        case ZB_BDB_SIGNAL_TOUCHLINK:
            break; /* ZB_BDB_SIGNAL_TOUCHLINK */

        case ZB_BDB_SIGNAL_TOUCHLINK_TARGET:
            break; /* ZB_BDB_SIGNAL_TOUCHLINK_TARGET */

        case ZB_BDB_SIGNAL_TOUCHLINK_NWK:
            break; /* ZB_BDB_SIGNAL_TOUCHLINK_NWK */

        case ZB_BDB_SIGNAL_TOUCHLINK_TARGET_FINISHED:
            break; /* ZB_BDB_SIGNAL_TOUCHLINK_TARGET_FINISHED */

        case ZB_BDB_SIGNAL_TOUCHLINK_ADD_DEVICE_TO_NWK:
            break; /* ZB_BDB_SIGNAL_TOUCHLINK_ADD_DEVICE_TO_NWK */
#endif         /* ZB_ENABLE_ZLL */

        case ZB_BDB_SIGNAL_STEERING:
            if (status == RET_OK)
            {
                tr_set_connection_state(TR_CONN_STATE_NWK_STEERING_ATTEMPT_SUCCESS);
            }
            else
            {
                // is this device using an install code?
                // if (g_tr_install_code_valid)
                // {
                //     // NWK steering attempt failed, was it with an install code?
                //     tr_app_printf("   Supported psk secrets 0x%x\n", zb_get_supported_psk_secrets());

                //     // if (zb_get_supported_psk_secrets() & (0x01 << ZB_TLV_PSK_SECRET_INSTALL_CODE))
                //     if (zb_get_supported_psk_secrets() != ZB_TLV_PSK_SECRET_WELL_KNOWN_KEY)
                //     {
                //         // install code was enabled, try again without install code
                //         zb_disable_psk_secret(ZB_TLV_PSK_SECRET_INSTALL_CODE);
                //         tr_app_printf("   disable install code\n");
                //         bdb_start_top_level_commissioning(ZB_BDB_NETWORK_STEERING);
                //         return;
                //     }
                //     else
                //     {
                //         // reenable the install code
                //         zb_enable_psk_secret(ZB_TLV_PSK_SECRET_INSTALL_CODE);
                //     }
                // }
                tr_set_connection_state(TR_CONN_STATE_NWK_STEERING_ATTEMPT_FAILURE);
            }
            break; /* ZB_BDB_SIGNAL_STEERING */

        case ZB_BDB_SIGNAL_FORMATION:
            break; /* ZB_BDB_SIGNAL_FORMATION */

        case ZB_BDB_SIGNAL_FINDING_AND_BINDING_TARGET_FINISHED:
            break; /* ZB_BDB_SIGNAL_FINDING_AND_BINDING_TARGET_FINISHED */

        case ZB_BDB_SIGNAL_FINDING_AND_BINDING_INITIATOR_FINISHED:
            break; /* ZB_BDB_SIGNAL_FINDING_AND_BINDING_INITIATOR_FINISHED */

        case ZB_NWK_SIGNAL_DEVICE_ASSOCIATED:
            break; /* ZB_NWK_SIGNAL_DEVICE_ASSOCIATED */

        case ZB_ZDO_SIGNAL_LEAVE_INDICATION:
            break; /* ZB_ZDO_SIGNAL_LEAVE_INDICATION */

        case ZB_BDB_SIGNAL_WWAH_REJOIN_STARTED:
            /* also case for ZB_SE_SIGNAL_REJOIN_STARTED */
            break; /* ZB_BDB_SIGNAL_WWAH_REJOIN_STARTED */

        case ZB_ZGP_SIGNAL_COMMISSIONING:
            break; /* ZB_ZGP_SIGNAL_COMMISSIONING */

        case ZB_COMMON_SIGNAL_CAN_SLEEP:
#ifdef TR_IS_SLEEPY_ZED

            // only do this if the device is sleepy
            if (!zb_get_rx_on_when_idle())
            {
                if (status == RET_OK)
                {
                    zb_zdo_signal_can_sleep_params_t *can_sleep_params = ZB_ZDO_SIGNAL_GET_PARAMS(sg_p, zb_zdo_signal_can_sleep_params_t);

                    if (tr_check_for_sleep(can_sleep_params->sleep_tmo))
                    {
                        zb_sleep_now();
                        tr_post_wake_cb();
                    }
                }
            }
#endif /* ifdef TR_IS_SLEEPY_ZED */
            break; /* ZB_COMMON_SIGNAL_CAN_SLEEP */

        case ZB_ZDO_SIGNAL_PRODUCTION_CONFIG_READY:
            break; /* ZB_ZDO_SIGNAL_PRODUCTION_CONFIG_READY */

        case ZB_NWK_SIGNAL_NO_ACTIVE_LINKS_LEFT:
            break; /* ZB_NWK_SIGNAL_NO_ACTIVE_LINKS_LEFT */

#if defined ZB_ENABLE_SE_MIN_CONFIG || defined DOXYGEN
        case ZB_SE_SIGNAL_SKIP_JOIN:
            break; /* ZB_SE_SIGNAL_SKIP_JOIN */

        case ZB_SE_SIGNAL_REJOIN:
            break; /* ZB_SE_SIGNAL_REJOIN */

        case ZB_SE_SIGNAL_CHILD_REJOIN:
            break; /* ZB_SE_SIGNAL_CHILD_REJOIN */

        case ZB_SE_TC_SIGNAL_CHILD_JOIN_CBKE:
            break; /* ZB_SE_TC_SIGNAL_CHILD_JOIN_CBKE */

        case ZB_SE_TC_SIGNAL_CHILD_JOIN_NON_CBKE:
            break; /* ZB_SE_TC_SIGNAL_CHILD_JOIN_NON_CBKE */

        case ZB_SE_SIGNAL_CBKE_FAILED:
            break; /* ZB_SE_SIGNAL_CBKE_FAILED */

        case ZB_SE_SIGNAL_CBKE_OK:
            break; /* ZB_SE_SIGNAL_CBKE_OK */

        case ZB_SE_SIGNAL_SERVICE_DISCOVERY_START:
            break; /* ZB_SE_SIGNAL_SERVICE_DISCOVERY_START */

        case ZB_SE_SIGNAL_SERVICE_DISCOVERY_DO_BIND:
            break; /* ZB_SE_SIGNAL_SERVICE_DISCOVERY_DO_BIND */

        case ZB_SE_SIGNAL_SERVICE_DISCOVERY_BIND_OK:
            break; /* ZB_SE_SIGNAL_SERVICE_DISCOVERY_BIND_OK */

        case ZB_SE_SIGNAL_SERVICE_DISCOVERY_BIND_FAILED:
            break; /* ZB_SE_SIGNAL_SERVICE_DISCOVERY_BIND_FAILED */

        case ZB_SE_SIGNAL_SERVICE_DISCOVERY_BIND_INDICATION:
            break; /* ZB_SE_SIGNAL_SERVICE_DISCOVERY_BIND_INDICATION */

        case ZB_SE_SIGNAL_SERVICE_DISCOVERY_OK:
            break; /* ZB_SE_SIGNAL_SERVICE_DISCOVERY_OK */

        case ZB_SE_SIGNAL_SERVICE_DISCOVERY_FAILED:
            break; /* ZB_SE_SIGNAL_SERVICE_DISCOVERY_FAILED */

        case ZB_SE_SIGNAL_APS_KEY_READY:
            break; /* ZB_SE_SIGNAL_APS_KEY_READY */

        case ZB_SE_SIGNAL_APS_KEY_FAIL:
            break; /* ZB_SE_SIGNAL_APS_KEY_FAIL */
#endif         /* ZB_ENABLE_SE_MIN_CONFIG */

        case ZB_SIGNAL_SUBGHZ_SUSPEND:
            break; /* ZB_SIGNAL_SUBGHZ_SUSPEND */

        case ZB_SIGNAL_SUBGHZ_RESUME:
            break; /* ZB_SIGNAL_SUBGHZ_RESUME */

#ifdef ZB_MACSPLIT
        case ZB_MACSPLIT_DEVICE_BOOT:
            break; /* ZB_MACSPLIT_DEVICE_BOOT */

        case ZB_MACSPLIT_DEVICE_READY_FOR_UPGRADE:
            break; /* ZB_MACSPLIT_DEVICE_READY_FOR_UPGRADE */

        case ZB_MACSPLIT_DEVICE_FW_UPGRADE_EVENT:
            break; /* ZB_MACSPLIT_DEVICE_FW_UPGRADE_EVENT */
#endif         /* ZB_MACSPLIT */

#ifdef NCP_MODE
        case ZB_SIGNAL_NWK_INIT_DONE:
            break; /* ZB_SIGNAL_NWK_INIT_DONE */
#endif         /* NCP_MODE */

        case ZB_ZDO_SIGNAL_DEVICE_AUTHORIZED:
            break; /* ZB_ZDO_SIGNAL_DEVICE_AUTHORIZED */

        case ZB_ZDO_SIGNAL_DEVICE_UPDATE:
            break; /* ZB_ZDO_SIGNAL_DEVICE_UPDATE */

        case ZB_ZDO_SIGNAL_DEVICE_READY_FOR_INTERVIEW:
            break; /* ZB_ZDO_SIGNAL_DEVICE_READY_FOR_INTERVIEW */

        case ZB_ZDO_SIGNAL_DEVICE_INTERVIEW_STARTED:
            break; /* ZB_ZDO_SIGNAL_DEVICE_INTERVIEW_STARTED */

        case ZB_NWK_SIGNAL_PANID_CONFLICT_DETECTED:
            break; /* ZB_NWK_SIGNAL_PANID_CONFLICT_DETECTED */

        case ZB_NLME_STATUS_INDICATION:
            if (status == RET_OK)
            {
                zb_zdo_signal_nlme_status_indication_params_t *nlme_params = ZB_ZDO_SIGNAL_GET_PARAMS(sg_p,
                                                                                                      zb_zdo_signal_nlme_status_indication_params_t);
                tr_nlme_status_indication_handler(nlme_params);
            }
            break; /* ZB_NLME_STATUS_INDICATION */

        case ZB_TCSWAP_DB_BACKUP_REQUIRED_SIGNAL:
            break; /* ZB_TCSWAP_DB_BACKUP_REQUIRED_SIGNAL */

        case ZB_TC_SWAPPED_SIGNAL:
            break; /* ZB_TC_SWAPPED_SIGNAL */

        case ZB_TCLK_UPDATED_SIGNAL:
            break; /* ZB_TCLK_UPDATED_SIGNAL */

        case ZB_SIGNAL_JOIN_DONE:
            break; /* ZB_SIGNAL_JOIN_DONE */

        case ZB_BUFFER_TEST_REQ_SIGNAL:
            break; /* ZB_BUFFER_TEST_REQ_SIGNAL */

        case ZB_BDB_SIGNAL_TC_REJOIN_DONE:
            // runs when nwk rejoin attempt completes. Successful or not.
            // does NOT run when first rejoin attempt made after boot is successful (see ZB_BDB_SIGNAL_DEVICE_REBOOT)
            if (status != RET_OK)
            {
                tr_set_connection_state(TR_CONN_STATE_JOINED_NETWORK_NO_PARENT);
            }
            else
            {
                tr_set_connection_state(TR_CONN_STATE_JOINED_NETWORK);
            }
            break; /* ZB_BDB_SIGNAL_TC_REJOIN_DONE */

        case ZB_NWK_SIGNAL_PERMIT_JOIN_STATUS:
            break; /* ZB_NWK_SIGNAL_PERMIT_JOIN_STATUS */

        case ZB_ZDO_SIGNAL_DEVICE_INTERVIEW_FINISHED:
            break; /* ZB_ZDO_SIGNAL_DEVICE_INTERVIEW_FINISHED */

        case ZB_BDB_SIGNAL_STEERING_CANCELLED:
            break; /* ZB_BDB_SIGNAL_STEERING_CANCELLED */

        case ZB_BDB_SIGNAL_FORMATION_CANCELLED:
            break; /* ZB_BDB_SIGNAL_FORMATION_CANCELLED */

        case ZB_SIGNAL_READY_TO_SHUT:
            break; /* ZB_SIGNAL_READY_TO_SHUT */

        case ZB_SIGNAL_INTERPAN_PREINIT:
            break; /* ZB_SIGNAL_INTERPAN_PREINIT */

        case ZB_ZGP_SIGNAL_MODE_CHANGE:
            break; /* ZB_ZGP_SIGNAL_MODE_CHANGE */

        case ZB_ZDO_DEVICE_UNAVAILABLE:
            // Could not send a packet. Could be caused by:
            // - no MAC ACK
            // - no APS ACK
            // - no response to a nwk address request

            // passes in: zb_zdo_device_unavailable_params_t
            // TODO: message send failure callback
            break; /* ZB_ZDO_DEVICE_UNAVAILABLE */

        case ZB_ZGP_SIGNAL_APPROVE_COMMISSIONING:
            break; /* ZB_ZGP_SIGNAL_APPROVE_COMMISSIONING */

        case ZB_DEBUG_SIGNAL_TCLK_READY:
            break; /* ZB_DEBUG_SIGNAL_TCLK_READY */

        default:
            break;
    }

    if (param)
    {
        zb_buf_free(param);
    }
}

/************************************************************************************/
/**                            Weak Callback Definitions                           **/
/************************************************************************************/

ZB_WEAK zb_bool_t tr_eui64_user_override_cb(zb_uint8_t *eui64)
{
    ZVUNUSED(*eui64);
    return ZB_FALSE;
}

/************************************************************************************/
/**                                Private Functions                               **/
/************************************************************************************/

static void tr_set_connection_state(tr_conn_state_e conn_state)
{
    g_conn_state = conn_state;

#ifdef TR_OVER_THE_AIR_BOOTLOADING_CLIENT_PLUGIN_ENABLE
    tr_over_the_air_bootloading_client_connection_state_cb(conn_state);
#endif
#ifdef TR_POLL_CONTROL_SERVER_PLUGIN_ENABLE
    tr_poll_control_server_connection_state_cb(conn_state);
#endif
#ifdef TR_NETWORK_REJOIN_PLUGIN_ENABLE
    tr_network_rejoin_connection_state_cb(conn_state);
#endif
    tr_connection_state_cb(g_conn_state);
}

static void tr_nlme_status_indication_handler(zb_zdo_signal_nlme_status_indication_params_t *nlme_params)
{
    // TODO: remove debug prints
    tr_app_printf("ZB_NLME_STATUS_INDICATION\n");
    tr_app_printf(" - status: 0x%02X\n", nlme_params->nlme_status.status);
    tr_app_printf(" - NWK Address: 0x%04X\n", nlme_params->nlme_status.network_addr);
    tr_app_printf(" - unknown cmd id: 0x%02X\n", nlme_params->nlme_status.unknown_command_id);

    // TODO: should we have an application callback for this?

    // see nwk_command_states in zboss_api_nwk.h for state information
    switch (nlme_params->nlme_status.status)
    {
        case ZB_NWK_COMMAND_STATUS_NO_ROUTE_AVAILABLE:
            break; /* ZB_NWK_COMMAND_STATUS_NO_ROUTE_AVAILABLE */

        case ZB_NWK_COMMAND_STATUS_TREE_LINK_FAILURE:
            break; /* ZB_NWK_COMMAND_STATUS_TREE_LINK_FAILURE */

        case ZB_NWK_COMMAND_STATUS_NONE_TREE_LINK_FAILURE:
            break; /* ZB_NWK_COMMAND_STATUS_NONE_TREE_LINK_FAILURE */

        case ZB_NWK_COMMAND_STATUS_LOW_BATTERY_LEVEL:
            break; /* ZB_NWK_COMMAND_STATUS_LOW_BATTERY_LEVEL */

        case ZB_NWK_COMMAND_STATUS_NO_ROUTING_CAPACITY:
            break; /* ZB_NWK_COMMAND_STATUS_NO_ROUTING_CAPACITY */

        case ZB_NWK_COMMAND_STATUS_NO_INDIRECT_CAPACITY:
            break; /* ZB_NWK_COMMAND_STATUS_NO_INDIRECT_CAPACITY */

        case ZB_NWK_COMMAND_STATUS_INDIRECT_TRANSACTION_EXPIRY:
            break; /* ZB_NWK_COMMAND_STATUS_INDIRECT_TRANSACTION_EXPIRY */

        case ZB_NWK_COMMAND_STATUS_TARGET_DEVICE_UNAVAILABLE:
            break; /* ZB_NWK_COMMAND_STATUS_TARGET_DEVICE_UNAVAILABLE */

        case ZB_NWK_COMMAND_STATUS_TARGET_ADDRESS_UNALLOCATED:
            break; /* ZB_NWK_COMMAND_STATUS_TARGET_ADDRESS_UNALLOCATED */

        case ZB_NWK_COMMAND_STATUS_PARENT_LINK_FAILURE:
            tr_set_connection_state(TR_CONN_STATE_JOINED_NETWORK_NO_PARENT);
            break; /* ZB_NWK_COMMAND_STATUS_PARENT_LINK_FAILURE */

        case ZB_NWK_COMMAND_STATUS_VALIDATE_ROUTE:
            break; /* ZB_NWK_COMMAND_STATUS_VALIDATE_ROUTE */

        case ZB_NWK_COMMAND_STATUS_SOURCE_ROUTE_FAILURE:
            break; /* ZB_NWK_COMMAND_STATUS_SOURCE_ROUTE_FAILURE */

        case ZB_NWK_COMMAND_STATUS_MANY_TO_ONE_ROUTE_FAILURE:
            break; /* ZB_NWK_COMMAND_STATUS_MANY_TO_ONE_ROUTE_FAILURE */

        case ZB_NWK_COMMAND_STATUS_ADDRESS_CONFLICT:
            break; /* ZB_NWK_COMMAND_STATUS_ADDRESS_CONFLICT */

        case ZB_NWK_COMMAND_STATUS_VERIFY_ADDRESS:
            break; /* ZB_NWK_COMMAND_STATUS_VERIFY_ADDRESS */

        case ZB_NWK_COMMAND_STATUS_PAN_IDENTIFIER_UPDATE:
            break; /* ZB_NWK_COMMAND_STATUS_PAN_IDENTIFIER_UPDATE */

        case ZB_NWK_COMMAND_STATUS_NETWORK_ADDRESS_UPDATE:
            break; /* ZB_NWK_COMMAND_STATUS_NETWORK_ADDRESS_UPDATE */

        case ZB_NWK_COMMAND_STATUS_BAD_FRAME_COUNTER:
            break; /* ZB_NWK_COMMAND_STATUS_BAD_FRAME_COUNTER */

        case ZB_NWK_COMMAND_STATUS_BAD_KEY_SEQUENCE_NUMBER:
            break; /* ZB_NWK_COMMAND_STATUS_BAD_KEY_SEQUENCE_NUMBER */

        case ZB_NWK_COMMAND_STATUS_UNKNOWN_COMMAND:
            break; /* ZB_NWK_COMMAND_STATUS_UNKNOWN_COMMAND */

        default:
            break;
    }
}

static void tr_initiate_secure_nwk_rejoin(zb_uint8_t param)
{
    if (!param)
    {
        zb_buf_get_out_delayed(tr_initiate_secure_nwk_rejoin);
    }
    else
    {
        zb_uint8_t *rejoin_reason = NULL;
        rejoin_reason             = ZB_BUF_GET_PARAM(param, zb_uint8_t);
        *rejoin_reason            = ZB_REJOIN_REASON_PARENT_LOST;
        ZB_SCHEDULE_ALARM(zdo_commissioning_initiate_rejoin, param, 0);
    }
}
