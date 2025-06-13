/// ****************************************************************************
/// @file tr_osif_ota.h
///
/// @brief Declarations for hardware interface functions for the OTA
/// bootloading client plugin.
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_OSIF_OTA_H
#define TR_OSIF_OTA_H

#define MULTIPLE_OF_32K(Add) ((((Add) & (0x8000 - 1)) == 0) ? 1 : 0)
#define MULTIPLE_OF_64K(Add) ((((Add) & (0x10000 - 1)) == 0) ? 1 : 0)
#define SIZE_OF_FLASH_SECTOR_ERASE 4096

#define OTA_FLASH_WRITE_SIZE       2048                       // write to flash when we have received this many bytes
#define OTA_FLASH_WRITE_BUF_SIZE   (2 * OTA_FLASH_WRITE_SIZE) // size must be power of 2

zb_zcl_ota_upgrade_file_header_t *tr_ota_upgrade_client_get_ota_header(void);

void tr_osif_ota_flash_set_write_addr(zb_uint32_t write_offset);
void *zb_osif_ota_open_storage(void);
zb_bool_t zb_osif_ota_fw_size_ok(zb_uint32_t image_size);
zb_uint32_t zb_osif_ota_get_erase_portion(void);
void zb_osif_ota_erase_fw(void        *dev,
                          zb_uint_t   offset,
                          zb_uint32_t size);
void zb_osif_ota_write(void        *dev,
                       zb_uint8_t  *data,
                       zb_uint_t   off,
                       zb_uint_t   size,
                       zb_uint32_t image_size);
void zb_osif_ota_write_last_data(void);

zb_bool_t tr_osif_ota_mark_fw_ready(void        *dev,
                                    zb_uint32_t size,
                                    zb_uint32_t revision);
void zb_osif_upgrade_now();
void zb_osif_ota_mark_fw_absent(void);
void zb_osif_ota_mark_fw_updated(void);
void zb_osif_ota_close_storage(void *dev);
zb_bool_t zb_osif_ota_verify_integrity(void        *dev,
                                       zb_uint32_t raw_len);

/* WARNING: Works with absolute address! */
void zb_osif_ota_read(void        *dev,
                      zb_uint8_t  *data,
                      zb_uint32_t addr,
                      zb_uint32_t size);

zb_bool_t zb_osif_ota_verify_integrity_async(void        *dev,
                                             zb_uint32_t raw_len);
zb_uint8_t zb_erase_fw(zb_uint32_t address,
                       zb_uint32_t pages_count);
zb_uint8_t zb_write_fw(zb_uint32_t address,
                       zb_uint8_t  *buf,
                       zb_uint16_t len);
void Hash16_Calc(zb_uint32_t pBuffer,
                 zb_uint32_t BufferLength,
                 zb_uint8_t  *hash16);

#endif /* TR_OSIF_OTA_H */
