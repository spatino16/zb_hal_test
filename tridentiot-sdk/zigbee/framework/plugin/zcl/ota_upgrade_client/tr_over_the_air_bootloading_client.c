/// ****************************************************************************
/// @file tr_over_the_air_bootloading_client.c
///
/// @brief This plugin handles OTA bootload client commands, imlementing
/// download, storage, and verification of ota images.
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_af.h"
#include "tr_over_the_air_bootloading_client.h"
#include "tr_ota_upgrade_common.h"
#include "tr_osif_ota.h"
#include "tr_nvram_attr.h"

// remote server side values
static tr_ota_server_info_t tr_ota_server_info = { 1, ZB_UNKNOWN_SHORT_ADDR, 0, 0, 0 };

// client side values. if the callback is not
// implemented, the stub will return the values from the cluster attributes
static tr_ota_client_info_t tr_ota_client_info;

#ifdef OVER_THE_AIR_BOOTLOADING_CLIENT_SUPPORTED_COMMANDS_RECEIVE
static zb_uint8_t gs_ota_upgrade_client_received_commands[] =
{
    OVER_THE_AIR_BOOTLOADING_CLIENT_SUPPORTED_COMMANDS_RECEIVE
};
#endif

#ifdef OVER_THE_AIR_BOOTLOADING_CLIENT_SUPPORTED_COMMANDS_SEND
static zb_uint8_t gs_ota_upgrade_client_generated_commands[] =
{
    OVER_THE_AIR_BOOTLOADING_CLIENT_SUPPORTED_COMMANDS_SEND
};
#endif

static zb_discover_cmd_list_t gs_ota_upgrade_client_cmd_list =
{
#ifdef OVER_THE_AIR_BOOTLOADING_CLIENT_SUPPORTED_COMMANDS_RECEIVE
    sizeof(gs_ota_upgrade_client_received_commands),  gs_ota_upgrade_client_received_commands,
#else
    0,                                                NULL,
#endif
#ifdef OVER_THE_AIR_BOOTLOADING_CLIENT_SUPPORTED_COMMANDS_SEND
    sizeof(gs_ota_upgrade_client_generated_commands), gs_ota_upgrade_client_generated_commands
#else
    0,                                                NULL,
#endif
};

// forward function declarations
static zb_bool_t ota_upgrade_client_cluster_handler(zb_uint8_t param);
static zb_bool_t process_ota_client_specific_commands(zb_uint8_t param);
static void zb_zcl_ota_upgrade_request_server(zb_uint8_t endpoint);

static tr_ota_upgrade_client_state_t g_ota_upgrade_client_state = TR_OTA_UPGRADE_STATE_IDLE;

// accessors for the server and client info structures
tr_ota_server_info_t *tr_ota_upgrade_client_get_server_info(void)
{
    return &tr_ota_server_info;
}

tr_ota_client_info_t *tr_ota_upgrade_client_get_client_info(void)
{
    return &tr_ota_client_info;
}

// helper function to schedule a qnir to ensure that there is only 1 scheduled
static void schedule_qnir(void)
{
    zb_uint8_t endpoint = get_endpoint_by_cluster(TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID, TR_ZCL_CLUSTER_CLIENT_ROLE);

    ZB_SCHEDULE_ALARM_CANCEL(zb_zcl_ota_upgrade_request_server, ZB_ALARM_ALL_CB);
    ZB_SCHEDULE_ALARM(zb_zcl_ota_upgrade_request_server,
                      endpoint,
                      ZB_ZCL_OTA_UPGRADE_QUERY_TIMER_INTERVAL * tr_ota_client_info.query_delay);
}

zb_uint8_t ota_upgrade_start(zb_uint32_t image_size,
                             zb_uint32_t image_version)
{
    zb_uint8_t ret = ZB_ZCL_OTA_UPGRADE_STATUS_OK;

    // TODO: LCD 10/1/24 - not sure if we need to track the flash dev or not
    // if (g_device_ctx.ota_ctx.flash_dev)
    if (0)
    {
        ret = ZB_ZCL_OTA_UPGRADE_STATUS_ERROR;
    }
    else if (!zb_osif_ota_fw_size_ok(image_size))
    {
        tr_ota_upgrade_client_printf("bad file length %d\n", image_size);
        ret = ZB_ZCL_OTA_UPGRADE_STATUS_ERROR;
    }
    else
    {
        /*
         * OTA server sends OTA file with OTA header at begin.
         * We support here trivial OTA file with only single image. We are not
         * interested on OTA header - will skip it.
         */
        tr_ota_server_info.image_size = image_size;
        tr_ota_server_info.fw_version = image_version;

        if (g_ota_upgrade_client_state != TR_OTA_UPGRADE_STATE_RESUME)
        {
            tr_ota_upgrade_client_printf("OTA Upgrade: starting upgrade to version 0x%x, file size %d\n", image_version, image_size);
            zb_osif_ota_mark_fw_absent();
        }
        else
        {
            tr_ota_upgrade_client_printf("OTA Upgrade: resuming upgrade to version 0x%x, file size %d\n", image_version, image_size);
        }

        // invoke app callback to signal ota upgrade start
        tr_over_the_air_bootloading_client_upgrade_start_cb(image_version, image_size);

        // don't erase upgrade partition if we are resuming an upgrade
        if (g_ota_upgrade_client_state != TR_OTA_UPGRADE_STATE_RESUME)
        {
            /* Simplify our life: sync erase space for entire FW.
               Alternetively can erase by portions in ota_upgrade_write_next_portion().
             */
            zb_osif_ota_erase_fw(0, 0, image_size);
        }

        ret = ZB_ZCL_OTA_UPGRADE_STATUS_OK;
    }
    return ret;
}

void ota_upgrade_server_not_found(void)
{
#ifdef CONTROL4_COMPATIBLE
//   ZB_SCHEDULE_APP_CALLBACK(sp_ota_upgrade_client_start_manually, 0);
#endif /* SP_CONTROL4_COMPATIBLE */

    // invoke app callback to signal no ota upgrade server was found
    tr_over_the_air_bootloading_client_server_not_found_cb();
}

zb_ret_t ota_upgrade_write_next_portion(zb_uint8_t  *ptr,
                                        zb_uint32_t off,
                                        zb_uint8_t  len)
{
    zb_osif_ota_write(0, ptr, off, len, tr_ota_server_info.image_size);

    return ZB_ZCL_OTA_UPGRADE_STATUS_OK;
}

zb_uint8_t ota_upgrade_check_fw(zb_uint8_t param)
{
    // flash last bit of image
    zb_osif_ota_write_last_data();

    // TODO: LCD 10/24/24 - For now use the non-async version of verify
    // if (zb_osif_ota_verify_integrity_async(0, tr_ota_server_info.image_size))
    if (zb_osif_ota_verify_integrity(0, tr_ota_server_info.image_size))
    {
        tr_ota_server_info.param = param;
        return ZB_ZCL_OTA_UPGRADE_STATUS_OK;
    }
    return ZB_ZCL_OTA_UPGRADE_STATUS_ERROR;
}

void zb_osif_ota_verify_integrity_done(zb_uint8_t integrity_is_ok)
{
    zb_zcl_ota_upgrade_send_upgrade_end_req(tr_ota_server_info.param,
                                            (integrity_is_ok == ZB_TRUE) ? ZB_ZCL_OTA_UPGRADE_STATUS_OK : ZB_ZCL_OTA_UPGRADE_STATUS_ERROR);
}

zb_uint8_t ota_upgrade_mark_fw_ok(void)
{
    zb_bool_t fw_ok;

    fw_ok = tr_osif_ota_mark_fw_ready(0, tr_ota_server_info.image_size, tr_ota_server_info.fw_version);
    zb_osif_ota_close_storage(0);
    // g_device_ctx.ota_ctx.flash_dev = NULL;

    if (fw_ok)
    {
        return ZB_ZCL_OTA_UPGRADE_STATUS_OK;
    }
    else
    {
        return ZB_ZCL_OTA_UPGRADE_STATUS_ERROR;
    }
}

void ota_upgrade_abort(void)
{
    zb_osif_ota_close_storage(0);
    // zb_osif_ota_close_storage(g_device_ctx.ota_ctx.flash_dev);
    // g_device_ctx.ota_ctx.flash_dev = NULL;
}

void tr_ota_init(zb_uint8_t param)
{
    // this schedules the init of the client which starts the server discovery
    ZB_SCHEDULE_ALARM_CANCEL(zb_zcl_ota_upgrade_init_client, ZB_ALARM_ALL_CB);
    ZB_SCHEDULE_ALARM(zb_zcl_ota_upgrade_init_client, param, 15 * ZB_TIME_ONE_SECOND);
}

void tr_over_the_air_bootloading_client_init(void)
{
    zb_zcl_add_cluster_handlers(TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                                TR_ZCL_CLUSTER_CLIENT_ROLE,
                                tr_zcl_check_value_ota_upgrade,
                                (zb_zcl_cluster_write_attr_hook_t)NULL,
                                ota_upgrade_client_cluster_handler);

    tr_ota_client_info.query_delay   = TR_OTA_UPGRADE_QUERY_DELAY_MIN;
    tr_ota_client_info.max_data_size = TR_OTA_UPGRADE_MAX_DATA_SIZE;

    // invoke app callback to signal ota client has been initialized
    tr_over_the_air_bootloading_client_init_cb();

}

// the connection state has changed, see if we need to do anything
void tr_over_the_air_bootloading_client_connection_state_cb(tr_conn_state_e conn_state)
{
    switch (conn_state)
    {
        // a join/rejoin just happened
        case TR_CONN_STATE_NWK_STEERING_ATTEMPT_SUCCESS:
        case TR_CONN_STATE_JOINED_NETWORK:
            // kick off the ota client
            zb_buf_get_in_delayed(tr_ota_init);
            break;

        default:
            break;
    }
}

static void schedule_resend_buffer(zb_uint8_t endpoint);

/* public API */
void zb_zcl_ota_set_file_size(zb_uint8_t  endpoint,
                              zb_uint32_t size)
{
    tr_ota_client_info.download_file_size = size;
}

static void zb_zcl_ota_upgrade_block_req_cb(zb_uint8_t param)
{
    zb_zcl_command_send_status_t *cmd_send_status = ZB_BUF_GET_PARAM(param, zb_zcl_command_send_status_t);

    tr_ota_client_info.img_block_req_sent = 0;

    if (tr_ota_client_info.pending_img_block_resp)
    {
        if (cmd_send_status->status == RET_OK)
        {
            /* Block_req is acknowledged, can process pending resp if exists */
            ota_upgrade_client_cluster_handler(tr_ota_client_info.pending_img_block_resp);
        }
        else
        {
            /* Block_req is not acknowledged, drop pending img_block_resp */
            zb_buf_free(tr_ota_client_info.pending_img_block_resp);
        }
    }

    tr_ota_client_info.pending_img_block_resp = 0;

    zb_buf_free(param);
}

/* Do not allow to do OTA upgrade too fast (even if it is configured by ZCL attr) - dups from OTA
 * server should be filtered. */
#define OTA_MIN_BLOCK_REQ_DELAY          \
        ZB_TIME_BEACON_INTERVAL_TO_MSEC( \
            ZB_N_APS_ACK_WAIT_DURATION_FROM_NON_SLEEPY * (ZB_N_APS_MAX_FRAME_RETRIES - 1) / ZB_APS_DUPS_TABLE_SIZE)
#define OTA_BLOCK_REQ_DELAY(_delay) (((_delay > OTA_MIN_BLOCK_REQ_DELAY)) ? (_delay) : OTA_MIN_BLOCK_REQ_DELAY)

static void zb_zcl_ota_upgrade_send_block_request(zb_uint8_t param,
                                                  zb_time_t  current_delay)
{
    zb_zcl_parsed_hdr_t cmd_info;
    zb_ieee_addr_t      our_long_address;
    zb_uint8_t          endpoint;
    zb_uint32_t         current_offset;

    g_ota_upgrade_client_state = TR_OTA_UPGRADE_STATE_IN_PROGRESS;

    ZB_MEMCPY(&cmd_info, ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t), sizeof(zb_zcl_parsed_hdr_t));
    endpoint = ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint;

    current_offset = tr_zcl_ota_upgrade_get32(endpoint, TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_FILE_OFFSET_ID);

    /* Do not request block if offset is default (image downloading is not started) */
    ZB_ASSERT(current_offset != ZB_ZCL_OTA_UPGRADE_FILE_OFFSET_DEF_VALUE);

    zb_get_long_address(our_long_address);
    ZB_ZCL_OTA_UPGRADE_SEND_IMAGE_BLOCK_REQ(param,
                                            ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).source.u.short_addr,
                                            ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                            ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).src_endpoint,
                                            endpoint,
                                            ZB_AF_HA_PROFILE_ID,
                                            ZB_FALSE,
                                            zb_zcl_ota_upgrade_block_req_cb,
                                            ZB_ZCL_OTA_UPGRADE_QUERY_IMAGE_BLOCK_IEEE_PRESENT |
                                            ZB_ZCL_OTA_UPGRADE_QUERY_IMAGE_BLOCK_DELAY_PRESENT,
                                            tr_ota_client_info.mfg_id,
                                            tr_ota_client_info.image_type,
                                            ZCL_CTX().ota_cli.ota_dfv,
                                            current_offset,
                                            tr_ota_client_info.max_data_size,
                                            our_long_address,
                                            /* We are using attribute value directly as answer to server's request to slow down,
                                             *  but calculating real delay using OTA_BLOCK_DELAY, because we MAY behave this way according to spec:
                                             * - ZCL 11.5.3 Rate Limiting
                                             *   The MinimumBlockPeriod attribute is a minimum delay. The client MAY request data slower than what the
                                             *   server specifies (i.e. with a longer delay). Sleeping end devices MAY do this normally to conserve battery
                                             *   power.
                                             * - ZCL 11.13.6.2.8 MinimumBlockPeriod (optional)
                                             *   This attribute does not necessarily reflect the actual delay applied by the client between Image Block Re-
                                             *   quests, only the value set by the server on the client. */
                                            tr_zcl_ota_upgrade_get16(endpoint, TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_MINIMUM_BLOCK_REQUEST_PERIOD_ID),
                                            OTA_BLOCK_REQ_DELAY(current_delay));
    tr_ota_client_info.img_block_req_sent = 1;
}

/****************** public function and its helper ****************/

static void zb_zcl_ota_upgrade_request_server_send(zb_uint8_t  param,
                                                   zb_uint16_t endpoint16)
{
    zb_uint8_t  endpoint;
    zb_uint16_t addr;
    zb_uint8_t  dst_endpoint;

    endpoint = (zb_uint8_t)endpoint16;

    /* zb_zcl_ota_upgrade_request_server_send is called by delayed buffer alloc,
     * so check for scheduling more than 1 zb_zcl_ota_upgrade_request_server_send
     * when ZBOSS is in OOM state. */
    if ((zb_zcl_ota_upgrade_get_ota_status(endpoint) == ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_NORMAL) ||
        (zb_zcl_ota_upgrade_get_ota_status(endpoint) == ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_DOWNLOADING) ||
        (zb_zcl_ota_upgrade_get_ota_status(endpoint) == ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_WAIT_FOR_MORE))
    {
        tr_over_the_air_bootloading_client_version_cb(endpoint,
                                                      &tr_ota_client_info.fw_version,
                                                      &tr_ota_client_info.mfg_id,
                                                      &tr_ota_client_info.image_type,
                                                      &tr_ota_client_info.hw_version);

        addr         = tr_ota_server_info.short_addr;
        dst_endpoint = tr_ota_server_info.endpoint;

        ZB_ZCL_OTA_UPGRADE_SEND_QUERY_NEXT_IMAGE_REQ(param,
                                                     addr,
                                                     ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                                     dst_endpoint,
                                                     endpoint,
                                                     ZB_AF_HA_PROFILE_ID,
                                                     ZB_FALSE,
                                                     NULL,
                                                     0,
                                                     tr_ota_client_info.mfg_id,
                                                     tr_ota_client_info.image_type,
                                                     tr_ota_client_info.fw_version,
                                                     0,
                                                     ZB_FALSE);
    }
}

static void zb_zcl_ota_upgrade_request_server(zb_uint8_t endpoint)
{
    // send a query next image request or try and resume
    if ((zb_zcl_ota_upgrade_get_ota_status(endpoint) == ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_NORMAL) || g_ota_upgrade_client_state == TR_OTA_UPGRADE_STATE_RESUME)
    {
        zb_buf_get_out_delayed_ext(zb_zcl_ota_upgrade_request_server_send, endpoint, 0);
    }

    // send another QNIR in "query_delay" minutes
    schedule_qnir();
}

static void zb_zcl_ota_server_discovery_callback(zb_uint8_t param)
{
    zb_uint8_t *zdp_cmd = zb_buf_begin(param);

    zb_zdo_match_desc_resp_t *resp       = (zb_zdo_match_desc_resp_t*)zdp_cmd;
    zb_uint8_t               *match_list = (zb_uint8_t*)(resp + 1);

    if (resp->status == ZB_ZDP_STATUS_SUCCESS &&
        /* See Use Trust Center for Cluster Command */
        ZB_ZDO_CHECK_CLUSTER_PERMISSION(resp->nwk_addr, TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID))
    {
        /* The client shall use the first response received6. 6 CCB 1314. */
        if (!resp->match_len ||
            zb_zcl_ota_upgrade_start_client((*match_list), resp->nwk_addr) != RET_OK)
        {
            /* OTA Upgrade server not found */
            ota_upgrade_server_not_found();
        }
    }
    else
    {
        /* OTA Upgrade server not found */
        if ((tr_ota_server_info.endpoint == 0) ||
            (tr_ota_server_info.short_addr == ZB_UNKNOWN_SHORT_ADDR))
        {
            ota_upgrade_server_not_found();
        }
    }

    zb_buf_free(param);
}

/* public API */
void zb_zcl_ota_upgrade_init_client(zb_uint8_t param)
{
    zb_zdo_match_desc_param_t *req;
    zb_uint16_t               tc_addr;
    zb_uint8_t                endpoint = get_endpoint_by_cluster(TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID, TR_ZCL_CLUSTER_CLIENT_ROLE);

    // start_ota_server_discovery
    req = zb_buf_initial_alloc(param, sizeof(zb_zdo_match_desc_param_t) + sizeof(zb_uint16_t));

    tc_addr = zb_aib_get_trust_center_short_address();

    /* If device is forced to use TC and we know TC addr then send request to TC or
     * send broadcast request otherwise */
    req->nwk_addr = !IS_DISTRIBUTED_SECURITY() &&
                    ZB_ZDO_CHECK_IF_FORCED_TO_USE_TC(TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID) &&
                    tc_addr != ZB_UNKNOWN_SHORT_ADDR ?
                    tc_addr : ZB_NWK_BROADCAST_RX_ON_WHEN_IDLE;
    req->addr_of_interest = req->nwk_addr;
    req->profile_id       = ZB_AF_HA_PROFILE_ID;
    req->num_in_clusters  = 1;
    req->num_out_clusters = 0;
    req->cluster_list[0]  = TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID;

    /* Do not set initial value: upgrade could be started before Match Desc Request.
       Silently fill variables in this case (zb_zcl_ota_upgrade_request_server() func already checks
       if upgrade is started). */

    if (((zb_zcl_ota_upgrade_get_ota_status(endpoint) == ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_DOWNLOADING) ||
         (zb_zcl_ota_upgrade_get_ota_status(endpoint) == ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_WAIT_FOR_MORE)) &&
        (g_ota_upgrade_client_state == TR_OTA_UPGRADE_STATE_IDLE))
    {
        // this is a resume, set the resume flag
        g_ota_upgrade_client_state = TR_OTA_UPGRADE_STATE_RESUME;
    }

    if (zb_zdo_match_desc_req(param, zb_zcl_ota_server_discovery_callback) == ZB_ZDO_INVALID_TSN)
    {
        zb_zdo_match_desc_resp_t *resp;

        resp         = zb_buf_initial_alloc(param, sizeof(zb_zdo_match_desc_resp_t));
        resp->tsn    = ZB_ZDO_INVALID_TSN;
        resp->status = ZB_ZDP_STATUS_INSUFFICIENT_SPACE;
        ZB_SCHEDULE_CALLBACK(zb_zcl_ota_server_discovery_callback, param);
    }
}

/* public API */
zb_ret_t zb_zcl_ota_upgrade_start_client(zb_uint8_t  server_ep,
                                         zb_uint16_t server_addr)
{
    zb_uint8_t    endpoint;
    zb_zcl_attr_t *attr_desc;
    zb_ret_t      ret = RET_ERROR;

    if (server_addr != ZB_ZCL_OTA_UPGRADE_SERVER_ADDR_DEF_VALUE &&
        server_ep != ZB_ZCL_OTA_UPGRADE_SERVER_ENDPOINT_DEF_VALUE)
    {
        endpoint = get_endpoint_by_cluster(TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID, TR_ZCL_CLUSTER_CLIENT_ROLE);

        /* update attribute */
        attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                           TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                                           TR_ZCL_CLUSTER_CLIENT_ROLE,
                                           TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_UPGRADE_SERVER_ID_ID);
        ZB_ASSERT(attr_desc);

        zb_address_ieee_by_short(server_addr, (zb_uint8_t*)(attr_desc->data_p));

        // save the server info
        tr_ota_server_info.endpoint   = server_ep;
        tr_ota_server_info.short_addr = server_addr;

        if ((zb_zcl_ota_upgrade_get_ota_status(endpoint) != ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_DOWNLOADING) &&
            (zb_zcl_ota_upgrade_get_ota_status(endpoint) != ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_WAIT_FOR_MORE))
        {
            // fix the state
            tr_zcl_ota_upgrade_set_ota_status(endpoint, ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_NORMAL);
        }

        // we already delayed 15 seconds since network join, now
        // delay .75 - 4.75 minutes (45-285 seconds) before first QNIR
        ZB_SCHEDULE_ALARM_CANCEL(zb_zcl_ota_upgrade_request_server, ZB_ALARM_ALL_CB);
        ZB_SCHEDULE_ALARM(zb_zcl_ota_upgrade_request_server,
                          endpoint,
                          ZB_TIME_ONE_SECOND * (ZB_RANDOM_VALUE(285) + 45));

        /* OTA Upgrade client is started */
        ret = RET_OK;
    }

    return ret;
}

/* public API */
void zb_zcl_ota_upgrade_stop_client(void)
{
    zb_uint8_t endpoint;

    /* Set OTA status to Normal - initial value for OTA */
    endpoint = get_endpoint_by_cluster(TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID, TR_ZCL_CLUSTER_CLIENT_ROLE);

    if (endpoint)
    {
        tr_zcl_ota_upgrade_set_ota_status(endpoint, ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_NORMAL);
    }

    ZB_SCHEDULE_ALARM_CANCEL(zb_zcl_ota_upgrade_init_client, ZB_ALARM_ANY_PARAM);
    ZB_SCHEDULE_ALARM_CANCEL(zb_zcl_ota_upgrade_request_server, ZB_ALARM_ALL_CB);
}

/* public API */

// this doesn't work yet...
void zb_zcl_ota_upgrade_pause_client(void)
{
    // zb_uint8_t endpoint;

    /* Set OTA status to Normal - initial value for OTA */
    // endpoint = get_endpoint_by_cluster(TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID, TR_ZCL_CLUSTER_CLIENT_ROLE);

    ZB_SCHEDULE_ALARM_CANCEL(zb_zcl_ota_upgrade_init_client, ZB_ALARM_ANY_PARAM);
    ZB_SCHEDULE_ALARM_CANCEL(zb_zcl_ota_upgrade_request_server, ZB_ALARM_ALL_CB);

    // cancel_resend_buffer();
}

void zb_zcl_ota_upgrade_file_upgraded(zb_uint8_t endpoint)
{
    zb_uint32_t value;

    value = ZB_ZCL_OTA_UPGRADE_FILE_OFFSET_DEF_VALUE;
    zb_zcl_set_attr_val_manuf(endpoint,
                              TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                              TR_ZCL_CLUSTER_CLIENT_ROLE,
                              TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_FILE_OFFSET_ID,
                              ZB_ZCL_NON_MANUFACTURER_SPECIFIC,
                              (zb_uint8_t*)&value,
                              ZB_FALSE);

    value = ZB_ZCL_OTA_UPGRADE_DOWNLOADED_FILE_VERSION_DEF_VALUE;
    zb_zcl_set_attr_val_manuf(endpoint,
                              TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                              TR_ZCL_CLUSTER_CLIENT_ROLE,
                              TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_DOWNLOADED_FILE_VERSION_ID,
                              ZB_ZCL_NON_MANUFACTURER_SPECIFIC,
                              (zb_uint8_t*)&value,
                              ZB_FALSE);
    ZCL_CTX().ota_cli.ota_dfv = ZB_ZCL_OTA_UPGRADE_DOWNLOADED_FILE_VERSION_DEF_VALUE;

    /* Restore attribute of request period from backup:
     * 11.10.10  MinimumBlockPeriod Attribute
     * This attribute SHALL reflect the minimum delay between Image Block Request commands generated by the
     * client in milliseconds. The value of this attribute SHALL be updated when the rate is changed by the server,
     * but SHOULD reflect the client default when an upgrade is not in progress or a server does not support this
     * feature. */
    zb_zcl_set_attr_val_manuf(endpoint,
                              TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                              TR_ZCL_CLUSTER_CLIENT_ROLE,
                              TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_MINIMUM_BLOCK_REQUEST_PERIOD_ID,
                              ZB_ZCL_NON_MANUFACTURER_SPECIFIC,
                              (zb_uint8_t*)&ZCL_CTX().ota_cli.ota_period_backup,
                              ZB_FALSE);

    tr_ota_client_info.download_file_size = 0;

    tr_zcl_ota_upgrade_set_ota_status(endpoint, ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_NORMAL);

    // schedule the query next image requests to start up again
    schedule_qnir();
}

/*************************** Client command handlers *************************/

// Image Notify command
static zb_ret_t image_notify_handler(zb_uint8_t param)
{
    zb_ret_t ret = RET_OK;
    /* Compilers may complain here about maybe-uninitialized without {0} when optimization is enabled */
    zb_zcl_ota_upgrade_image_notify_t payload = { 0 };
    zb_zcl_parse_status_t             status;
    zb_zcl_parsed_hdr_t               cmd_info;

    ZB_MEMCPY(&cmd_info, ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t), sizeof(zb_zcl_parsed_hdr_t));
    ZB_ZCL_OTA_UPGRADE_GET_IMAGE_NOTIFY_REQ(&payload, param, status);

    if (status != ZB_ZCL_PARSE_STATUS_SUCCESS)
    {
        ret = RET_INVALID_PARAMETER_1;
    }
    else
    {
        zb_uint8_t endpoint      = ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint;
        zb_bool_t  is_agree_file = ZB_TRUE;

        if (ZB_NWK_IS_ADDRESS_BROADCAST(cmd_info.addr_data.common_data.dst_addr))
        {
            // note: no break this switch
            //        each value payload_type mean ALL low tests must be
            switch (payload.payload_type)
            {
                case ZB_ZCL_OTA_UPGRADE_IMAGE_NOTIFY_PAYLOAD_JITTER_CODE_IMAGE_VER:
                    is_agree_file = is_agree_file &&
                                    ZB_ZCL_OTA_UPGRADE_VERSION_CMP(payload.file_version, tr_ota_client_info.fw_version);

                /* FALLTHROUGH */
                case ZB_ZCL_OTA_UPGRADE_IMAGE_NOTIFY_PAYLOAD_JITTER_CODE_IMAGE:
                    is_agree_file = is_agree_file &&
                                    (payload.image_type == tr_ota_client_info.image_type);

                /* FALLTHROUGH */
                case ZB_ZCL_OTA_UPGRADE_IMAGE_NOTIFY_PAYLOAD_JITTER_CODE:
                    is_agree_file = is_agree_file &&
                                    (payload.manufacturer == tr_ota_client_info.mfg_id);
            }

            if (is_agree_file)
            {
                zb_uint8_t my_jitter_rnd = ZB_RANDOM_JTR(ZB_ZCL_OTA_UPGRADE_QUERY_JITTER_MAX_VALUE) + 1;
#ifdef ZB_STACK_REGRESSION_TESTING_API

                if (ZB_REGRESSION_TESTS_API().zcl_ota_custom_query_jitter)
                {
                    my_jitter_rnd = ZB_REGRESSION_TESTS_API().zcl_ota_custom_query_jitter;
                }
#endif /* ZB_STACK_REGRESSION_TESTING_API */
                is_agree_file = is_agree_file && (my_jitter_rnd <= payload.query_jitter);
            }
        }
        else
        {
            is_agree_file = is_agree_file &&
                            (cmd_info.addr_data.common_data.dst_addr == zb_get_short_address());
        }

        if ((zb_zcl_ota_upgrade_get_ota_status(endpoint) != ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_NORMAL) &&
            (g_ota_upgrade_client_state != TR_OTA_UPGRADE_STATE_RESUME))
        {
            /* FIXME: WHY? It is not by spec!
               Zigbee Document 095264r22, Zigbee Over-the-Air Upgrading Cluster, Revision 22, Version 1.1

               6.10.3.3:
               However, payload type value of 0x03 has a slightly different effect.  If  the client device has all the
               information matching those included in the command including the new file version, the device shall
               then ignore the command.  This indicates that the device has already gone through the upgrade process.
               This is to prevent the device from downloading the same image version multiple times.  This is only
               true if the command is sent as broadcast/multicast.

               6.10.3.4:
               On receipt of a unicast Image Notify command, the device shall always send a Query Next Image
               request back to the upgrade server.  This provides a way for the server to force reinstallation of image
               on the device.
               ...
               On receipt of a broadcast or multicast Image Notify command, the device shall keep examining each
               field included in the payload with its own value.  For each field, if the value matches its own, it shall
               proceed to examine the next field.  If values in all three fields (naming manufacturer code, image type
               and new file version) match its own values, then it shall discard the command.  The new file version in
               the payload shall match either the devices current running file version or the downloaded file version
               (on the additional memory space).
               ...
               For payload type value of 0x03, if
               both manufacturer code and image type match the devices own values but the new file version is not a
               match, the device shall proceed.  In this case, the (new) file version may be lower or higher than the
               devices file version to indicate a downgrade or an upgrade of the firmware respectively.

               TODO: Modify this behaviour:
               <--- Drop it only if it is ZB_ZCL_OTA_UPGRADE_IMAGE_NOTIFY_PAYLOAD_JITTER_CODE_IMAGE_VER,
               all checks (including file version) are ok and command is broadcast/multicast. If command
               is unicast, proceed it even if all fields are equal (even if file version is the same) -
               abort current download and begin new.
             */
            is_agree_file = ZB_FALSE;
        }

        if (is_agree_file)
        {
            /* Reschedule regular Query Next Image Request, because the most recent call is going to happen now */
            schedule_qnir();

            /* For the case we didn't got it from the notify command */
            tr_over_the_air_bootloading_client_version_cb(endpoint,
                                                          &tr_ota_client_info.fw_version,
                                                          &tr_ota_client_info.mfg_id,
                                                          &tr_ota_client_info.image_type,
                                                          &tr_ota_client_info.hw_version);
            ZB_ZCL_OTA_UPGRADE_SEND_QUERY_NEXT_IMAGE_REQ(param,
                                                         ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).source.u.short_addr,
                                                         ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                                         ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).src_endpoint,
                                                         ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
                                                         ZB_AF_HA_PROFILE_ID,
                                                         ZB_FALSE,
                                                         NULL,
                                                         ZB_ZCL_OTA_UPGRADE_QUERY_NEXT_IMAGE_HW_VERSION,
                                                         tr_ota_client_info.mfg_id,
                                                         tr_ota_client_info.image_type,
                                                         tr_ota_client_info.fw_version,
                                                         tr_ota_client_info.hw_version,
                                                         (zb_bool_t)ZB_APS_FC_GET_SECURITY(cmd_info.addr_data.common_data.fc));

            ret = RET_BUSY;
        }
    }

    return ret;
}

// Query Next Image Request command
static zb_ret_t query_next_image_resp_handler(zb_uint8_t param)
{
    zb_ret_t                                  ret     = RET_OK;
    zb_zcl_ota_upgrade_query_next_image_res_t payload = { 0 };
    zb_zcl_parse_status_t                     status;
    zb_zcl_parsed_hdr_t                       cmd_info;
    zb_uint32_t                               file_offset;

    ZB_MEMCPY(&cmd_info, ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t), sizeof(zb_zcl_parsed_hdr_t));

    ZB_ZCL_OTA_UPGRADE_GET_QUERY_NEXT_IMAGE_RES(&payload, param, status);

    if (status != ZB_ZCL_PARSE_STATUS_SUCCESS)
    {
        ret = RET_INVALID_PARAMETER_1;
    }
    else
    {
        zb_uint8_t endpoint = ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint;

        switch (payload.status)
        {
            /* Server returned success if FW must be upgraded basing on file_version
             * hw_version. It is up to application to decide which sub-element must be
             * upgraded (see below)
             */
            case TR_ZCL_STATUS_SUCCESS:
            {
                zb_uint16_t delay      = tr_zcl_ota_upgrade_get16(endpoint, TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_MINIMUM_BLOCK_REQUEST_PERIOD_ID);
                zb_uint8_t  app_status = TR_ZCL_STATUS_SUCCESS;

                if (payload.file_version ==
                    tr_zcl_ota_upgrade_get32(endpoint, TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_CURRENT_FILE_VERSION_ID))
                {
                    /* ZCL8, 11.13.5.4 Effect on receipt:
                       If the file version contained in the Query Next Image Response is the same as the
                       CurrentFileVersion attribute (the current running version of software) or the
                       DownloadedFileVersion attribute for the specified Image Type, then the message SHOULD
                       be discarded and no further processing SHOULD be done. */
                    app_status = ZB_ZCL_OTA_UPGRADE_STATUS_ERROR;
                }
                else
                {
                    /* indicate upgrade process start. */
                    app_status = ota_upgrade_start(payload.image_size, payload.file_version);
                }

                if (app_status == ZB_ZCL_OTA_UPGRADE_STATUS_OK)
                {
                    // stop the qnir from going out while upgrading
                    ZB_SCHEDULE_ALARM_CANCEL(zb_zcl_ota_upgrade_request_server, ZB_ALARM_ALL_CB);

                    // are we resuming or starting fresh?
                    if (g_ota_upgrade_client_state == TR_OTA_UPGRADE_STATE_RESUME)
                    {

                        // we are resuming, adjust the current file offset back to the last written page
                        file_offset              = tr_zcl_ota_upgrade_get32(endpoint, TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_FILE_OFFSET_ID);
                        zb_zcl_attr_t *attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                                                          TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                                                                          TR_ZCL_CLUSTER_CLIENT_ROLE,
                                                                          TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_FILE_OFFSET_ID);
                        ZB_ASSERT(attr_desc);
                        file_offset &= (zb_uint32_t)(~(OTA_FLASH_WRITE_SIZE - 1));
                        ZB_ZCL_SET_DIRECTLY_ATTR_VAL32(attr_desc, file_offset);

                        // update the next write address to the flash
                        tr_osif_ota_flash_set_write_addr(file_offset);
                        tr_ota_upgrade_client_printf("Resuming OTA upgrade from offset 0x%x\n", file_offset);
                        g_ota_upgrade_client_state = TR_OTA_UPGRADE_STATE_IN_PROGRESS;
                    }
                    else
                    {
                        file_offset = 0;
                        zb_zcl_set_attr_val_manuf(endpoint,
                                                  TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                                                  TR_ZCL_CLUSTER_CLIENT_ROLE,
                                                  TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_FILE_OFFSET_ID,
                                                  ZB_ZCL_NON_MANUFACTURER_SPECIFIC,
                                                  (zb_uint8_t*)&file_offset,
                                                  ZB_FALSE);
                        tr_ota_upgrade_client_printf("Starting OTA upgrade from offset 0x%x\n", file_offset);
                    }
                    tr_ota_client_info.offset_at_last_attr_save = file_offset;
                    g_ota_upgrade_client_state                  = TR_OTA_UPGRADE_STATE_IN_PROGRESS;

                    ZCL_CTX().ota_cli.ota_dfv = payload.file_version;

#ifdef ZB_STACK_REGRESSION_TESTING_API

                    if (ZB_REGRESSION_TESTS_API().zcl_ota_custom_file_version != 0)
                    {
                        ZCL_CTX().ota_cli.ota_dfv = ZB_REGRESSION_TESTS_API().zcl_ota_custom_file_version;
                    }
#endif /* ZB_STACK_REGRESSION_TESTING_API */

                    tr_ota_client_info.download_file_size = payload.image_size;

                    /* Backup period attribute */
                    if (delay > 5)
                    {
                        delay = 0;
                    }
                    ZCL_CTX().ota_cli.ota_period_backup = delay;
                    tr_zcl_ota_upgrade_set_ota_status(endpoint, ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_DOWNLOADING);

                    // send query block: offset 0
                    ZB_MEMCPY(ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t), &cmd_info, sizeof(zb_zcl_parsed_hdr_t));
                    /* In any case we need OTA file header. Then there can be variants (see
                     * multi-fw). Variants are to be implemented in the application. */
                    zb_zcl_ota_upgrade_send_block_request(param, delay);

                    ret = RET_BUSY;
                    /* Fill data for resend_buffer. FIXME: rewrite it */
                    ZB_MEMCPY(&ZCL_CTX().ota_cli.cmd_info_2, &cmd_info, sizeof(zb_zcl_parsed_hdr_t));
                    ZCL_CTX().ota_cli.payload_2.response.success.manufacturer = payload.manufacturer;
                    ZCL_CTX().ota_cli.payload_2.response.success.image_type   = payload.image_type;
                    ZCL_CTX().ota_cli.payload_2.response.success.file_version = payload.file_version;
                    schedule_resend_buffer(endpoint);
                }
            }
            break;

            case TR_ZCL_STATUS_NO_IMAGE_AVAILABLE:
            default:
                break;
        }
    }

    return ret;
}

static void resend_buffer(zb_uint8_t param);

static void cancel_resend_buffer()
{
    ZCL_CTX().ota_cli.resend_retries = 0;
    ZB_SCHEDULE_ALARM_CANCEL(resend_buffer, 0);
}

static void schedule_resend_buffer(zb_uint8_t endpoint)
{
    zb_uint16_t delay = tr_zcl_ota_upgrade_get16(endpoint, TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_MINIMUM_BLOCK_REQUEST_PERIOD_ID);

    ZCL_CTX().ota_cli.resend_retries = 0;
    ZB_SCHEDULE_ALARM_CANCEL(resend_buffer, 0);
    /* Extend resend interval to exclude situation when we request new block and retransmit APS packet
     * with old request. */
    ZB_SCHEDULE_ALARM(resend_buffer, 0, ZB_ZCL_OTA_UPGRADE_RESEND_BUFFER_DELAY + ZB_MILLISECONDS_TO_BEACON_INTERVAL(delay));
}

/* public API */
void zcl_ota_abort(zb_uint8_t endpoint,
                   zb_uint8_t param)
{
    zb_uint8_t    status = ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_NORMAL;
    zb_zcl_attr_t *attr_desc;
    zb_uint32_t   value;

    tr_ota_upgrade_client_printf("OTA Upgrade: Abort\n");
    attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                       TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                                       TR_ZCL_CLUSTER_CLIENT_ROLE,
                                       TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_IMAGE_UPGRADE_STATUS_ID);

    if (attr_desc)
    {
        status = ZB_ZCL_GET_ATTRIBUTE_VAL_8(attr_desc);
    }
    // set attribute
    tr_zcl_ota_upgrade_set_ota_status(endpoint, ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_NORMAL);
    /* cancel resend */
    cancel_resend_buffer();

    // set attribute
    value = ZB_ZCL_OTA_UPGRADE_FILE_OFFSET_DEF_VALUE;
    zb_zcl_set_attr_val_manuf(endpoint,
                              TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                              TR_ZCL_CLUSTER_CLIENT_ROLE,
                              TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_FILE_OFFSET_ID,
                              ZB_ZCL_NON_MANUFACTURER_SPECIFIC,
                              (zb_uint8_t*)&value,
                              ZB_FALSE);

    // reset downloaded file version attribute
    value = ZB_ZCL_OTA_UPGRADE_DOWNLOADED_FILE_VERSION_DEF_VALUE;
    zb_zcl_set_attr_val_manuf(endpoint,
                              TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                              TR_ZCL_CLUSTER_CLIENT_ROLE,
                              TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_DOWNLOADED_FILE_VERSION_ID,
                              ZB_ZCL_NON_MANUFACTURER_SPECIFIC,
                              (zb_uint8_t*)&value,
                              ZB_FALSE);
    ZCL_CTX().ota_cli.ota_dfv = ZB_ZCL_OTA_UPGRADE_DOWNLOADED_FILE_VERSION_DEF_VALUE;

    /* Restore attribute of request period from backup*/
    zb_zcl_set_attr_val_manuf(endpoint,
                              TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                              TR_ZCL_CLUSTER_CLIENT_ROLE,
                              TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_MINIMUM_BLOCK_REQUEST_PERIOD_ID,
                              ZB_ZCL_NON_MANUFACTURER_SPECIFIC,
                              (zb_uint8_t*)&ZCL_CTX().ota_cli.ota_period_backup,
                              ZB_FALSE);
    // TODO: LCD 10/23/24 - what is going on here, missing "if"? the code is this way in
    // zboss zcl_ota_upgrade_commands.c
    {
        tr_ota_client_info.img_block_req_sent = 0;

        if (tr_ota_client_info.pending_img_block_resp)
        {
            zb_buf_free((tr_ota_client_info.pending_img_block_resp));
            tr_ota_client_info.pending_img_block_resp = 0;
        }
    }

    if (status != ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_NORMAL)
    {
        if ((param > 0) && (param != ZB_UNDEFINED_BUFFER))
        {
            ota_upgrade_abort();
        }
        else
        {
            zb_bufid_t send_buf = zb_buf_get_out();

            ZB_ASSERT(send_buf);
            ota_upgrade_abort();
            zb_buf_free(send_buf);
        }
    }

    // schedule the query next image requests to start up again
    schedule_qnir();
}

/* Helper routine to finish OTA Upgrade */
static void zb_zcl_ota_upgrade_end(zb_uint8_t                           param,
                                   zb_uint8_t                           status,
                                   zb_zcl_parsed_hdr_t                  *cmd_info,
                                   zb_zcl_ota_upgrade_image_block_res_t *payload)
{
    zb_uint8_t endpoint = ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).dst_endpoint;

    if (status != TR_ZCL_STATUS_SUCCESS)
    {
        zcl_ota_abort(endpoint, param);
    }

    /* send upgrade end request */
    ZB_ZCL_OTA_UPGRADE_SEND_UPGRADE_END_REQ(param,
                                            ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).source.u.short_addr,
                                            ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                            ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).src_endpoint,
                                            ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).dst_endpoint,
                                            ZB_AF_HA_PROFILE_ID,
                                            ZB_FALSE,
                                            NULL,
                                            status,
                                            payload->response.success.manufacturer,
                                            payload->response.success.image_type,
                                            payload->response.success.file_version);
}

// try resend req
static void resend_buffer(zb_uint8_t param)
{
    zb_ieee_addr_t our_long_address = { 0 };
    zb_bufid_t     send_buf         = 0;
    zb_uint32_t    current_offset;
    zb_zcl_attr_t  *attr_desc;
    zb_uint8_t     endpoint;
    zb_uint16_t    delay;

    ZCL_CTX().ota_cli.ota_restart_after_rejoin = 0;
    ZVUNUSED(param);
    send_buf = zb_buf_get_out();

    if (send_buf)
    {
        zb_buf_reuse(send_buf);
        endpoint  = ZB_ZCL_PARSED_HDR_SHORT_DATA(&ZCL_CTX().ota_cli.cmd_info_2).dst_endpoint;
        attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                           TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                                           TR_ZCL_CLUSTER_CLIENT_ROLE,
                                           TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_IMAGE_UPGRADE_STATUS_ID);

        if (attr_desc
            && ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_DOWNLOADING == ZB_ZCL_GET_ATTRIBUTE_VAL_8(attr_desc))
        {
            ZCL_CTX().ota_cli.resend_retries++;

            if (ZCL_CTX().ota_cli.resend_retries < ZCL_OTA_MAX_RESEND_RETRIES)
            {
                attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                                   TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                                                   TR_ZCL_CLUSTER_CLIENT_ROLE,
                                                   TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_FILE_OFFSET_ID);
                ZB_ASSERT(attr_desc);
                current_offset = ZB_ZCL_GET_ATTRIBUTE_VAL_32(attr_desc);
                delay          = tr_zcl_ota_upgrade_get16(endpoint, TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_MINIMUM_BLOCK_REQUEST_PERIOD_ID);

                zb_get_long_address(our_long_address);
                ZB_ZCL_OTA_UPGRADE_SEND_IMAGE_BLOCK_REQ(send_buf,
                                                        ZB_ZCL_PARSED_HDR_SHORT_DATA(&ZCL_CTX().ota_cli.cmd_info_2).source.u.short_addr,
                                                        ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                                        ZB_ZCL_PARSED_HDR_SHORT_DATA(&ZCL_CTX().ota_cli.cmd_info_2).src_endpoint,
                                                        ZB_ZCL_PARSED_HDR_SHORT_DATA(&ZCL_CTX().ota_cli.cmd_info_2).dst_endpoint,
                                                        ZB_AF_HA_PROFILE_ID,
                                                        ZB_FALSE,
                                                        zb_zcl_ota_upgrade_block_req_cb,
                                                        (ZB_ZCL_OTA_UPGRADE_QUERY_IMAGE_BLOCK_IEEE_PRESENT |
                                                         ZB_ZCL_OTA_UPGRADE_QUERY_IMAGE_BLOCK_DELAY_PRESENT),
                                                        ZCL_CTX().ota_cli.payload_2.response.success.manufacturer,
                                                        ZCL_CTX().ota_cli.payload_2.response.success.image_type,
                                                        ZCL_CTX().ota_cli.payload_2.response.success.file_version,
                                                        current_offset,
                                                        tr_ota_client_info.max_data_size,
                                                        our_long_address,
                                                        /* We are using attribute value directly as answer to server's request to slow down,
                                                         *  but calculating real delay using OTA_BLOCK_DELAY, because we MAY behave this way according to spec:
                                                         * - ZCL 11.5.3 Rate Limiting
                                                         *   The MinimumBlockPeriod attribute is a minimum delay. The client MAY request data slower than what the
                                                         *   server specifies (i.e. with a longer delay). Sleeping end devices MAY do this normally to conserve battery
                                                         *   power.
                                                         * - ZCL 11.13.6.2.8 MinimumBlockPeriod (optional)
                                                         *   This attribute does not necessarily reflect the actual delay applied by the client between Image Block Re-
                                                         *   quests, only the value set by the server on the client. */
                                                        delay,
                                                        OTA_BLOCK_REQ_DELAY(delay));
                tr_ota_client_info.img_block_req_sent = 1;
                ZB_SCHEDULE_ALARM_CANCEL(resend_buffer, 0);
                /* Extend resend interval to exclude situation when we request new block and retransmit APS packet
                 * with old request. */
                ZB_SCHEDULE_ALARM(resend_buffer, 0, ZB_ZCL_OTA_UPGRADE_RESEND_BUFFER_DELAY + ZB_MILLISECONDS_TO_BEACON_INTERVAL(delay));
            }
            else
            {
                zb_zcl_ota_upgrade_end(send_buf,
                                       TR_ZCL_STATUS_ABORT,
                                       &(ZCL_CTX().ota_cli.cmd_info_2),
                                       &(ZCL_CTX().ota_cli.payload_2));

                /* zb_buf_free(send_buf); */
            }
        }
        else
        {
            zb_buf_free(send_buf);
        }
    }
    else
    {
        ZB_SCHEDULE_CALLBACK(resend_buffer, 0);
    }
}

/* Helper routine to get next image block */
static void zb_zcl_ota_upgrade_get_next_image_block(zb_uint8_t                           param,
                                                    zb_zcl_parsed_hdr_t                  *cmd_info,
                                                    zb_zcl_ota_upgrade_image_block_res_t *payload)
{
    zb_uint8_t endpoint = ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).dst_endpoint;

    zb_zcl_ota_upgrade_send_block_request(param, tr_zcl_ota_upgrade_get16(endpoint, TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_MINIMUM_BLOCK_REQUEST_PERIOD_ID));

    /* Schedule resend buffer if we don't get response */
    ZB_MEMCPY(&ZCL_CTX().ota_cli.cmd_info_2, cmd_info, sizeof(zb_zcl_parsed_hdr_t));
    ZB_MEMCPY(&ZCL_CTX().ota_cli.payload_2, payload, sizeof(zb_zcl_ota_upgrade_image_block_res_t));
    schedule_resend_buffer(endpoint);
}

/* Helper routine to process downloaded image */
static void zb_zcl_ota_upgrade_process_downloaded_image(zb_uint8_t                           param,
                                                        zb_zcl_parsed_hdr_t                  *cmd_info,
                                                        zb_zcl_ota_upgrade_image_block_res_t *payload)
{
    zb_uint8_t endpoint;
    zb_uint8_t upgrade_status;

    /* reserve space for user callback parameters, it will be used later while
     * calling this callback. When space is reserved, buffer data is moved to the left -
     * it is better to do it here before any actions with pointers to data */
    (void)ZB_BUF_GET_PARAM(param, zb_zcl_device_callback_param_t);

    endpoint = ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).dst_endpoint;

    cancel_resend_buffer();

    // update attribute in case it was declared by user
    zb_zcl_set_attr_val_manuf(endpoint,
                              TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                              TR_ZCL_CLUSTER_CLIENT_ROLE,
                              TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_DOWNLOADED_FILE_VERSION_ID,
                              ZB_ZCL_NON_MANUFACTURER_SPECIFIC,
                              (zb_uint8_t*)&ZCL_CTX().ota_cli.ota_dfv,
                              ZB_FALSE);

    tr_zcl_ota_upgrade_set_ota_status(endpoint, ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_DOWNLOADED);

    upgrade_status = ota_upgrade_check_fw(param);

    if (upgrade_status != ZB_ZCL_OTA_UPGRADE_STATUS_BUSY)
    {
        /* Finish OTA process */
        zb_zcl_ota_upgrade_end(param,
                               (upgrade_status == ZB_ZCL_OTA_UPGRADE_STATUS_OK ?
                                TR_ZCL_STATUS_SUCCESS : TR_ZCL_STATUS_INVALID_IMAGE),
                               cmd_info,
                               payload);
    }
    else
    {
        /* Application is busy, restore parameters and wait the signal to resume */
        ZB_MEMCPY(ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t), cmd_info, sizeof(zb_zcl_parsed_hdr_t));
    }
}

/* public API. Never called */

/* Resume OTA Upgrade signal from application */
void zb_zcl_ota_upgrade_resume_client(zb_uint8_t param,
                                      zb_uint8_t upgrade_status)
{
    zb_uint8_t                           endpoint;
    zb_uint8_t                           ota_status;
    zb_zcl_ota_upgrade_image_block_res_t payload = { 0 };
    zb_zcl_parsed_hdr_t                  cmd_info;
    zb_zcl_parse_status_t                status = ZB_ZCL_PARSE_STATUS_FAILURE;

    ZB_MEMCPY(&cmd_info, ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t), sizeof(zb_zcl_parsed_hdr_t));

    ZB_ZCL_OTA_UPGRADE_GET_IMAGE_BLOCK_RES(&payload, param, status);

    if (status != ZB_ZCL_PARSE_STATUS_SUCCESS)
    {
        zb_buf_free(param);
        return;
    }

    endpoint   = ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint;
    ota_status = zb_zcl_ota_upgrade_get_ota_status(endpoint);

    if (ota_status == ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_DOWNLOADING ||
        ota_status == ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_WAIT_FOR_MORE)
    {
        if (upgrade_status != ZB_ZCL_OTA_UPGRADE_STATUS_OK)
        {
            zb_zcl_ota_upgrade_end(param, TR_ZCL_STATUS_INVALID_IMAGE, &cmd_info, &payload);
        }
        else
        {
            zb_uint32_t   file_size;
            zb_uint32_t   current_offset;
            zb_zcl_attr_t *attr_desc = NULL;

            attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                               TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                                               TR_ZCL_CLUSTER_CLIENT_ROLE,
                                               TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_FILE_OFFSET_ID);
            ZB_ASSERT(attr_desc);

            file_size      = tr_ota_client_info.download_file_size;
            current_offset = ZB_ZCL_GET_ATTRIBUTE_VAL_32(attr_desc);

            if (file_size > current_offset)
            {
                zb_zcl_ota_upgrade_get_next_image_block(param, &cmd_info, &payload);
            }
            else
            {
                zb_zcl_ota_upgrade_process_downloaded_image(param, &cmd_info, &payload);
            }
        }
    }
    else if (ota_status == ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_DOWNLOADED)
    {
        zb_zcl_ota_upgrade_end(param,
                               (upgrade_status == ZB_ZCL_OTA_UPGRADE_STATUS_OK ?
                                TR_ZCL_STATUS_SUCCESS : TR_ZCL_STATUS_INVALID_IMAGE),
                               &cmd_info,
                               &payload);
    }
    else
    {
        zb_buf_free(param);
    }
}

/* public API */
void zb_zcl_ota_restart_after_rejoin(zb_uint8_t endpoint)
{
    zb_zcl_attr_t *attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                                      TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                                                      TR_ZCL_CLUSTER_CLIENT_ROLE,
                                                      TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_IMAGE_UPGRADE_STATUS_ID);

    if ((attr_desc != NULL)
        && ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_DOWNLOADING == ZB_ZCL_GET_ATTRIBUTE_VAL_8(attr_desc))
    {
        ZCL_CTX().ota_cli.ota_restart_after_rejoin = 1;

        /* Moved from zb_zcl_ota_server_discovery_callback */
        /* Fill data for resend_buffer.  */
        ZB_ZCL_PARSED_HDR_SHORT_DATA(&ZCL_CTX().ota_cli.cmd_info_2).dst_endpoint        = endpoint;        /*    src ep (yes, src) */
        ZB_ZCL_PARSED_HDR_SHORT_DATA(&ZCL_CTX().ota_cli.cmd_info_2).source.u.short_addr = zb_zcl_ota_upgrade_get16(endpoint,
                                                                                                                   ZB_ZCL_ATTR_OTA_UPGRADE_SERVER_ADDR_ID);
        ZB_ZCL_PARSED_HDR_SHORT_DATA(&ZCL_CTX().ota_cli.cmd_info_2).src_endpoint = zb_zcl_ota_upgrade_get8(endpoint,
                                                                                                           ZB_ZCL_ATTR_OTA_UPGRADE_SERVER_ENDPOINT_ID);         /*    dst ep (yes, dst) */
        ZCL_CTX().ota_cli.payload_2.response.success.manufacturer = tr_ota_client_info.mfg_id;
        ZCL_CTX().ota_cli.payload_2.response.success.image_type   = tr_ota_client_info.image_type;
        ZCL_CTX().ota_cli.payload_2.response.success.file_version = ZCL_CTX().ota_cli.ota_dfv;

        schedule_resend_buffer(endpoint);
    }
}

static void zb_zcl_ota_upgrade_finish_upgrade(zb_uint8_t param)
{
    zb_uint8_t user_ret;
    zb_uint8_t endpoint = *ZB_BUF_GET_PARAM(param, zb_uint8_t);

    zb_zcl_ota_upgrade_file_upgraded(endpoint);
    ZVUNUSED(user_ret);

    zb_buf_free(param);

    tr_zcl_ota_upgrade_set_ota_status(endpoint, ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_NORMAL);

    // schedule the query next image requests to start up again
    schedule_qnir();
    zb_osif_upgrade_now();
}

// Image Block Response command
static zb_ret_t image_block_resp_handler(zb_uint8_t param)
{
    zb_ret_t                             ret = RET_OK;
    zb_zcl_ota_upgrade_image_block_res_t payload;
    zb_zcl_parse_status_t                status;
    zb_zcl_parsed_hdr_t                  cmd_info;

    ZB_MEMCPY(&cmd_info, ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t), sizeof(zb_zcl_parsed_hdr_t));

    ZB_ZCL_OTA_UPGRADE_GET_IMAGE_BLOCK_RES(&payload, param, status);

    if (status != ZB_ZCL_PARSE_STATUS_SUCCESS)
    {
        ret = RET_INVALID_PARAMETER_1;
    }
    else
    {
        zb_uint8_t    endpoint = ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint;
        zb_zcl_attr_t *attr_desc;
        zb_uint8_t    ota_status = zb_zcl_ota_upgrade_get_ota_status(endpoint);

        /* If we are not downloading the image, do nothing */
        if (ota_status == ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_NORMAL)
        {
            ret = RET_ERROR;
        }
        /* If fw is already downloaded, do not allow new Image Block Responses */
        else if (ota_status == ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_DOWNLOADED ||
                 ota_status == ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_WAITING_UPGRADE ||
                 ota_status == ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_COUNT_DOWN)
        {
            /* Do not abort here - simply drop the packet (it may be retransmit etc). */
            /* zcl_ota_abort(endpoint, param); */

            /* ZB_ZCL_OTA_UPGRADE_SEND_UPGRADE_END_REQ(param, */
            /*                                         ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).source.u.short_addr, */
            /*                                         ZB_APS_ADDR_MODE_16_ENDP_PRESENT, */
            /*                                         ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).src_endpoint, */
            /*                                         ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint, */
            /*                                         ZB_AF_HA_PROFILE_ID, ZB_FALSE, NULL, */
            /*                                         TR_ZCL_STATUS_INVALID_IMAGE, */
            /*                                         zb_zcl_ota_upgrade_get16(endpoint, TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_MANUFACTURER_ID_ID), */
            /*                                         zb_zcl_ota_upgrade_get16(endpoint, TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_IMAGE_TYPE_ID_ID), */
            /*                                         zb_zcl_ota_upgrade_get32(endpoint, TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_DOWNLOADED_FILE_VERSION_ID)); */
            zb_buf_free(param);
            ret = RET_BUSY;
        }
        else
        /* According to ota_status, download is in progress. */
        {
            switch (payload.status)
            {
                case TR_ZCL_STATUS_SUCCESS:
                    attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                                       TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                                                       TR_ZCL_CLUSTER_CLIENT_ROLE,
                                                       TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_MINIMUM_BLOCK_REQUEST_PERIOD_ID);
                    ZB_ASSERT(attr_desc);
                    // TODO: LCD 10/23/24 - should this be commented out? it is
                    // commented out in zboss zcl_ota_upgrade_commands.c
                    /* clear delay before use
                       ZB_ZCL_SET_DIRECTLY_ATTR_VAL16(attr_desc, 0);*/

                    if (payload.response.success.manufacturer == tr_ota_client_info.mfg_id &&
                        payload.response.success.image_type == tr_ota_client_info.image_type &&
                        payload.response.success.file_version == ZCL_CTX().ota_cli.ota_dfv)
                    {
                        zb_uint32_t file_size = tr_ota_client_info.download_file_size;
                        zb_uint32_t current_offset;
                        zb_bool_t   get_next_block = ZB_FALSE;
                        zb_uint_t   upgrade_status = TR_ZCL_STATUS_SUCCESS;

                        attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                                           TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                                                           TR_ZCL_CLUSTER_CLIENT_ROLE,
                                                           TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_FILE_OFFSET_ID);
                        ZB_ASSERT(attr_desc);

                        current_offset = ZB_ZCL_GET_ATTRIBUTE_VAL_32(attr_desc);

                        if (payload.response.success.file_offset == current_offset)
                        {
                            /* reserve space for user callback parameters, it will be used later while
                             * calling this callback. When space is reserved, buffer data is moved to the left -
                             * it is better to do it here before any actions with pointers to data */
                            (void)ZB_BUF_GET_PARAM(param, zb_zcl_device_callback_param_t);

                            /* Make sure that the image payload pointer is correct after making the reservation. */
                            ZB_ZCL_OTA_UPGRADE_GET_IMAGE_BLOCK_RES(&payload, param, status);

                            /* Block with expected offset is received, cancel resending. */
                            cancel_resend_buffer();
                            // call User App
                            // NOTE file data place`s in buffer, payload saves pointer to data only!

                            /* Move offset before user's callback in case user's callback move it */
                            current_offset += payload.response.success.data_size;
                            zb_zcl_set_attr_val_manuf(endpoint,
                                                      TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                                                      TR_ZCL_CLUSTER_CLIENT_ROLE,
                                                      TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_FILE_OFFSET_ID,
                                                      ZB_ZCL_NON_MANUFACTURER_SPECIFIC,
                                                      (zb_uint8_t*)&current_offset,
                                                      ZB_FALSE);

                            upgrade_status = ota_upgrade_write_next_portion(payload.response.success.image_data,
                                                                            payload.response.success.file_offset,
                                                                            payload.response.success.data_size);

                            if (upgrade_status == ZB_ZCL_OTA_UPGRADE_STATUS_OK)
                            {
                                /* User's callback can change current_offset, so re-read it */
                                current_offset = ZB_ZCL_GET_ATTRIBUTE_VAL_32(attr_desc);
                                get_next_block = ZB_TRUE;

                                // we need to force saving attributes to nvram every time the flash is written
                                if ((current_offset - tr_ota_client_info.offset_at_last_attr_save) >= OTA_FLASH_WRITE_SIZE)
                                {
                                    tr_ota_client_info.offset_at_last_attr_save = current_offset;
                                    tr_check_for_attr_nvram_update_and_force_save(endpoint,
                                                                                  TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                                                                                  TR_ZCL_CLUSTER_CLIENT_ROLE,
                                                                                  TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_FILE_OFFSET_ID,
                                                                                  ZB_ZCL_NON_MANUFACTURER_SPECIFIC);
                                }
                            }
                            else if (upgrade_status == ZB_ZCL_OTA_UPGRADE_STATUS_BUSY)
                            {
                                /* restore parameters */
                                ZB_MEMCPY(ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t), &cmd_info, sizeof(zb_zcl_parsed_hdr_t));
                                ret = RET_BUSY;
                                break;
                            }
                            else
                            {
                                zb_zcl_ota_upgrade_end(param, TR_ZCL_STATUS_INVALID_IMAGE, &cmd_info, &payload);
                                ret = RET_BUSY;
                                /* Prevent sending Upgrade End request twice when we downloaded entire file with some error. */
                                break;
                            }
                        }
                        else
                        {
                            if (ZCL_CTX().ota_cli.ota_restart_after_rejoin)
                            {
                                /* ZC thinks we are doing OTA. If so, let's start just now */
                                ZCL_CTX().ota_cli.ota_restart_after_rejoin = 0;
                                get_next_block                             = ZB_TRUE;
                            }
                        }

                        if (file_size > current_offset)
                        {
                            if (get_next_block)
                            {
                                /* restore parameters */
                                ZB_MEMCPY(ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t), &cmd_info, sizeof(zb_zcl_parsed_hdr_t));
                                zb_zcl_ota_upgrade_get_next_image_block(param, &cmd_info, &payload);

                                /* Mark buffer as BUSY - it will not be release */
                                ret = RET_BUSY;
                            }
                        }
                        else
                        {
                            zb_zcl_ota_upgrade_process_downloaded_image(param, &cmd_info, &payload);

                            /* Mark buffer as BUSY - it will not be release */
                            ret = RET_BUSY;
                        }
                    }
                    else
                    {
                        ret = RET_INVALID_PARAMETER_1;
                    }
                    break;

                case TR_ZCL_STATUS_WAIT_FOR_DATA:
                {
                    zb_uint32_t delay32;

                    /* TODO: implement attribute value check in zb_zcl_check_attr_value (0 - 0x258)
                       NOTE: it seems error in the spec, too low upper limit;
                       possibly it should be 6000? */

                    /* update attribute from server command:
                     * 11.10.10  MinimumBlockPeriod Attribute
                     * This attribute SHALL reflect the minimum delay between Image Block Request commands generated by the
                     * client in milliseconds. The value of this attribute SHALL be updated when the rate is changed by the server,
                     * but SHOULD reflect the client default when an upgrade is not in progress or a server does not support this
                     * feature. */
                    ret = ((TR_ZCL_STATUS_SUCCESS == zb_zcl_set_attr_val(endpoint,
                                                                         TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                                                                         TR_ZCL_CLUSTER_CLIENT_ROLE,
                                                                         TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_MINIMUM_BLOCK_REQUEST_PERIOD_ID,
                                                                         (zb_uint8_t*)&payload.response.wait_for_data.delay,
                                                                         ZB_FALSE))
                 ? RET_OK
                 : RET_ERROR);

                    if (ret == RET_OK)
                    {
                        /* OTA spec 6.10.8.4
                           if (request_time - current_time) == 0, use BlockRequestDelay value to wait */

                        delay32 = ZB_TIME32_SUBTRACT(payload.response.wait_for_data.request_time,
                                                     payload.response.wait_for_data.current_time);

                        /* request/current time is sent in UTC (seconds), translate it to ms */
                        delay32 = ZB_SECONDS_TO_MILLISECONDS(delay32);

                        if (delay32 == 0)
                        {
                            /* if time delta is zero, use BlockRequestDelay value */
                            delay32 = payload.response.wait_for_data.delay;
                        }

                        /* re-send query next block */
                        /* TODO: ImageBlockResp may also be received as a response
                         * to ImagePageReq, in this case ImagePageReq should be resent */
                        ZB_MEMCPY(ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t), &cmd_info, sizeof(zb_zcl_parsed_hdr_t));
                        zb_zcl_ota_upgrade_send_block_request(param, delay32);

                        ret = RET_BUSY;
                        /* cancel resend */
                        cancel_resend_buffer();
                    }
                }
                break;

                case TR_ZCL_STATUS_ABORT:
                    zcl_ota_abort(endpoint, param);
                    break;
            }
        }
    }

    return ret;
}

/* public API */
void zb_zcl_ota_upgrade_send_upgrade_end_req(zb_uint8_t param,
                                             zb_uint8_t upgrade_status)
{
    zb_zcl_ota_upgrade_image_block_res_t payload;
    zb_zcl_parse_status_t                status;
    zb_zcl_parsed_hdr_t                  cmd_info;

    ZB_BZERO(&payload, sizeof(payload));

    ZB_MEMCPY(&cmd_info, ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t), sizeof(zb_zcl_parsed_hdr_t));

    ZB_ZCL_OTA_UPGRADE_GET_IMAGE_BLOCK_RES(&payload, param, status);

    if (status != ZB_ZCL_PARSE_STATUS_SUCCESS)
    {
        zb_buf_free(param);
    }
    else
    {
        /* Finish OTA process */
        zb_zcl_ota_upgrade_end(param,
                               (upgrade_status == ZB_ZCL_OTA_UPGRADE_STATUS_OK ?
                                TR_ZCL_STATUS_SUCCESS : TR_ZCL_STATUS_INVALID_IMAGE),
                               &cmd_info,
                               &payload);
    }
}

// Upgrade End Response command
static zb_ret_t upgrade_end_resp_handler(zb_uint8_t param)
{
    zb_ret_t                             ret = RET_OK;
    zb_zcl_ota_upgrade_upgrade_end_res_t payload;
    zb_zcl_parse_status_t                status;
    zb_zcl_parsed_hdr_t                  cmd_info;
    zb_uint8_t                           user_ret;

    ZB_MEMCPY(&cmd_info, ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t), sizeof(zb_zcl_parsed_hdr_t));

    ZB_ZCL_OTA_UPGRADE_GET_UPGRADE_END_RES(&payload, param, status);

    if (status != ZB_ZCL_PARSE_STATUS_SUCCESS)
    {
        ret = RET_INVALID_PARAMETER_1;
    }
    else
    {
        zb_uint8_t endpoint = ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint;

        /* OTA spec 6.10.10.4: examine the manufacturer code, image
           type and file version to verify that they match its own. If
           the received values do not match its own values or they are
           not wild card values, then it shall discard the command and
           no further processing shall continue.

           TODO: make correct check - take into account wildcard values
         */
        if (payload.manufacturer == tr_ota_client_info.mfg_id &&
            payload.image_type == tr_ota_client_info.image_type &&
            payload.file_version == ZCL_CTX().ota_cli.ota_dfv &&
            (payload.current_time <= payload.upgrade_time || ZB_ZCL_OTA_UPGRADE_UPGRADE_TIME_DEF_VALUE == payload.upgrade_time))
        {
            if (payload.upgrade_time == ZB_ZCL_OTA_UPGRADE_UPGRADE_TIME_DEF_VALUE)
            {
                tr_zcl_ota_upgrade_set_ota_status(endpoint, ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_WAITING_UPGRADE);
            }
            else
            {
                // call User App
                user_ret = ota_upgrade_mark_fw_ok();

                switch (user_ret)
                {
                    case ZB_ZCL_OTA_UPGRADE_STATUS_REQUIRE_MORE_IMAGE:
                        tr_zcl_ota_upgrade_set_ota_status(endpoint, ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_WAIT_FOR_MORE);

                        // send
                        ZB_ZCL_OTA_UPGRADE_SEND_UPGRADE_END_REQ(param,
                                                                ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).source.u.short_addr,
                                                                ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                                                ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).src_endpoint,
                                                                ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
                                                                ZB_AF_HA_PROFILE_ID,
                                                                ZB_FALSE,
                                                                NULL,
                                                                TR_ZCL_STATUS_REQUIRE_MORE_IMAGE,
                                                                payload.manufacturer,
                                                                payload.image_type,
                                                                payload.file_version);

                        ret = RET_BUSY;
                        break;

                    case ZB_ZCL_OTA_UPGRADE_STATUS_OK:
                        tr_zcl_ota_upgrade_set_ota_status(endpoint, ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_COUNT_DOWN);

                        if (ZB_ZCL_OTA_UPGRADE_UPGRADE_TIME_DEF_VALUE == payload.upgrade_time)
                        {
                            zb_zcl_ota_upgrade_file_upgraded(endpoint);
                            zb_osif_upgrade_now();
                        }
                        else
                        {
                            *ZB_BUF_GET_PARAM(param, zb_uint8_t) = endpoint;

                            ZB_SCHEDULE_ALARM(zb_zcl_ota_upgrade_finish_upgrade,
                                              param,
                                              (payload.upgrade_time - payload.current_time) * ZB_TIME_ONE_SECOND);
                            ret = RET_BUSY;
                        }
                        break;

                    case ZB_ZCL_OTA_UPGRADE_STATUS_ERROR:
                    default:
                        tr_zcl_ota_upgrade_set_ota_status(endpoint, ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_NORMAL);

                        // schedule the query next image requests to start up again
                        schedule_qnir();
                        break;
                }
            }
        }
        else
        {
            ret = RET_INVALID_PARAMETER_2;
        }
    }

    return ret;
}

// Query Specific File Response command
static zb_ret_t query_specific_file_resp_handler(zb_uint8_t param)
{
    zb_ret_t                                     ret = RET_OK;
    zb_zcl_ota_upgrade_query_specific_file_res_t payload;
    zb_zcl_parse_status_t                        status;
    zb_zcl_parsed_hdr_t                          cmd_info;
    zb_uint32_t                                  value;

    ZB_MEMCPY(&cmd_info, ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t), sizeof(zb_zcl_parsed_hdr_t));

    ZB_ZCL_OTA_UPGRADE_GET_QUERY_SPECIFIC_FILE_RES(&payload, param, status);

    if (status != ZB_ZCL_PARSE_STATUS_SUCCESS)
    {
        ret = RET_INVALID_PARAMETER_1;
    }
    else
    {
        zb_uint8_t endpoint = ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint;

        switch (payload.status)
        {
            case TR_ZCL_STATUS_SUCCESS:
                if (payload.manufacturer == tr_ota_client_info.mfg_id &&
                    payload.image_type == tr_ota_client_info.image_type &&
                    ZB_ZCL_OTA_UPGRADE_VERSION_CMP(payload.file_version, tr_ota_client_info.fw_version))
                {
                    zb_uint16_t delay = tr_zcl_ota_upgrade_get16(endpoint, TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_MINIMUM_BLOCK_REQUEST_PERIOD_ID);

                    // set attribute
                    value = 0;
                    zb_zcl_set_attr_val_manuf(endpoint,
                                              TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                                              TR_ZCL_CLUSTER_CLIENT_ROLE,
                                              TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_FILE_OFFSET_ID,
                                              ZB_ZCL_NON_MANUFACTURER_SPECIFIC,
                                              (zb_uint8_t*)&value,
                                              ZB_FALSE);

                    ZCL_CTX().ota_cli.ota_dfv = payload.file_version;

                    tr_ota_client_info.download_file_size = payload.image_size;

                    /* Backup period attribute */
                    ZCL_CTX().ota_cli.ota_period_backup = delay;

                    tr_zcl_ota_upgrade_set_ota_status(endpoint, ZB_ZCL_OTA_UPGRADE_IMAGE_STATUS_DOWNLOADING);

                    // send query block: offset 0
                    ZB_MEMCPY(ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t), &cmd_info, sizeof(zb_zcl_parsed_hdr_t));
                    zb_zcl_ota_upgrade_send_block_request(param, delay);

                    ret = RET_BUSY;
                }
                break;

            case TR_ZCL_STATUS_NO_IMAGE_AVAILABLE:
                break;
        }
    }

    return ret;
}

static zb_bool_t process_ota_client_specific_commands(zb_uint8_t param)
{
    zb_bool_t           processed = ZB_TRUE;
    zb_zcl_parsed_hdr_t cmd_info;
    zb_ret_t            status = RET_OK;

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);

    ZB_ASSERT(TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID == cmd_info.cluster_id);

    switch (cmd_info.cmd_id)
    {
        case TR_ZCL_CMD_IMAGE_NOTIFY_ID:
            ZB_ASSERT(ZB_ZCL_FRAME_DIRECTION_TO_CLI == cmd_info.cmd_direction);

            // invoke the app image notify callback. if it returns false, do NOT process the image notify
            if (tr_over_the_air_bootloading_client_image_notify_cb())
            {
                status = image_notify_handler(param);
            }
            break;

        case TR_ZCL_CMD_QUERY_NEXT_IMAGE_RESPONSE_ID:
        {
            ZB_ASSERT(ZB_ZCL_FRAME_DIRECTION_TO_CLI == cmd_info.cmd_direction);

            zb_zcl_ota_upgrade_query_next_image_res_t qnir_resp = { 0 };
            zb_zcl_parse_status_t                     parse_status;

            ZB_ZCL_OTA_UPGRADE_GET_QUERY_NEXT_IMAGE_RES(&qnir_resp, param, parse_status);

            if (parse_status != ZB_ZCL_PARSE_STATUS_SUCCESS)
            {
                status = RET_INVALID_PARAMETER_1;
            }
            else
            {
                zb_bool_t cb_status = ZB_TRUE;

                // invoke the app QNIR response callback if there is an image available.
                if (qnir_resp.status == TR_ZCL_STATUS_SUCCESS)
                {
                    cb_status = tr_over_the_air_bootloading_client_query_next_image_resp_cb(qnir_resp.file_version,
                                                                                            qnir_resp.image_type,
                                                                                            qnir_resp.manufacturer);
                }

                // if the cb returned true, process the query next image response
                if (cb_status)
                {
                    status = query_next_image_resp_handler(param);
                }
            }
            break;
        }

        case TR_ZCL_CMD_IMAGE_BLOCK_RESPONSE_ID:
        {
            ZB_ASSERT(ZB_ZCL_FRAME_DIRECTION_TO_CLI == cmd_info.cmd_direction);

            if (tr_ota_client_info.img_block_req_sent)
            {
                /* We have pending request, do not handle new block immediately */
                if (tr_ota_client_info.pending_img_block_resp)
                {
                    /* Yes, we will skip this block. Another block is already pending. */
                    zb_buf_free(param);
                }
                else
                {
                    tr_ota_client_info.pending_img_block_resp = param;
                }
                return ZB_TRUE;         /* Notify ZCL that command is processed */
            }
            else
            {
                status = image_block_resp_handler(param);
            }
        }
        break;

        case TR_ZCL_CMD_UPGRADE_END_RESPONSE_ID:
            ZB_ASSERT(ZB_ZCL_FRAME_DIRECTION_TO_CLI == cmd_info.cmd_direction);

            // invoke the app upgrade end response callback. if it returns false, do NOT process the response
            if (tr_over_the_air_bootloading_client_upgrade_end_resp_cb())
            {
                status = upgrade_end_resp_handler(param);
            }
            break;

        case TR_ZCL_CMD_QUERY_SPECIFIC_FILE_RESPONSE_ID:
            ZB_ASSERT(ZB_ZCL_FRAME_DIRECTION_TO_CLI == cmd_info.cmd_direction);
            status = query_specific_file_resp_handler(param);
            break;

        case TR_ZCL_CMD_DEFAULT_RESPONSE_ID:
        {
            zb_zcl_default_resp_payload_t *default_res;
            ZB_ASSERT(ZB_ZCL_FRAME_DIRECTION_TO_CLI == cmd_info.cmd_direction);

            default_res = ZB_ZCL_READ_DEFAULT_RESP(param);

            // don't send a default response to a default response
            cmd_info.disable_default_response = ZB_TRUE;

            switch (default_res->status)
            {
                case TR_ZCL_STATUS_ABORT:
                    /* This behaviour is implied by OTA-TC-14C */
                    zcl_ota_abort(ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint, param);
                    break;

                case TR_ZCL_STATUS_NO_IMAGE_AVAILABLE:
                    // LCD 10/1/24 I saw this behavior after deleting the file from the server
                    // during an OTA upgrade. The server was sending a default response with
                    // "no image available" for image block requests.
                    zcl_ota_abort(ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint, param);
                    break;

                default:
                    /* No action needs to be performed */
                    break;
            }
            break;
        }

        default:
            processed = ZB_FALSE;
            break;
    }

    if (processed)
    {
        if ((ZB_NWK_IS_ADDRESS_BROADCAST(cmd_info.addr_data.common_data.dst_addr) ||
             (cmd_info.disable_default_response)) && status == RET_OK)
        {
            zb_buf_free(param);
        }
        /* Fixed according to CCB 2519, malformed command should be ignored, default response should
         * not be sent, see ZCL8 spec subclause 11.13.3.5.1 */
        else if ((cmd_info.cmd_id == TR_ZCL_CMD_IMAGE_NOTIFY_ID) && (ZB_NWK_IS_ADDRESS_BROADCAST(cmd_info.addr_data.common_data.dst_addr)) &&
                 (status == RET_INVALID_PARAMETER_1 || status == RET_INVALID_PARAMETER_2))
        {
            zb_buf_free(param);
        }
        else if (status != RET_BUSY)
        {
            ZB_ZCL_SEND_DEFAULT_RESP_DIRECTION(param,
                                               ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).source.u.short_addr,
                                               ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                               ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).src_endpoint,
                                               ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
                                               cmd_info.profile_id,
                                               TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                                               cmd_info.seq_number,
                                               cmd_info.cmd_id,
                                               (status == RET_OK ? TR_ZCL_STATUS_SUCCESS :
                                                (status == RET_INVALID_PARAMETER_2 ? TR_ZCL_STATUS_MALFORMED_COMMAND : TR_ZCL_STATUS_INVALID_FIELD)),
                                               (ZB_ZCL_FRAME_DIRECTION_TO_CLI == cmd_info.cmd_direction ?
                                                ZB_ZCL_FRAME_DIRECTION_TO_SRV :
                                                ZB_ZCL_FRAME_DIRECTION_TO_CLI));
        }
    }

    return processed;
}

static zb_bool_t ota_upgrade_client_cluster_handler(zb_uint8_t param)
{
    if (ZB_ZCL_GENERAL_GET_CMD_LISTS_PARAM == param)
    {
        ZCL_CTX().zb_zcl_cluster_cmd_list = &gs_ota_upgrade_client_cmd_list;
        return ZB_TRUE;
    }
    return process_ota_client_specific_commands(param);
}
