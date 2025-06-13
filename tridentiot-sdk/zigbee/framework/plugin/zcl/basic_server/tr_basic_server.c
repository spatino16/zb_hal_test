/// ****************************************************************************
/// @file tr_basic_server.c
///
/// @brief ZCL BASIC cluster server implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_basic_server.h"
#include "zb_zcl_diagnostics.h"

#define PLUGIN_NAME (zb_char_t*)("Basic Server")

#ifdef BASIC_SERVER_SUPPORTED_COMMANDS_RECEIVE
static zb_uint8_t gs_basic_server_received_commands[] =
{
    BASIC_SERVER_SUPPORTED_COMMANDS_RECEIVE
};
#endif

#ifdef BASIC_SERVER_SUPPORTED_COMMANDS_SEND
static zb_uint8_t gs_basic_server_generated_commands[] =
{
    BASIC_SERVER_SUPPORTED_COMMANDS_SEND
};
#endif

static zb_discover_cmd_list_t gs_basic_server_cmd_list =
{
#ifdef BASIC_SERVER_SUPPORTED_COMMANDS_RECEIVE
    sizeof(gs_basic_server_received_commands),  gs_basic_server_received_commands,
#else
    0,                                          NULL,
#endif
#ifdef BASIC_SERVER_SUPPORTED_COMMANDS_SEND
    sizeof(gs_basic_server_generated_commands), gs_basic_server_generated_commands
#else
    0,                                          NULL
#endif
};

static void zcl_reset_to_factory_defaults(void)
{
    zb_af_endpoint_desc_t **ep_desc_list;
    tr_basic_server_printf("resetting to factory defaults\n");

    /* ZCL9, 3.2.2.3.1 Reset to Factory Defaults Command:
     * On receipt of this command, the device resets all the attributes of all its clusters to
     * their factory defaults. Note that networking functionality, bindings, groups, scenes, or
     * other persistent data are not affected by this command.
     */
    ep_desc_list = ZCL_CTX().device_ctx->ep_desc_list;

    // walk all clusters and attributes, setting them to their default values
    for (zb_uint8_t ep_index = 0 ; ep_index < ZCL_CTX().device_ctx->ep_count ; ep_index++)
    {
        tr_zcl_endpoint_config_attr_init(ep_desc_list[ep_index]->ep_id);
    }

    ZB_BZERO(&diagnostics_ctx_zcl.mac_data, sizeof(zb_mac_diagnostic_info_t));
    ZB_BZERO(&diagnostics_ctx_zcl.zdo_data, sizeof(zdo_diagnostics_info_t));

    // NOTE: should not reset reporting configurations here
    // zb_zcl_reset_reporting_ctx();
    // tr_zcl_endpoint_config_reporting_init();
}

// Check the value of an attribute
static zb_ret_t basic_server_check_value(zb_uint16_t attr_id,
                                         zb_uint8_t  endpoint,
                                         zb_uint8_t  *value)
{
    ZVUNUSED(attr_id);
    ZVUNUSED(value);
    ZVUNUSED(endpoint);

    /* All values for mandatory attributes are allowed, extra check for
     * optional attributes is needed */

    return RET_OK;
}

// process basic server attribute write commands
static void basic_server_write_attr_hook(zb_uint8_t  endpoint,
                                         zb_uint16_t attr_id,
                                         zb_uint8_t  *new_value,
                                         zb_uint16_t manuf_code)
{
    tr_basic_server_printf("RX:(%s) Write Attribute Cmd, ATTR: %04X, EP: %02X\n",
                           PLUGIN_NAME,
                           attr_id,
                           endpoint);
    tr_basic_server_write_attr_cb(endpoint, attr_id, new_value, manuf_code);
}

static void basic_server_reset_invoke_user_app(zb_uint8_t param)
{
    zb_zcl_parsed_hdr_t cmd_info;
    zb_ret_t            result = RET_OK;

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);

    // invoke application callback
    if (tr_basic_server_reset_to_factory_defaults_cb(&cmd_info) == ZB_FALSE)
    {
        // if there is a callback setup to reset all attributes to the default settings, invoke it
        if (ZCL_CTX().set_default_attr_cb != NULL)
        {
            (ZCL_CTX().set_default_attr_cb)(ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
        }
        else
        {
            zcl_reset_to_factory_defaults();
        }
    }

    ZB_ZCL_PROCESS_COMMAND_FINISH(param,
                                  &cmd_info,
                                  result ==
                                  RET_OK ? TR_ZCL_STATUS_SUCCESS : (zb_zcl_get_backward_compatible_statuses_mode() ==
                                                                    ZB_ZCL_STATUSES_ZCL8_MODE) ? TR_ZCL_STATUS_FAILURE : TR_ZCL_STATUS_HARDWARE_FAILURE);
}

static zb_bool_t basic_server_cluster_handler(zb_uint8_t param)
{
    zb_bool_t           processed = ZB_TRUE;
    zb_zcl_parsed_hdr_t cmd_info;
    zb_ret_t            status = RET_OK;

    if (ZB_ZCL_GENERAL_GET_CMD_LISTS_PARAM == param)
    {
        ZCL_CTX().zb_zcl_cluster_cmd_list = &gs_basic_server_cmd_list;
        return ZB_TRUE;
    }

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);

    switch (cmd_info.cmd_id)
    {
        case TR_ZCL_CMD_RESET_TO_FACTORY_DEFAULTS_ID:
            tr_basic_server_printf("RX:(%s) Reset to Defaults Cmd, EP: %02X\n",
                                   PLUGIN_NAME,
                                   ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
            ZB_SCHEDULE_CALLBACK(basic_server_reset_invoke_user_app, param);
            status = RET_BUSY;
            break;

        default:
            tr_basic_server_printf("RX:(%s) Unknown Cmd ID: %02X, EP: %02X\n",
                                   PLUGIN_NAME,
                                   cmd_info.cmd_id,
                                   ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
            processed = ZB_FALSE;
            break;
    }

    if (processed)
    {
        if (cmd_info.disable_default_response && status == RET_OK)
        {
            zb_buf_free(param);
        }
        else if (status != RET_BUSY)
        {
            ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);
            ZB_ZCL_SEND_DEFAULT_RESP(param,
                                     ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).source.u.short_addr,
                                     ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                     ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).src_endpoint,
                                     ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
                                     cmd_info.profile_id,
                                     TR_ZCL_CLUSTER_BASIC_ID,
                                     cmd_info.seq_number,
                                     cmd_info.cmd_id,
                                     status == RET_OK ? TR_ZCL_STATUS_SUCCESS : TR_ZCL_STATUS_INVALID_FIELD);
        }
    }
    return processed;
}

static zb_bool_t basic_server_check_is_device_enabled(zb_uint8_t  ep_id,
                                                      zb_uint8_t  cmd_id,
                                                      zb_uint16_t cluster_id,
                                                      zb_bool_t   is_common_command)
{
    zb_ret_t              ret           = ZB_TRUE;
    zb_zcl_cluster_desc_t *cluster_desc = get_cluster_desc(zb_af_get_endpoint_desc(ep_id),
                                                           TR_ZCL_CLUSTER_BASIC_ID,
                                                           TR_ZCL_CLUSTER_SERVER_ROLE);

    if (cluster_desc)
    {
        zb_zcl_attr_t *attr_desc = zb_zcl_get_attr_desc_a(ep_id,
                                                          TR_ZCL_CLUSTER_BASIC_ID,
                                                          TR_ZCL_CLUSTER_SERVER_ROLE,
                                                          TR_ZCL_ATTR_BASIC_DEVICE_ENABLED_ID);

        if (attr_desc)
        {
            /* See ZCL spec 3.2.2.2.18 DeviceEnabled Attribute */
            if (!ZB_U2B(ZB_ZCL_GET_ATTRIBUTE_VAL_8(attr_desc)))
            {
                ret = ZB_FALSE;

                if ((is_common_command && (cmd_id == TR_ZCL_CMD_READ_ATTRIBUTES_ID ||
                                           cmd_id == TR_ZCL_CMD_READ_ATTRIBUTES_RESPONSE_ID ||
                                           cmd_id == TR_ZCL_CMD_WRITE_ATTRIBUTES_ID ||
                                           cmd_id == TR_ZCL_CMD_WRITE_ATTRIBUTES_UNDIVIDED_ID ||
                                           cmd_id == TR_ZCL_CMD_WRITE_ATTRIBUTES_RESPONSE_ID ||
                                           cmd_id == TR_ZCL_CMD_WRITE_ATTRIBUTES_NO_RESPONSE_ID)) ||
                    ((cmd_id == TR_ZCL_CMD_DEFAULT_RESPONSE_ID || !is_common_command) && cluster_id == TR_ZCL_CLUSTER_IDENTIFY_ID))
                {
                    ret = ZB_TRUE;
                }
            }
        }

        if (!ret)
        {
            tr_basic_server_printf("Device is disabled. The command should be dropped\n");
        }
    }
    return ret;
}

zb_bool_t zb_zcl_check_is_device_enabled(zb_uint8_t  ep_id,
                                         zb_uint8_t  cmd_id,
                                         zb_uint16_t cluster_id,
                                         zb_bool_t   is_common_command)
{
    return basic_server_check_is_device_enabled(ep_id, cmd_id, cluster_id, is_common_command);
}

// Basic server cluster plugin init
void tr_basic_server_init(void)
{
    zb_zcl_add_cluster_handlers(TR_ZCL_CLUSTER_BASIC_ID,
                                TR_ZCL_CLUSTER_SERVER_ROLE,
                                basic_server_check_value,
                                basic_server_write_attr_hook,
                                basic_server_cluster_handler);

    tr_basic_server_init_cb();
}
