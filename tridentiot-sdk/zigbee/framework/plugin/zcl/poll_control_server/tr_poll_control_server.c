/// ****************************************************************************
/// @file tr_poll_control_server.c
///
/// @brief ZCL POLL CONTROL cluster server implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_poll_control_server.h"
#include "tr_nvram_attr.h"

#define PLUGIN_NAME (zb_char_t*)("Poll Control Server")

// client address data
zb_zcl_poll_control_srv_cfg_data_t g_addr_data;

// forward declarations
void poll_control_server_start_check_in(zb_uint8_t param);
void poll_control_server_check_in_non_response(zb_uint8_t endpoint);
zb_ret_t poll_control_server_check_value(zb_uint16_t attr_id,
                                         zb_uint8_t  endpoint,
                                         zb_uint8_t  *value);
zb_bool_t poll_control_server_process_specific_commands(zb_uint8_t param);
static void poll_control_server_write_attr_hook(zb_uint8_t  endpoint,
                                                zb_uint16_t attr_id,
                                                zb_uint8_t  *new_value,
                                                zb_uint16_t manuf_code);
static void poll_control_server_check_binding(zb_bufid_t    param,
                                              zb_uint8_t    src_endpoint,
                                              zb_callback_t cb);

// macros for check in response timeouts
#define ZB_CHECK_IN_NO_RESPONSE_INTERVAL_MS 7680
#define ZB_CHECK_IN_NO_RESPONSE_INTERVAL    ZB_MILLISECONDS_TO_BEACON_INTERVAL(ZB_CHECK_IN_NO_RESPONSE_INTERVAL_MS)

#ifdef POLL_CONTROL_SERVER_SUPPORTED_COMMANDS_RECEIVE
static zb_uint8_t gs_poll_control_server_received_commands[] =
{
    POLL_CONTROL_SERVER_SUPPORTED_COMMANDS_RECEIVE
};
#endif

#ifdef POLL_CONTROL_SERVER_SUPPORTED_COMMANDS_SEND
static zb_uint8_t gs_poll_control_server_generated_commands[] =
{
    POLL_CONTROL_SERVER_SUPPORTED_COMMANDS_SEND
};
#endif

static zb_discover_cmd_list_t gs_poll_control_server_cmd_list =
{
#ifdef POLL_CONTROL_SERVER_SUPPORTED_COMMANDS_RECEIVE
    sizeof(gs_poll_control_server_received_commands),  gs_poll_control_server_received_commands,
#else
    0,                                                 NULL,
#endif

#ifdef POLL_CONTROL_SERVER_SUPPORTED_COMMANDS_SEND
    sizeof(gs_poll_control_server_generated_commands), gs_poll_control_server_generated_commands
#else
    0,                                                 NULL
#endif
};

// initialize the poll control server functions
void tr_poll_control_server_init(void)
{
    zb_zcl_add_cluster_handlers(TR_ZCL_CLUSTER_POLL_CONTROL_ID,
                                TR_ZCL_CLUSTER_SERVER_ROLE,
                                poll_control_server_check_value,
                                poll_control_server_write_attr_hook,
                                poll_control_server_process_specific_commands);

    tr_poll_control_server_init_cb();
}

// verify attribute values
zb_ret_t poll_control_server_check_value(zb_uint16_t attr_id,
                                         zb_uint8_t  endpoint,
                                         zb_uint8_t  *value)
{
    zb_bool_t     ret     = ZB_TRUE;
    zb_ret_t      ret_val = RET_ERROR;
    zb_zcl_attr_t *attr_desc;
    zb_uint32_t   value32 = 0;
    zb_uint16_t   value16 = 0;
    ZB_ASSIGN_UINT32(&value32, value);
    ZB_ASSIGN_UINT16(&value16, value);

    switch (attr_id)
    {
        case TR_ZCL_ATTR_POLL_CONTROL_CHECK_IN_INTERVAL_ID:
            // if 0, then no check ins and no need to verify any further
            if (ZB_ZCL_POLL_CONTROL_CHECKIN_INTERVAL_NO_CHECK_IN_VALUE == value32)
            {
                ret = ZB_TRUE;
            }
            else
            {
                // make sure check in interval is less than or equal to the limit from the spec
                ret = (value32 <= ZB_ZCL_POLL_CONTROL_CHECKIN_INTERVAL_MAX_VALUE) ? ZB_TRUE : ZB_FALSE;

                // make sure it is greater than or equal to the min interval value, if it is there
                attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                                   TR_ZCL_CLUSTER_POLL_CONTROL_ID,
                                                   TR_ZCL_CLUSTER_SERVER_ROLE,
                                                   TR_ZCL_ATTR_POLL_CONTROL_CHECK_IN_INTERVAL_MIN_ID);

                if (attr_desc != NULL)
                {
                    zb_uint32_t min_value = ZB_ZCL_GET_ATTRIBUTE_VAL_32(attr_desc);
                    ret                  &= value32 >= min_value;
                }

                // make sure check in interval is greater than or equal to the long poll interval
                attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                                   TR_ZCL_CLUSTER_POLL_CONTROL_ID,
                                                   TR_ZCL_CLUSTER_SERVER_ROLE,
                                                   TR_ZCL_ATTR_POLL_CONTROL_LONG_POLL_INTERVAL_ID);

                if (attr_desc != NULL)
                {
                    zb_uint32_t long_poll = ZB_ZCL_GET_ATTRIBUTE_VAL_32(attr_desc);
                    ret                  &= value32 >= long_poll;
                }
            }
            break;

        case TR_ZCL_ATTR_POLL_CONTROL_LONG_POLL_INTERVAL_ID:
            // make sure long poll interval is greater than or equal to the min from the spec
            ret = (ZB_ZCL_POLL_CONTROL_LONG_POLL_INTERVAL_MIN_VALUE <= value32) &&
                  (value32 <= ZB_ZCL_POLL_CONTROL_LONG_POLL_INTERVAL_MAX_VALUE)
              ? ZB_TRUE : ZB_FALSE;

            // make sure it is greater than or equal to the min interval value, if there is one
            attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                               TR_ZCL_CLUSTER_POLL_CONTROL_ID,
                                               TR_ZCL_CLUSTER_SERVER_ROLE,
                                               TR_ZCL_ATTR_POLL_CONTROL_LONG_POLL_INTERVAL_MIN_ID);

            if (attr_desc != NULL)
            {
                zb_uint32_t poll_min = ZB_ZCL_GET_ATTRIBUTE_VAL_32(attr_desc);
                ret                 &= poll_min <= value32;
            }

            // make sure it is less than or equal to the check in interval
            attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                               TR_ZCL_CLUSTER_POLL_CONTROL_ID,
                                               TR_ZCL_CLUSTER_SERVER_ROLE,
                                               TR_ZCL_ATTR_POLL_CONTROL_CHECK_IN_INTERVAL_ID);

            if (attr_desc != NULL)
            {
                zb_uint32_t check_in_interval = ZB_ZCL_GET_ATTRIBUTE_VAL_32(attr_desc);
                ret                          &= value32 <= check_in_interval;
            }

            // make sure it is greater than or equal to the short poll interval
            attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                               TR_ZCL_CLUSTER_POLL_CONTROL_ID,
                                               TR_ZCL_CLUSTER_SERVER_ROLE,
                                               TR_ZCL_ATTR_POLL_CONTROL_SHORT_POLL_INTERVAL_ID);

            if (attr_desc != NULL)
            {
                zb_uint16_t short_poll_interval = ZB_ZCL_GET_ATTRIBUTE_VAL_32(attr_desc);
                ret                            &= short_poll_interval <= value32;
            }

            break;

        case TR_ZCL_ATTR_POLL_CONTROL_SHORT_POLL_INTERVAL_ID:
            // make sure the short poll interval is greater than or equal to the min from the spc
            ret = (ZB_ZCL_POLL_CONTROL_SHORT_POLL_INTERVAL_MIN_VALUE <= value16) ? ZB_TRUE : ZB_FALSE;

            // make sure it is less than or equal to the long poll interval
            attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                               TR_ZCL_CLUSTER_POLL_CONTROL_ID,
                                               TR_ZCL_CLUSTER_SERVER_ROLE,
                                               TR_ZCL_ATTR_POLL_CONTROL_LONG_POLL_INTERVAL_ID);

            if (attr_desc != NULL)
            {
                zb_uint32_t long_poll = ZB_ZCL_GET_ATTRIBUTE_VAL_32(attr_desc);
                ret                  &= value16 <= long_poll;
            }
            break;

        case TR_ZCL_ATTR_POLL_CONTROL_FAST_POLL_TIMEOUT_ID:
            // make sure the fast poll timeout is greater than or equal to the min from the spec
            ret = (ZB_ZCL_POLL_CONTROL_FAST_POLL_TIMEOUT_MIN_VALUE <= value16) ? ZB_TRUE : ZB_FALSE;

            // make sure it is less than or equal to the max from the attribute
            attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                               TR_ZCL_CLUSTER_POLL_CONTROL_ID,
                                               TR_ZCL_CLUSTER_SERVER_ROLE,
                                               TR_ZCL_ATTR_POLL_CONTROL_FAST_POLL_TIMEOUT_MAX_ID);

            if (attr_desc != NULL)
            {
                zb_uint16_t poll_max = ZB_ZCL_GET_ATTRIBUTE_VAL_16(attr_desc);
                ret                 &= value16 <= poll_max;
            }
            break;

        default:
            break;
    }

    if (ret)
    {
        ret_val = RET_OK;
    }
    return ret_val;
}

// restart check in cycle with passed interval
static void poll_control_start_internal(zb_uint8_t  param,
                                        zb_uint32_t new_checkin_interval_qsec)
{
    // let go of the buffer if there is one
    if (param != 0U)
    {
        zb_buf_free(param);
    }

    // buffer is not reused here to prevent buffer blocking for pretty long check in interval
    ZB_SCHEDULE_ALARM(poll_control_server_start_check_in, 0, ZB_QUARTERECONDS_TO_BEACON_INTERVAL(new_checkin_interval_qsec));
}

// start check in cycle with interval taken from poll control cluster
void poll_control_start(zb_uint8_t param,
                        zb_uint8_t endpoint)
{
    zb_zcl_attr_t   *attr_desc;
    tr_conn_state_e conn_state = tr_get_connection_state();

    // only start if we are on a network
    if ((conn_state == TR_CONN_STATE_NWK_STEERING_ATTEMPT_SUCCESS) || (conn_state == TR_CONN_STATE_JOINED_NETWORK))
    {
        // setup the long poll interval from the attribute
        attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                           TR_ZCL_CLUSTER_POLL_CONTROL_ID,
                                           TR_ZCL_CLUSTER_SERVER_ROLE,
                                           TR_ZCL_ATTR_POLL_CONTROL_LONG_POLL_INTERVAL_ID);
        ZB_ASSERT(attr_desc);
        zb_zdo_pim_set_long_poll_interval(ZB_QUARTERECONDS_TO_MSEC(ZB_ZCL_GET_ATTRIBUTE_VAL_32(attr_desc)));

        // setup the short poll interval from the attribute
        attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                           TR_ZCL_CLUSTER_POLL_CONTROL_ID,
                                           TR_ZCL_CLUSTER_SERVER_ROLE,
                                           TR_ZCL_ATTR_POLL_CONTROL_SHORT_POLL_INTERVAL_ID);
        ZB_ASSERT(attr_desc);
        zb_zdo_pim_set_fast_poll_interval(ZB_QUARTERECONDS_TO_MSEC(ZB_ZCL_GET_ATTRIBUTE_VAL_16(attr_desc)));

        // setup the short poll timeout from the attribute
        attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                           TR_ZCL_CLUSTER_POLL_CONTROL_ID,
                                           TR_ZCL_CLUSTER_SERVER_ROLE,
                                           TR_ZCL_ATTR_POLL_CONTROL_FAST_POLL_TIMEOUT_ID);
        ZB_ASSERT(attr_desc);
        zb_zdo_pim_set_fast_poll_timeout(ZB_QUARTERECONDS_TO_MSEC(ZB_ZCL_GET_ATTRIBUTE_VAL_16(attr_desc)));

        attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                           TR_ZCL_CLUSTER_POLL_CONTROL_ID,
                                           TR_ZCL_CLUSTER_SERVER_ROLE,
                                           TR_ZCL_ATTR_POLL_CONTROL_CHECK_IN_INTERVAL_ID);
        ZB_ASSERT(attr_desc);

        // start the check in cycle with the check in interval from the attirbute
        poll_control_start_internal(param, ZB_ZCL_GET_ATTRIBUTE_VAL_32(attr_desc));

        // let the app know
        tr_poll_control_server_started_cb();
    }
}

// stop check in
zb_uint8_t poll_control_stop(void)
{
    zb_uint8_t canceled_param = 0;
    ZB_SCHEDULE_ALARM_CANCEL_AND_GET_BUF(
        poll_control_server_start_check_in,
        ZB_ALARM_ANY_PARAM,
        &canceled_param);

    return canceled_param;
}

// this will start the poll control server once we are on a network
// TODO: LCD 1/21/25 should this also stop the poll control server based on network connection state?
void tr_poll_control_server_connection_state_cb(tr_conn_state_e conn_state)
{
    switch (conn_state)
    {
        case TR_CONN_STATE_NWK_STEERING_ATTEMPT_SUCCESS:
        case TR_CONN_STATE_JOINED_NETWORK:
            // we have joined, start the poll control cluster
            poll_control_start(0, 1);
            break;

        default:
            break;
    }
}

// check in interval was written, update the alarm event if needed
void write_attr_check_in_interval_hook(zb_uint8_t endpoint,
                                       zb_uint8_t *new_value_ptr)
{
    zb_time_t   new_interval;
    zb_uint32_t new_val;
    zb_uint8_t  canceled_param = 0;

    ZVUNUSED(endpoint);

    ZB_MEMCPY(&new_val, new_value_ptr, sizeof(zb_uint32_t));
    new_interval = ZB_QUARTERECONDS_TO_BEACON_INTERVAL(new_val);
    ZVUNUSED(new_interval);

    // stop check ins for the moment
    canceled_param = poll_control_stop();

    if (new_val != ZB_ZCL_POLL_CONTROL_CHECKIN_INTERVAL_NO_CHECK_IN_VALUE)
    {
        // if new interval is not 0, restart check ins with new interval
        poll_control_start_internal(canceled_param, new_val);
    }
    else
    {
        // new interval is 0, not restarting, free the buffer
        if (canceled_param)
        {
            zb_buf_free(canceled_param);
        }
    }
}

static void poll_control_send_default_response(zb_bufid_t          param,
                                               zb_zcl_parsed_hdr_t *cmd_info,
                                               zb_zcl_status_t     status)
{
    if (cmd_info->disable_default_response && status == TR_ZCL_STATUS_SUCCESS)
    {
        zb_buf_free(param);
    }
    else
    {
        ZB_ZCL_SEND_DEFAULT_RESP(param,
                                 ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).source.u.short_addr,
                                 ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                 ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).src_endpoint,
                                 ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).dst_endpoint,
                                 cmd_info->profile_id,
                                 TR_ZCL_CLUSTER_POLL_CONTROL_ID,
                                 cmd_info->seq_number,
                                 cmd_info->cmd_id,
                                 status);
    }
}

static zb_size_t get_aligned_size(zb_size_t size)
{
    return ((size + ZB_BUF_ALLOC_ALIGN - 1) / ZB_BUF_ALLOC_ALIGN) * ZB_BUF_ALLOC_ALIGN;
}

static void check_in_res_handler_pack_params(zb_bufid_t                         param,
                                             zb_zcl_poll_control_check_in_res_t *payload,
                                             zb_zcl_parsed_hdr_t                *cmd_info)
{
    zb_size_t  payload_size_aligned = get_aligned_size(sizeof(*payload));
    zb_size_t  params_size          = payload_size_aligned + sizeof(*cmd_info);
    zb_uint8_t *ptr                 = NULL;

    ptr = zb_buf_initial_alloc(param, (zb_uint_t)params_size);

    ZB_MEMCPY(ptr, payload, sizeof(*payload));
    ptr += payload_size_aligned;

    ZB_MEMCPY(ptr, cmd_info, sizeof(*cmd_info));
}

static void check_in_res_handler_unpack_params(zb_bufid_t                         param,
                                               zb_zcl_poll_control_check_in_res_t *payload,
                                               zb_zcl_parsed_hdr_t                *cmd_info)
{
    zb_size_t  payload_size_aligned = get_aligned_size(sizeof(*payload));
    zb_uint8_t *ptr                 = NULL;

    ptr = zb_buf_begin(param);

    ZB_MEMCPY(payload, ptr, sizeof(*payload));
    ptr += payload_size_aligned;

    ZB_MEMCPY(cmd_info, ptr, sizeof(*cmd_info));
}

// verify there is a binding to the client before processing a receive check in response
static void check_in_res_handler_check_binding_response_cb(zb_bufid_t param)
{
    zb_zcl_poll_control_check_in_res_t payload;
    zb_zcl_parsed_hdr_t                cmd_info            = { 0 };
    zb_aps_check_binding_resp_t        *check_binding_resp = NULL;
    zb_zcl_status_t                    status              = TR_ZCL_STATUS_SUCCESS;
    zb_uint8_t                         endpoint;

    check_binding_resp = ZB_BUF_GET_PARAM(param, zb_aps_check_binding_resp_t);

    if (!check_binding_resp->exists)
    {
        // there is no binding, respond with an error
        status = (zb_zcl_get_backward_compatible_statuses_mode() == ZB_ZCL_STATUSES_ZCL8_MODE) ?
                 TR_ZCL_STATUS_FAILURE : TR_ZCL_STATUS_ACTION_DENIED;
    }

    if (status == TR_ZCL_STATUS_SUCCESS)
    {
        check_in_res_handler_unpack_params(param, &payload, &cmd_info);
        endpoint = ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint;
    }

    if (status == TR_ZCL_STATUS_SUCCESS)
    {
        zb_time_t tmp;
        zb_ret_t  ret;

        // did the check in response come after the default fast poll period finished?
        ret = ZB_SCHEDULE_GET_ALARM_TIME(poll_control_server_check_in_non_response, endpoint, &tmp);

        if (ret != RET_OK)
        {
            status = TR_ZCL_STATUS_FAILURE;
        }
    }

    if (status == TR_ZCL_STATUS_SUCCESS)
    {
        // we are good, cancel the non-response alarm
        ZB_SCHEDULE_ALARM_CANCEL(poll_control_server_check_in_non_response, endpoint);

        if (payload.is_start)
        {
            // check in response said to start fast polling but first check the optional long poll timeout
            // if it is longer than the max long poll interval, send a default response with "invalid field"
            zb_zcl_attr_t *attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                                              TR_ZCL_CLUSTER_POLL_CONTROL_ID,
                                                              TR_ZCL_CLUSTER_SERVER_ROLE,
                                                              TR_ZCL_ATTR_POLL_CONTROL_FAST_POLL_TIMEOUT_MAX_ID);

            if (attr_desc != NULL)
            {
                if (payload.timeout > ZB_ZCL_GET_ATTRIBUTE_VAL_16(attr_desc))
                {
                    // the requested fast poll timeout is longer than our max fast poll timeout
                    status = TR_ZCL_STATUS_INVALID_FIELD;
                }
            }

            if (status == TR_ZCL_STATUS_SUCCESS)
            {
                // fast poll timeout is ok, keep going
                attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                                   TR_ZCL_CLUSTER_POLL_CONTROL_ID,
                                                   TR_ZCL_CLUSTER_SERVER_ROLE,
                                                   TR_ZCL_ATTR_POLL_CONTROL_FAST_POLL_TIMEOUT_ID);
                zb_time_t new_interval = ZB_QUARTERECONDS_TO_MSEC(payload.timeout);

                if (!new_interval)
                {
                    ZB_ASSERT(attr_desc);
                    new_interval = ZB_QUARTERECONDS_TO_MSEC(ZB_ZCL_GET_ATTRIBUTE_VAL_16(attr_desc));
                }

                // continue fast polling based on the timeout from the check in response OR the attribute if
                // fast poll timeout was 0 in the check in response
                zb_zdo_pim_stop_fast_poll(0);
                zb_zdo_pim_set_fast_poll_timeout(new_interval);
                zb_zdo_pim_start_fast_poll(0);
            }
            else
            {
                // check in response said to stop fast polling
                zb_zdo_pim_stop_fast_poll(0);
            }

#ifdef ZB_ZCL_ENABLE_WWAH_SERVER
            zb_zcl_wwah_bad_parent_recovery_signal(ZB_ZCL_WWAH_BAD_PARENT_RECOVERY_POLL_CONTROL_CHECK_IN_OK);
#endif
        }
    }

    poll_control_send_default_response(param, &cmd_info, status);
}

// check in response command
static zb_ret_t check_in_res_handler(zb_uint8_t param)
{
    zb_ret_t                           ret = RET_OK;
    zb_zcl_poll_control_check_in_res_t payload;
    zb_zcl_parse_status_t              status;
    zb_zcl_parsed_hdr_t                cmd_info;
    zb_uint8_t                         endpoint;

    ZB_MEMCPY(&cmd_info, ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t), sizeof(zb_zcl_parsed_hdr_t));

    endpoint = ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint;

    ZB_ZCL_POLL_CONTROL_GET_CHECK_IN_RES(&payload, param, status);

    if (status != ZB_ZCL_PARSE_STATUS_SUCCESS)
    {
        ret = RET_INVALID_PARAMETER_1;
    }

    // verify we have a binding to this client before we do anything
    if (ret == RET_OK)
    {
        check_in_res_handler_pack_params(param, &payload, &cmd_info);
        poll_control_server_check_binding(param,
                                          endpoint,
                                          check_in_res_handler_check_binding_response_cb);
        ret = RET_BUSY;
    }

    return ret;
}

static void fast_poll_stop_handler_send_default_response(zb_uint8_t param)
{
    zb_zcl_status_t status = (zb_zcl_get_backward_compatible_statuses_mode() == ZB_ZCL_STATUSES_ZCL8_MODE) ?
                             TR_ZCL_STATUS_FAILURE : TR_ZCL_STATUS_ACTION_DENIED;
    zb_zdo_pim_stop_fast_poll_extended_resp_t *resp     = ZB_BUF_GET_PARAM(param, zb_zdo_pim_stop_fast_poll_extended_resp_t);
    zb_zcl_parsed_hdr_t                       *cmd_info = NULL;

    if (resp->stop_result == ZB_ZDO_PIM_STOP_FAST_POLL_RESULT_STOPPED)
    {
        status = TR_ZCL_STATUS_SUCCESS;
    }

    cmd_info = (zb_zcl_parsed_hdr_t*)zb_buf_begin(param);
    poll_control_send_default_response(param, cmd_info, status);
}

// only accept fast poll stop commands if we have a binding with the client
static void fast_poll_stop_handler_check_binding_response_cb(zb_bufid_t param)
{
    zb_aps_check_binding_resp_t *check_binding_resp = NULL;

    check_binding_resp = ZB_BUF_GET_PARAM(param, zb_aps_check_binding_resp_t);

    if (check_binding_resp->exists)
    {
        // we have a binding, stop the fast polling
        zb_zdo_pim_stop_fast_poll_extended_req(param, fast_poll_stop_handler_send_default_response);
    }
    else
    {
        // no binding, send an error
        zb_zcl_parsed_hdr_t *cmd_info = NULL;

        cmd_info = (zb_zcl_parsed_hdr_t*)zb_buf_begin(param);
        poll_control_send_default_response(param,
                                           cmd_info,
                                           (zb_zcl_get_backward_compatible_statuses_mode() == ZB_ZCL_STATUSES_ZCL8_MODE) ?
                                           TR_ZCL_STATUS_FAILURE : TR_ZCL_STATUS_ACTION_DENIED);
    }
}

// handle the fast_poll_stop command
static zb_ret_t fast_poll_stop_handler(zb_uint8_t param)
{
    zb_zcl_parsed_hdr_t *cmd_info = NULL;
    zb_uint8_t          endpoint;

    // copy cmd_info so we can use it to send a response later
    cmd_info = zb_buf_initial_alloc(param, sizeof(zb_zcl_parsed_hdr_t));
    ZB_MEMCPY(cmd_info,
              ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t),
              sizeof(zb_zcl_parsed_hdr_t));

    endpoint = ZB_ZCL_PARSED_HDR_SHORT_DATA(cmd_info).dst_endpoint;

    // verify we have a binding to this client before we do anything
    poll_control_server_check_binding(param, endpoint, fast_poll_stop_handler_check_binding_response_cb);

    return RET_BUSY;
}

// set long poll interval command
static zb_ret_t set_long_poll_interval_handler(zb_uint8_t param)
{
    zb_ret_t                                     ret = RET_OK;
    zb_zcl_poll_control_set_long_poll_interval_t payload;
    zb_zcl_parse_status_t                        status;
    zb_zcl_parsed_hdr_t                          cmd_info;
    zb_uint8_t                                   endpoint;

    ZB_MEMCPY(&cmd_info, ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t), sizeof(zb_zcl_parsed_hdr_t));
    endpoint = ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint;

    ZB_ZCL_POLL_CONTROL_GET_SET_LONG_POLL_INTERVAL_REQ(&payload, param, status);

    if (status != ZB_ZCL_PARSE_STATUS_SUCCESS)
    {
        ret = RET_INVALID_PARAMETER_1;
    }
    else if (/*!*/ zb_zcl_check_attr_value(TR_ZCL_CLUSTER_POLL_CONTROL_ID,
                                           TR_ZCL_CLUSTER_SERVER_ROLE,
                                           endpoint,
                                           TR_ZCL_ATTR_POLL_CONTROL_LONG_POLL_INTERVAL_ID,
                                           (zb_uint8_t*)(&payload.interval)) == RET_ERROR)
    {
        ZB_ZCL_PROCESS_COMMAND_FINISH(param, &cmd_info, TR_ZCL_STATUS_INVALID_VALUE);
        ret = RET_BUSY; // not need send answer yet
    }
    else
    {
        zb_zcl_attr_t *attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                                          TR_ZCL_CLUSTER_POLL_CONTROL_ID,
                                                          TR_ZCL_CLUSTER_SERVER_ROLE,
                                                          TR_ZCL_ATTR_POLL_CONTROL_LONG_POLL_INTERVAL_ID);
        ZB_ASSERT(attr_desc);
        ZB_ZCL_SET_DIRECTLY_ATTR_VAL32(attr_desc, payload.interval);

        // update the value for the long poll interval in nvram if needed
        tr_check_for_attr_nvram_update(endpoint,
                                       TR_ZCL_CLUSTER_POLL_CONTROL_ID,
                                       TR_ZCL_CLUSTER_SERVER_ROLE,
                                       TR_ZCL_ATTR_POLL_CONTROL_LONG_POLL_INTERVAL_ID,
                                       ZB_ZCL_NON_MANUFACTURER_SPECIFIC);

        // start polling at the new interval
        zb_zdo_pim_set_long_poll_interval(ZB_QUARTERECONDS_TO_MSEC(payload.interval));
    }

    return ret;
}

// set short poll interval command
static zb_ret_t set_short_poll_interval_handler(zb_uint8_t param)
{
    zb_ret_t                                      ret = RET_OK;
    zb_zcl_poll_control_set_short_poll_interval_t payload;
    zb_zcl_parse_status_t                         status;
    zb_zcl_parsed_hdr_t                           cmd_info;
    zb_uint8_t                                    endpoint;

    ZB_MEMCPY(&cmd_info, ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t), sizeof(zb_zcl_parsed_hdr_t));
    endpoint = ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint;

    ZB_ZCL_POLL_CONTROL_GET_SET_SHORT_POLL_INTERVAL_REQ(&payload, param, status);

    if (status != ZB_ZCL_PARSE_STATUS_SUCCESS)
    {
        ret = RET_INVALID_PARAMETER_1;
    }
    else if (/*!*/ zb_zcl_check_attr_value(TR_ZCL_CLUSTER_POLL_CONTROL_ID,
                                           TR_ZCL_CLUSTER_SERVER_ROLE,
                                           endpoint,
                                           TR_ZCL_ATTR_POLL_CONTROL_SHORT_POLL_INTERVAL_ID,
                                           (zb_uint8_t*)(&payload.interval)) == RET_ERROR)
    {
        ZB_ZCL_PROCESS_COMMAND_FINISH(param, &cmd_info, TR_ZCL_STATUS_INVALID_VALUE);
        ret = RET_BUSY; // not need send answer yet
    }
    else
    {
        zb_zcl_attr_t *attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                                          TR_ZCL_CLUSTER_POLL_CONTROL_ID,
                                                          TR_ZCL_CLUSTER_SERVER_ROLE,
                                                          TR_ZCL_ATTR_POLL_CONTROL_SHORT_POLL_INTERVAL_ID);
        ZB_ASSERT(attr_desc);
        ZB_ZCL_SET_DIRECTLY_ATTR_VAL16(attr_desc, payload.interval);

        // update the value for the short poll interval in nvram if needed
        tr_check_for_attr_nvram_update(endpoint,
                                       TR_ZCL_CLUSTER_POLL_CONTROL_ID,
                                       TR_ZCL_CLUSTER_SERVER_ROLE,
                                       TR_ZCL_ATTR_POLL_CONTROL_SHORT_POLL_INTERVAL_ID,
                                       ZB_ZCL_NON_MANUFACTURER_SPECIFIC);

        zb_zdo_pim_set_fast_poll_interval(ZB_QUARTERECONDS_TO_MSEC(payload.interval));
    }

    return ret;
}

// process the poll control server received commands
zb_bool_t poll_control_server_process_specific_commands(zb_uint8_t param)
{
    zb_bool_t           processed = ZB_TRUE;
    zb_zcl_parsed_hdr_t cmd_info;
    zb_ret_t            status = RET_OK;
    zb_zcl_status_t     resp_code;

    if (ZB_ZCL_GENERAL_GET_CMD_LISTS_PARAM == param)
    {
        ZCL_CTX().zb_zcl_cluster_cmd_list = &gs_poll_control_server_cmd_list;
        return ZB_TRUE;
    }

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);

    ZB_ASSERT(TR_ZCL_CLUSTER_POLL_CONTROL_ID == cmd_info.cluster_id);
    ZB_ASSERT(ZB_ZCL_FRAME_DIRECTION_TO_SRV == cmd_info.cmd_direction);

    switch (cmd_info.cmd_id)
    {
        case TR_ZCL_CMD_CHECK_IN_RESPONSE_ID:
            if (tr_poll_control_server_check_in_response_cb(&cmd_info) == ZB_FALSE)
            {
                status = check_in_res_handler(param);
            }
            break;

        case TR_ZCL_CMD_FAST_POLL_STOP_ID:
            if (tr_poll_control_server_fast_poll_stop_cb(&cmd_info) == ZB_FALSE)
            {
                status = fast_poll_stop_handler(param);
            }
            break;

        case TR_ZCL_CMD_SET_LONG_POLL_INTERVAL_ID:
            if (tr_poll_control_server_set_long_poll_interval_cb(&cmd_info) == ZB_FALSE)
            {
                status = set_long_poll_interval_handler(param);
            }
            break;

        case TR_ZCL_CMD_SET_SHORT_POLL_INTERVAL_ID:
            if (tr_poll_control_server_set_short_poll_interval_cb(&cmd_info) == ZB_FALSE)
            {
                status = set_short_poll_interval_handler(param);
            }
            break;

        default:
            processed = ZB_FALSE;
            break;
    }

    if (processed && status != RET_BUSY)
    {
        switch (status)
        {
            case RET_OK:
                resp_code = TR_ZCL_STATUS_SUCCESS;
                break;

            case RET_INVALID_STATE:
                resp_code = (zb_zcl_get_backward_compatible_statuses_mode() == ZB_ZCL_STATUSES_ZCL8_MODE) ?
                            TR_ZCL_STATUS_FAILURE : TR_ZCL_STATUS_ACTION_DENIED;
                break;

            case RET_TIMEOUT:
                resp_code = TR_ZCL_STATUS_TIMEOUT;
                break;

            default:
                resp_code = TR_ZCL_STATUS_INVALID_FIELD;
                break;
        }

        poll_control_send_default_response(param, &cmd_info, resp_code);
    }

    return processed;
}

// no check in response, stop fast polling
void poll_control_server_check_in_non_response(zb_uint8_t endpoint)
{
    ZVUNUSED(endpoint);

    // we did not get a check in response, stop fast polling
    zb_zdo_pim_stop_fast_poll(0);

#ifdef ZB_ZCL_ENABLE_WWAH_SERVER
    zb_zcl_wwah_bad_parent_recovery_signal(ZB_ZCL_WWAH_BAD_PARENT_RECOVERY_POLL_CONTROL_CHECK_IN_FAILED);
#endif
}

// poll control check in send complete, verify status and do retries if needed
void poll_control_check_in_send_cb(zb_uint8_t param)
{
    zb_uint8_t                   endpoint;
    zb_zcl_command_send_status_t *cmd_send_status = ZB_BUF_GET_PARAM(param, zb_zcl_command_send_status_t);

    endpoint = get_endpoint_by_cluster(TR_ZCL_CLUSTER_POLL_CONTROL_ID, TR_ZCL_CLUSTER_SERVER_ROLE);

    if (cmd_send_status->status == 0)
    {
        // send was successful, restart the no check in response alarm
        ZB_SCHEDULE_ALARM_CANCEL(poll_control_server_check_in_non_response, endpoint);
        zb_zdo_pim_start_turbo_poll_packets(0);
        ZB_SCHEDULE_ALARM(poll_control_server_check_in_non_response, endpoint, ZB_CHECK_IN_NO_RESPONSE_INTERVAL);
    }

    // see if we need to do a retry
    if (g_addr_data.sending_cmd > 1)
    {
        ZB_SCHEDULE_CALLBACK(poll_control_server_start_check_in, param);
    }
    else
    {
        zb_buf_free(param);
    }
    g_addr_data.sending_cmd = 0;
}

// check client binding before sending a check in
static void check_in_handle_check_binding_confirm(zb_bufid_t param)
{
    zb_aps_check_binding_resp_t *check_binding_resp = NULL;
    zb_uint8_t                  endpoint;
    zb_uint32_t                 checkin_interval;
    zb_zcl_attr_t               *attr_desc;

    check_binding_resp = ZB_BUF_GET_PARAM(param, zb_aps_check_binding_resp_t);

    endpoint = get_endpoint_by_cluster(TR_ZCL_CLUSTER_POLL_CONTROL_ID, TR_ZCL_CLUSTER_SERVER_ROLE);
    ZB_ASSERT(endpoint);

    attr_desc = zb_zcl_get_attr_desc_a(
        endpoint,
        TR_ZCL_CLUSTER_POLL_CONTROL_ID,
        TR_ZCL_CLUSTER_SERVER_ROLE,
        TR_ZCL_ATTR_POLL_CONTROL_CHECK_IN_INTERVAL_ID);
    ZB_ASSERT(attr_desc);

    checkin_interval = ZB_ZCL_GET_ATTRIBUTE_VAL_32(attr_desc);

    if (check_binding_resp->exists)
    {
        zb_uint16_t dst_addr = 0;
        zb_uint8_t  dst_ep   = 0;
        zb_uint8_t  addr_mode;

        // we have a binding
        if (g_addr_data.poll_addr == ZB_ZCL_POLL_CTRL_INVALID_ADDR && g_addr_data.poll_ep == ZB_ZCL_POLL_INVALID_EP)
        {
            addr_mode = ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT;
        }
        else
        {
            dst_addr  = g_addr_data.poll_addr;
            dst_ep    = g_addr_data.poll_ep;
            addr_mode = ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
        }

        // send the check in
        ZB_ZCL_POLL_CONTROL_SEND_CHECK_IN_REQ(
            param,
            dst_addr,
            addr_mode,
            dst_ep,
            endpoint,
            ZB_AF_HA_PROFILE_ID,
            poll_control_check_in_send_cb);

        /* Set a flag - we are sending checkin command */
        g_addr_data.sending_cmd = 1;

        // set the alarm for not getting a check in response
        ZB_SCHEDULE_ALARM(poll_control_server_check_in_non_response, endpoint, ZB_CHECK_IN_NO_RESPONSE_INTERVAL);

        // start fast polling
        zb_zdo_pim_set_fast_poll_timeout(ZB_CHECK_IN_NO_RESPONSE_INTERVAL_MS);
        zb_zdo_pim_start_fast_poll(0);

    }
    else
    {
        // no binding, just free the buffer
        zb_buf_free(param);
    }

    // schedule next check in command
    ZB_SCHEDULE_ALARM(poll_control_server_start_check_in, 0, ZB_QUARTERECONDS_TO_BEACON_INTERVAL(checkin_interval));
}

static void poll_control_server_check_binding(zb_bufid_t    param,
                                              zb_uint8_t    src_endpoint,
                                              zb_callback_t cb)
{
    zb_aps_check_binding_req_t *check_binding_req = NULL;
    check_binding_req                             = ZB_BUF_GET_PARAM(param, zb_aps_check_binding_req_t);
    ZB_BZERO(check_binding_req, sizeof(*check_binding_req));

    check_binding_req->src_endpoint = src_endpoint;
    check_binding_req->cluster_id   = TR_ZCL_CLUSTER_POLL_CONTROL_ID;
    check_binding_req->response_cb  = cb;

    zb_aps_check_binding_request(param);
}

// start the process for sending a check in
void poll_control_server_start_check_in(zb_uint8_t param)
{
    zb_zcl_attr_t *attr_desc;
    zb_uint8_t    endpoint;
    zb_uint32_t   checkin_interval;
    zb_bool_t     skip_check_in = ZB_FALSE;

    if (param != 0)
    {
        // if param is 0 we will be re-entering this function after getting a buffer so don't call the callback yet
        // call the app callback, giving it a chance to say "no check in please"
        if (tr_poll_control_server_send_check_in_cb() == ZB_FALSE)
        {
            // app callback said not to send the check in
            skip_check_in = ZB_TRUE;
        }
    }

    endpoint = get_endpoint_by_cluster(TR_ZCL_CLUSTER_POLL_CONTROL_ID, TR_ZCL_CLUSTER_SERVER_ROLE);
    ZB_ASSERT(endpoint);

    attr_desc = zb_zcl_get_attr_desc_a(
        endpoint,
        TR_ZCL_CLUSTER_POLL_CONTROL_ID,
        TR_ZCL_CLUSTER_SERVER_ROLE,
        TR_ZCL_ATTR_POLL_CONTROL_CHECK_IN_INTERVAL_ID);
    ZB_ASSERT(attr_desc);

    checkin_interval = ZB_ZCL_GET_ATTRIBUTE_VAL_32(attr_desc);

    // don't send a check in if the interval is 0
    if (checkin_interval == ZB_ZCL_POLL_CONTROL_CHECKIN_INTERVAL_NO_CHECK_IN_VALUE)
    {
        skip_check_in = ZB_TRUE;
    }
    else
    {
        // is check in already in progress?
        if (g_addr_data.sending_cmd > 0)
        {
            if (g_addr_data.sending_cmd < 2)
            {
                g_addr_data.sending_cmd++;
            }
            skip_check_in = ZB_TRUE;
        }
    }

    if (skip_check_in == ZB_FALSE && param == 0)
    {
        zb_buf_get_out_delayed(poll_control_server_start_check_in);
    }
    else if (skip_check_in == ZB_FALSE)
    {
        // we are gonna send a check in, but verify the binding first
        poll_control_server_check_binding(param,
                                          ZB_ZCL_BROADCAST_ENDPOINT,
                                          check_in_handle_check_binding_confirm);
    }
    else
    {
        if (param != 0)
        {
            zb_buf_free(param);
        }
    }
}

// hook on write attribute
static void poll_control_server_write_attr_hook(zb_uint8_t  endpoint,
                                                zb_uint16_t attr_id,
                                                zb_uint8_t  *new_value,
                                                zb_uint16_t manuf_code)
{
    zb_uint32_t new_val = 0;
    ZB_MEMCPY(&new_val, new_value, sizeof(zb_uint32_t));

    (void)endpoint;
    (void)attr_id;
    (void)new_value;
    (void)manuf_code;

    if (attr_id == TR_ZCL_ATTR_POLL_CONTROL_CHECK_IN_INTERVAL_ID)
    {
        write_attr_check_in_interval_hook(endpoint, new_value);
    }
    else if (attr_id == TR_ZCL_ATTR_POLL_CONTROL_LONG_POLL_INTERVAL_ID)
    {
        zb_zdo_pim_set_long_poll_interval(ZB_QUARTERECONDS_TO_MSEC(new_val));
    }
    else if (attr_id == TR_ZCL_ATTR_POLL_CONTROL_SHORT_POLL_INTERVAL_ID)
    {
        zb_zdo_pim_set_fast_poll_interval(ZB_QUARTERECONDS_TO_MSEC(new_val));
    }

    tr_poll_control_server_write_attr_cb(endpoint, attr_id, new_value, manuf_code);
}

// we have a client, save the info to make it easier to access
zb_ret_t poll_control_set_client_addr(zb_uint8_t  local_ep,
                                      zb_uint16_t addr,
                                      zb_uint8_t  ep)
{
    zb_ret_t ret;

    if (ZB_ZDO_CHECK_CLUSTER_PERMISSION(addr, TR_ZCL_CLUSTER_POLL_CONTROL_ID))
    {
        g_addr_data.poll_addr = addr;
        g_addr_data.poll_ep   = ep;
        ret                   = RET_OK;
    }
    else
    {
        ret = RET_ERROR;
    }

    return ret;
}
