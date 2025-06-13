/// ****************************************************************************
/// @file tr_ota_upgrade_client_minimal.c
///
/// @brief Support functions for the OTA bootload client plugin
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_af.h"
#include "zb_aps.h"
#include "zdo_wwah_stubs.h"

/* That code may be called from zcl_poll_control_commands.c, so linked always.*/
zb_uint8_t zb_zcl_ota_upgrade_get_ota_status(zb_uint8_t endpoint)
{
    zb_zcl_attr_t *attr_desc;
    zb_uint8_t    status;

    attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                       TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                                       TR_ZCL_CLUSTER_CLIENT_ROLE,
                                       TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_IMAGE_UPGRADE_STATUS_ID);
    ZB_ASSERT(attr_desc);

    status = ZB_ZCL_GET_ATTRIBUTE_VAL_8(attr_desc);

    return status;
}

void zcl_ota_abort_and_set_tc(zb_uint8_t param)
{
    zb_zcl_cluster_handler_t cluster_handler;

    cluster_handler = zb_zcl_get_cluster_handler(TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                                                 TR_ZCL_CLUSTER_CLIENT_ROLE);

    if (cluster_handler)
    {
        zb_zcl_parsed_hdr_t *cmd_info      = ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t);
        cmd_info->disable_default_response = tr_global_default_response_policy;
        cmd_info->cluster_id               = TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID;
        cmd_info->cmd_id                   = ZB_ZCL_CMD_OTA_UPGRADE_INTERNAL_ABORT_ID;
        cluster_handler(param);
    }
    else
    {
        zb_buf_free(param);
    }
}
