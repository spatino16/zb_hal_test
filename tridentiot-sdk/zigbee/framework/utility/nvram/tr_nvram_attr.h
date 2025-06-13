/// ****************************************************************************
/// @file tr_nvram_attr.h
///
/// @brief TODO - documentation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_NVRAM_ATTR_H
#define TR_NVRAM_ATTR_H

#include "tr_af.h"

#define TR_NVRAM_WRITE_DELAY         ZB_MILLISECONDS_TO_BEACON_INTERVAL(2000)

#define  TR_NVRAM_ATTR_STORAGE_VER_1 (1)
#define TR_NVRAM_STORAGE_CUR_VER     (TR_NVRAM_ATTR_STORAGE_VER_1)

// version 1 strcuture
typedef ZB_PACKED_PRE struct
{
    zb_uint8_t  endpoint;
    zb_uint16_t cluster_id;
    zb_uint16_t attr_id;
    zb_uint8_t  cluster_role;
    zb_uint16_t manuf_code;
    zb_uint8_t  data_type;
}ZB_PACKED_STRUCT tr_nvram_attr_storage_v1_t;

/// ****************************************************************************
///                             function prototypes
/// ****************************************************************************
zb_uint16_t tr_get_nvram_data_size(void);

zb_ret_t tr_nvram_write_app_data_cb(zb_uint8_t  page,
                                    zb_uint32_t pos);
void tr_nvram_read_app_data_cb(zb_uint8_t  page,
                               zb_uint32_t pos,
                               zb_uint16_t payload_length);
void tr_check_for_attr_nvram_update(zb_uint8_t  ep,
                                    zb_uint16_t cluster_id,
                                    zb_uint8_t  cluster_role,
                                    zb_uint16_t attr_id,
                                    zb_uint16_t manuf_code);
void tr_check_for_attr_nvram_update_and_force_save(zb_uint8_t  ep,
                                                   zb_uint16_t cluster_id,
                                                   zb_uint8_t  cluster_role,
                                                   zb_uint16_t attr_id,
                                                   zb_uint16_t manuf_code);

#endif // TR_NVRAM_ATTR_H
