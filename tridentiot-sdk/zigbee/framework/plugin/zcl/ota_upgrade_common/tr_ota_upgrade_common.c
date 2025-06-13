/// ****************************************************************************
/// @file tr_ota_upgrade_common.c
///
/// @brief TODO - documentation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_af.h"
#include "zb_aps.h"
#include "zdo_wwah_stubs.h"

zb_ret_t tr_zcl_check_value_ota_upgrade(zb_uint16_t attr_id,
                                        zb_uint8_t  endpoint,
                                        zb_uint8_t  *value)
{
    zb_ret_t ret = ZB_TRUE;
    ZVUNUSED(endpoint);
    ZVUNUSED(value);

    switch (attr_id)
    {
    /* remove for NTS certification - test 9.5.9
     * TH sends wrong value - 6000 (e.q. 25 minutes) */
#ifdef NTS_HACK
        case TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_MINIMUM_BLOCK_REQUEST_PERIOD_ID:
            ret = 258 > *(zb_uint16_t*)value
            ? RET_OK : RET_ERROR;
            break;
#endif

        default:
            break;
    }
    return ret;
}

/*************************** Helper functions *************************/

zb_uint8_t tr_zcl_ota_upgrade_get8(zb_uint8_t  endpoint,
                                   zb_uint16_t attr_id)
{
    zb_zcl_attr_t *attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                                      TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                                                      TR_ZCL_CLUSTER_CLIENT_ROLE,
                                                      attr_id);
    ZB_ASSERT(attr_desc);
    return ZB_ZCL_GET_ATTRIBUTE_VAL_8(attr_desc);
}

zb_uint16_t tr_zcl_ota_upgrade_get16(zb_uint8_t  endpoint,
                                     zb_uint16_t attr_id)
{
    zb_zcl_attr_t *attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                                      TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                                                      TR_ZCL_CLUSTER_CLIENT_ROLE,
                                                      attr_id);
    ZB_ASSERT(attr_desc);
    return ZB_ZCL_GET_ATTRIBUTE_VAL_16(attr_desc);
}

zb_uint32_t tr_zcl_ota_upgrade_get32(zb_uint8_t  endpoint,
                                     zb_uint16_t attr_id)
{
    zb_zcl_attr_t *attr_desc = zb_zcl_get_attr_desc_a(endpoint,
                                                      TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                                                      TR_ZCL_CLUSTER_CLIENT_ROLE,
                                                      attr_id);
    ZB_ASSERT(attr_desc);
    return ZB_ZCL_GET_ATTRIBUTE_VAL_32(attr_desc);
}

void tr_zcl_ota_upgrade_set_ota_status(zb_uint8_t endpoint,
                                       zb_uint8_t status)
{
    zb_zcl_set_attr_val_manuf(endpoint,
                              TR_ZCL_CLUSTER_OVER_THE_AIR_BOOTLOADING_ID,
                              TR_ZCL_CLUSTER_CLIENT_ROLE,
                              TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_IMAGE_UPGRADE_STATUS_ID,
                              ZB_ZCL_NON_MANUFACTURER_SPECIFIC,
                              &status,
                              ZB_FALSE);
}
