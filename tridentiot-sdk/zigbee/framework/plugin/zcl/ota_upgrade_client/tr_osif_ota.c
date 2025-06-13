/// ****************************************************************************
/// @file tr_osif_ota.c
///
/// @brief Hardware interface functions for the OTA bootloading client plugin.
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_af.h"
#include "tr_over_the_air_bootloading_client.h"
#include "tr_osif_ota.h"
#include "flashctl.h"
#include "fota_define.h"

// comment this out to actually bootload after upgrade image is received
// #define TR_OTA_SKIP_UPGRADE

static zb_uint8_t  g_ota_flash_write_buf[OTA_FLASH_WRITE_BUF_SIZE];
static zb_uint16_t g_ota_flash_buf_in_ptr  = 0;
static zb_uint16_t g_ota_flash_buf_out_ptr = 0;
static zb_uint32_t g_flash_addr            = FOTA_UPDATE_BUFFER_FW_ADDRESS_2MB;

zb_zcl_ota_upgrade_file_header_t *tr_ota_upgrade_client_get_ota_header(void)
{
    return (zb_zcl_ota_upgrade_file_header_t*)FOTA_UPDATE_BUFFER_FW_ADDRESS_2MB;
}

zb_zcl_ota_upgrade_sub_element_t *tr_find_tag_header(zb_uint16_t tag_id,
                                                     zb_uint32_t raw_len)
{
    zb_zcl_ota_upgrade_file_header_t *ota_header;
    zb_zcl_ota_upgrade_sub_element_t *tag_header;

    // setup the ota header so we can find the first tag
    ota_header = (zb_zcl_ota_upgrade_file_header_t*)FOTA_UPDATE_BUFFER_FW_ADDRESS_2MB;

    // point to the first tag header in the ota file
    tag_header = (zb_zcl_ota_upgrade_sub_element_t*)(FOTA_UPDATE_BUFFER_FW_ADDRESS_2MB + ota_header->header_length);

    // find the image integrity code tag
    while ((tag_header->tag_id != tag_id) &&
           ((zb_uint32_t)tag_header < (FOTA_UPDATE_BUFFER_FW_ADDRESS_2MB + raw_len)))
    {
        // move to the next tag header, if there is one
        tag_header = (zb_zcl_ota_upgrade_sub_element_t*)((zb_uint32_t)tag_header + tag_header->length + 6);
    }

    if ((zb_uint32_t)tag_header >= (FOTA_UPDATE_BUFFER_FW_ADDRESS_2MB + raw_len))
    {
        // we did not find it
        tag_header = NULL;
    }
    return tag_header;
}

void tr_osif_ota_flash_set_write_addr(zb_uint32_t write_offset)
{
    g_flash_addr = write_offset + FOTA_UPDATE_BUFFER_FW_ADDRESS_2MB;
}

void tr_osif_ota_flash_erase(zb_uint32_t flash_addr,
                             zb_uint32_t image_size)
{
    zb_uint32_t ErasedSize = 0;

    while (image_size > ErasedSize)
    {
        if (((image_size - ErasedSize) > 0x10000) &&
            (MULTIPLE_OF_64K(flash_addr + ErasedSize)))
        {
            flash_erase(FLASH_ERASE_64K, flash_addr + ErasedSize);
            ErasedSize += 0x10000;
        }
        else if (((image_size - ErasedSize) > 0x8000) &&
                 (MULTIPLE_OF_32K(flash_addr + ErasedSize)))
        {
            flash_erase(FLASH_ERASE_32K, flash_addr + ErasedSize);
            ErasedSize += 0x8000;
        }
        else
        {
            flash_erase(FLASH_ERASE_SECTOR, flash_addr + ErasedSize);
            ErasedSize += SIZE_OF_FLASH_SECTOR_ERASE;
        }

        while (flash_check_busy());
    }
}

void *zb_osif_ota_open_storage(void)
{
    void *dev = NULL;

    return dev;
}

zb_bool_t zb_osif_ota_fw_size_ok(zb_uint32_t image_size)
{
    zb_bool_t ret = ZB_TRUE;

    // check the image size against the upgrade flash partition size
    if (image_size > SIZE_OF_FOTA_BANK_2MB)
    {
        ret = ZB_FALSE;
    }

    return ret;
}

zb_uint32_t zb_osif_ota_get_erase_portion(void)
{
    zb_uint32_t erase_portion = 0;

    return erase_portion;
}

void zb_osif_ota_erase_fw(void        *dev,
                          zb_uint_t   offset,
                          zb_uint32_t size)
{
    tr_ota_upgrade_client_printf("OTA upgrade: erase upgrade partition - ");
    g_flash_addr            = FOTA_UPDATE_BUFFER_FW_ADDRESS_2MB;
    g_ota_flash_buf_in_ptr  = 0;
    g_ota_flash_buf_out_ptr = 0;

    tr_osif_ota_flash_erase(FOTA_UPDATE_BUFFER_FW_ADDRESS_2MB, SIZE_OF_FOTA_BANK_2MB);
    tr_ota_upgrade_client_printf("done\n");
}

void zb_osif_ota_write(void        *dev,
                       zb_uint8_t  *data,
                       zb_uint_t   off,
                       zb_uint_t   size,
                       zb_uint32_t image_size)
{
    zb_uint16_t buf_len;

    // put new data into ota flash write buffer
    if ((g_ota_flash_buf_in_ptr + size) <= sizeof(g_ota_flash_write_buf))
    {
        // all the data fits at the pointer
        memcpy(&g_ota_flash_write_buf[g_ota_flash_buf_in_ptr], data, size);
        g_ota_flash_buf_in_ptr += size;
        g_ota_flash_buf_in_ptr &= (sizeof(g_ota_flash_write_buf) - 1);
    }
    else
    {
        memcpy(&g_ota_flash_write_buf[g_ota_flash_buf_in_ptr], data, sizeof(g_ota_flash_write_buf) - g_ota_flash_buf_in_ptr);
        memcpy(&g_ota_flash_write_buf[0],
               &data[sizeof(g_ota_flash_write_buf) - g_ota_flash_buf_in_ptr],
               size - (sizeof(g_ota_flash_write_buf) - g_ota_flash_buf_in_ptr));
        g_ota_flash_buf_in_ptr = size - (sizeof(g_ota_flash_write_buf) - g_ota_flash_buf_in_ptr);
    }

    // how much writable (contiguous) data is in the buffer?
    if (g_ota_flash_buf_out_ptr <= g_ota_flash_buf_in_ptr)
    {
        buf_len = g_ota_flash_buf_in_ptr - g_ota_flash_buf_out_ptr;
    }
    else
    {
        buf_len = sizeof(g_ota_flash_write_buf) - g_ota_flash_buf_out_ptr;
    }

    if (buf_len >= OTA_FLASH_WRITE_SIZE)
    {
        // write the flash
        tr_ota_upgrade_client_printf("OTA upgrade: flash write file offset %d progress %d%% flash addr 0x%x\n",
                                     off,
                                     (off * 100) / image_size,
                                     g_flash_addr);
        zb_uint16_t i;

        for (i = 0 ; i < OTA_FLASH_WRITE_SIZE ; i += FLASH_PROGRAM_SIZE_PAGE)
        {
            flash_write_page((zb_uint32_t)&g_ota_flash_write_buf[g_ota_flash_buf_out_ptr + i], g_flash_addr);
            g_flash_addr += FLASH_PROGRAM_SIZE_PAGE;
        }

        // move the out pointer
        g_ota_flash_buf_out_ptr += OTA_FLASH_WRITE_SIZE;

        if (g_ota_flash_buf_out_ptr >= sizeof(g_ota_flash_write_buf))
        {
            g_ota_flash_buf_out_ptr = 0;
        }
    }
}

void zb_osif_ota_write_last_data(void)
{
    zb_uint16_t i;
    zb_uint16_t buf_len;

    // how much writable (contiguous) data is in the buffer?
    if (g_ota_flash_buf_out_ptr <= g_ota_flash_buf_in_ptr)
    {
        buf_len = g_ota_flash_buf_in_ptr - g_ota_flash_buf_out_ptr;
    }
    else
    {
        buf_len = sizeof(g_ota_flash_write_buf) - g_ota_flash_buf_out_ptr;
    }

    tr_ota_upgrade_client_printf("OTA upgrade: flash write %d bytes\n", buf_len);

    for (i = 0 ; i < buf_len ; i += FLASH_PROGRAM_SIZE_PAGE)
    {
        flash_write_page((zb_uint32_t)&g_ota_flash_write_buf[g_ota_flash_buf_out_ptr + i], g_flash_addr);
        g_flash_addr += FLASH_PROGRAM_SIZE_PAGE;
    }
}

zb_bool_t tr_osif_ota_mark_fw_ready(void        *dev,
                                    zb_uint32_t size,
                                    zb_uint32_t revision)
{
    fota_information_t               fota_info;
    zb_zcl_ota_upgrade_sub_element_t *tag_header;
    zb_uint32_t                      flash_status;

    // find the upgrade image tag
    tag_header = tr_find_tag_header(ZB_ZCL_OTA_UPGRADE_FILE_TAG_UPGRADE_IMAGE, size);

    if (tag_header != NULL)
    {
        tr_ota_upgrade_client_printf("OTA upgrade: mark image revision 0x%x ready\n", revision);

        // create FOTA header and reboot
        memset(&fota_info, 0, sizeof(fota_info));                         // make sure all fields are erased
        fota_info.fotabank_startaddr = (zb_uint32_t)(&tag_header->value); // this is where the new image starts
        fota_info.fotabank_datalen   = (zb_uint32_t)(tag_header->length); // this is the size of the new image
        fota_info.target_startaddr   = APP_START_ADDRESS;                 // this is where to copy it
        fota_info.fotabank_ready     = FOTA_IMAGE_READY;                  // this is an indicator that the image is good
        fota_info.fotabank_crc       = zb_crc32((const zb_uint8_t*)fota_info.fotabank_startaddr, fota_info.fotabank_datalen);
#ifdef TR_OTA_SKIP_UPGRADE
        fota_info.fota_result = 0x00;                                     // this must be 0xFF in order for the bootloader to copy the image
        tr_ota_upgrade_client_printf("NOT MARKING UPGRADE FOR FLASHING!!!\n");
#else
        fota_info.fota_result = 0xFF;                                     // this must be 0xFF in order for the bootloader to copy the image
#endif

        // erase the fota space
        flash_status = flash_erase(FLASH_ERASE_SECTOR, FOTA_UPDATE_BANK_INFO_ADDRESS);

        if (flash_status != STATUS_SUCCESS)
        {
            tr_ota_upgrade_client_printf("Flash erase status 0x%x\n", flash_status);
        }

        // write the fota info to flash
        flash_status = flash_write_page((uint32_t)&fota_info, FOTA_UPDATE_BANK_INFO_ADDRESS);

        if (flash_status != STATUS_SUCCESS)
        {
            tr_ota_upgrade_client_printf("Flash write page status 0x%x\n", flash_status);
        }
    }
    else
    {
        tr_ota_upgrade_client_printf("OTA upgrade tag not found, abort\n");
        return ZB_FALSE;
    }
    return ZB_TRUE;
}

void zb_osif_upgrade_now(void)
{
    #ifndef TR_OTA_SKIP_UPGRADE
    // reboot to invoke the bootloader, it will transfer the image over the application
    tr_ota_upgrade_client_printf("OTA upgrade: reboot to complete upgrade\n");
    zb_reset(0);
#endif
}

void zb_osif_ota_mark_fw_absent(void)
{
    tr_ota_upgrade_client_printf("zb_osif_ota_mark_fw_absent\n");
}

void zb_osif_ota_mark_fw_updated(void)
{
    tr_ota_upgrade_client_printf("zb_osif_ota_mark_fw_updated\n");
}

void zb_osif_ota_close_storage(void *dev)
{
    tr_ota_upgrade_client_printf("zb_osif_ota_close_storage %d\n", dev);
}

zb_bool_t zb_osif_ota_verify_integrity(void        *dev,
                                       zb_uint32_t raw_len)
{
    zb_bool_t                        ret = ZB_TRUE;
    zb_uint8_t                       calc_hash[16];
    zb_uint8_t                       i;
    zb_uint8_t                       *image_hash;
    zb_zcl_ota_upgrade_sub_element_t *integrity_code_tag_header;

    tr_ota_upgrade_client_printf("OTA upgrade: image integrity check - ");

    // get the integrity tag if there is one
    integrity_code_tag_header = tr_find_tag_header(ZB_ZCL_OTA_UPGRADE_FILE_TAG_IMAGE_INTEGRITY_CODE, raw_len);

    if (integrity_code_tag_header != NULL)
    {
        // we found an integrity code tag, verify it
        zb_aes_mmo_128((const zb_uint8_t*)FOTA_UPDATE_BUFFER_FW_ADDRESS_2MB, raw_len - 22, calc_hash);

        if (memcmp(calc_hash, integrity_code_tag_header->value, sizeof(calc_hash)) != 0)
        {
            ret = ZB_FALSE;

            tr_ota_upgrade_client_printf("failed\n");
            tr_ota_upgrade_client_printf("   Calc  hash:");

            for (i = 0 ; i < 16 ; i++)
            {
                tr_ota_upgrade_client_printf(" %2.2x", calc_hash[i]);
            }
            tr_ota_upgrade_client_printf("\n");
            image_hash = (zb_uint8_t*)(FOTA_UPDATE_BUFFER_FW_ADDRESS_2MB + raw_len - 16);
            tr_ota_upgrade_client_printf("   Image hash:");

            for (i = 0 ; i < 16 ; i++)
            {
                tr_ota_upgrade_client_printf(" %2.2x", image_hash[i]);
            }
            tr_ota_upgrade_client_printf("\n");
        }
        else
        {
            tr_ota_upgrade_client_printf("passed\n");
        }
    }
    else
    {
        tr_ota_upgrade_client_printf("no tag found\n");
    }

    return ret;
}

/* WARNING: Works with absolute address! */
void zb_osif_ota_read(void        *dev,
                      zb_uint8_t  *data,
                      zb_uint32_t addr,
                      zb_uint32_t size)
{
    tr_ota_upgrade_client_printf("zb_osif_ota_read dev %d data %d addr %ld size %ld\n",
                                 dev,
                                 data,
                                 addr,
                                 size);
}

zb_bool_t zb_osif_ota_verify_integrity_async(void        *dev,
                                             zb_uint32_t raw_len)
{
    zb_bool_t                        ret = ZB_TRUE;
    zb_uint8_t                       calc_hash[16];
    zb_uint8_t                       i;
    zb_uint8_t                       *image_hash;
    zb_zcl_ota_upgrade_sub_element_t *integrity_code_tag_header;

    tr_ota_upgrade_client_printf("OTA upgrade: image integrity check - ");

    // get the integrity tag if there is one
    integrity_code_tag_header = tr_find_tag_header(ZB_ZCL_OTA_UPGRADE_FILE_TAG_IMAGE_INTEGRITY_CODE, raw_len);

    if (integrity_code_tag_header != NULL)
    {
        // we found an integrity code tag, verify it
        zb_aes_mmo_128((const zb_uint8_t*)FOTA_UPDATE_BUFFER_FW_ADDRESS_2MB, raw_len - 22, calc_hash);

        if (memcmp(calc_hash, integrity_code_tag_header->value, sizeof(calc_hash)) != 0)
        {
            tr_ota_upgrade_client_printf("failed\n");
            tr_ota_upgrade_client_printf("   Calc  hash:");

            for (i = 0 ; i < 16 ; i++)
            {
                tr_ota_upgrade_client_printf(" %2.2x", calc_hash[i]);
            }
            tr_ota_upgrade_client_printf("\n");
            image_hash = (zb_uint8_t*)(FOTA_UPDATE_BUFFER_FW_ADDRESS_2MB + raw_len - 16);
            tr_ota_upgrade_client_printf("   Image hash:");

            for (i = 0 ; i < 16 ; i++)
            {
                tr_ota_upgrade_client_printf(" %2.2x", image_hash[i]);
            }
            tr_ota_upgrade_client_printf("\n");
        }
        else
        {
            tr_ota_upgrade_client_printf("passed\n");
        }
    }
    else
    {
        tr_ota_upgrade_client_printf("no tag found\n");
    }

    return ret;
}

zb_uint8_t zb_erase_fw(zb_uint32_t address,
                       zb_uint32_t pages_count)
{
    ZVUNUSED(address);
    ZVUNUSED(pages_count);
    tr_ota_upgrade_client_printf("TODO: zb_erase_fw emulation\n");
    return 0;
}

zb_uint8_t zb_write_fw(zb_uint32_t address,
                       zb_uint8_t  *buf,
                       zb_uint16_t len)
{
    ZVUNUSED(address);
    ZVUNUSED(buf);
    ZVUNUSED(len);
    tr_ota_upgrade_client_printf("TODO: zb_write_fw emulation\n");
    return 0;
}

void Hash16_Calc(zb_uint32_t pBuffer,
                 zb_uint32_t BufferLength,
                 zb_uint8_t  *hash16)
{
    ZVUNUSED(pBuffer);
    ZVUNUSED(BufferLength);
    ZVUNUSED(hash16);
    tr_ota_upgrade_client_printf("TODO: Hash16_Calc emulation\n");
}
