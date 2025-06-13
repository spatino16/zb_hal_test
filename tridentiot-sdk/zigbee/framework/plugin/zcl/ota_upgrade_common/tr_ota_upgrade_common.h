/// ****************************************************************************
/// @file tr_ota_upgrade_common.h
///
/// @brief TODO - documentation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************

zb_ret_t tr_zcl_check_value_ota_upgrade(zb_uint16_t attr_id,
                                        zb_uint8_t  endpoint,
                                        zb_uint8_t  *value);

zb_uint8_t tr_zcl_ota_upgrade_get8(zb_uint8_t  endpoint,
                                   zb_uint16_t attr_id);

zb_uint16_t tr_zcl_ota_upgrade_get16(zb_uint8_t  endpoint,
                                     zb_uint16_t attr_id);

zb_uint32_t tr_zcl_ota_upgrade_get32(zb_uint8_t  endpoint,
                                     zb_uint16_t attr_id);

void tr_zcl_ota_upgrade_set_ota_status(zb_uint8_t endpoint,
                                       zb_uint8_t status);
