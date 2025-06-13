/// ****************************************************************************
/// @file tr_ota_upgrade_client_cb_stubs.c
///
/// @brief weakly defined application callback stubs
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_af.h"
#include "tr_ota_upgrade_common.h"
#include "zb_common.h"

ZB_WEAK void tr_over_the_air_bootloading_client_version_cb(zb_uint8_t  endpoint,
                                                           zb_uint32_t *fw_version,
                                                           zb_uint16_t *mfg_id,
                                                           zb_uint16_t *image_type,
                                                           zb_uint16_t *hw_version)
{
    // the stub will just return the attribute values
    *mfg_id                  = tr_zcl_ota_upgrade_get16(endpoint, TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_MANUFACTURER_ID_ID);
    *image_type              = tr_zcl_ota_upgrade_get16(endpoint, TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_IMAGE_TYPE_ID_ID);
    *fw_version              = tr_zcl_ota_upgrade_get32(endpoint, TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_CURRENT_FILE_VERSION_ID);
    zb_zcl_attr_t *attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                                      TR_ZCL_CLUSTER_BASIC_ID,
                                                      TR_ZCL_CLUSTER_SERVER_ROLE,
                                                      TR_ZCL_ATTR_BASIC_HW_VERSION_ID);
    ZB_ASSERT(attr_desc);
    *hw_version = ZB_ZCL_GET_ATTRIBUTE_VAL_8(attr_desc);
}

ZB_WEAK void tr_over_the_air_bootloading_client_upgrade_start_cb(zb_uint32_t image_version,
                                                                 zb_uint32_t image_size)
{
    ZVUNUSED(image_version);
    ZVUNUSED(image_size);
}

ZB_WEAK void tr_over_the_air_bootloading_client_server_not_found_cb(void)
{
}

ZB_WEAK void tr_over_the_air_bootloading_client_init_cb(void)
{
}

ZB_WEAK zb_bool_t tr_over_the_air_bootloading_client_image_notify_cb(void)

{
    return ZB_TRUE;
}

ZB_WEAK zb_bool_t tr_over_the_air_bootloading_client_query_next_image_resp_cb(zb_uint32_t fw_version,
                                                                              zb_uint16_t image_type,
                                                                              zb_uint16_t mfg_id)
{
    return ZB_TRUE;
}

ZB_WEAK zb_bool_t tr_over_the_air_bootloading_client_upgrade_end_resp_cb(void)
{
    return ZB_TRUE;
}
