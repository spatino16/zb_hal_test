/// ****************************************************************************
/// @file tr_over_the_air_bootloading_client_cli.c
///
/// @brief Contains CLI commands specific to the OTA Bootloading client cluster
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "zboss_api.h"
#include "tr_cli_argument_parser.h"
#include "tr_cli_zcl_cmds.h"
#include "tr_debug_print.h"
#include "tr_over_the_air_bootloading_client.h"
#include "tr_osif_ota.h"
#include "tr_ota_upgrade_common.h"

zb_int_t cli_ota_upgrade_status(zb_int_t  argc,
                                zb_char_t *argv[])
{
    zb_uint32_t          current_offset;
    tr_ota_client_info_t *client_info = tr_ota_upgrade_client_get_client_info();

    // print state, t/f wating for response (next timer if so), offset in bytes and %
    current_offset =  tr_zcl_ota_upgrade_get32(1, TR_ZCL_ATTR_OVER_THE_AIR_BOOTLOADING_FILE_OFFSET_ID);

    tr_core_printf("State:                %d\n", zb_zcl_ota_upgrade_get_ota_status(1));
    tr_core_printf("Waiting for response: %d\n", client_info->pending_img_block_resp);
    tr_core_printf("Current file offset:  %d %3d%%\n", current_offset, (current_offset * 100) / client_info->download_file_size);

    return 0;
}

zb_int_t cli_ota_upgrade_start(zb_int_t  argc,
                               zb_char_t *argv[])
{
    zb_buf_get_in_delayed(tr_ota_init);

    return 0;
}

zb_int_t cli_ota_upgrade_stop(zb_int_t  argc,
                              zb_char_t *argv[])
{
    zb_zcl_ota_upgrade_stop_client();

    return 0;
}

zb_int_t cli_ota_upgrade_pause(zb_int_t  argc,
                               zb_char_t *argv[])
{
    zb_zcl_ota_upgrade_pause_client();

    return 0;
}

zb_int_t cli_ota_upgrade_info(zb_int_t  argc,
                              zb_char_t *argv[])
{
    tr_ota_server_info_t *server_info = tr_ota_upgrade_client_get_server_info();
    tr_ota_client_info_t *client_info = tr_ota_upgrade_client_get_client_info();

    // print client mfg id, image type, current fw version, hw version, query delay, server short, download error thresh
    tr_core_printf("Manuf ID:         0x%4.4x\n", client_info->mfg_id);
    tr_core_printf("Image Type ID     0x%4.4x\n", client_info->image_type);
    tr_core_printf("Current Version:  0x%8.8x\n", client_info->fw_version);
    tr_core_printf("Hardware Version: 0x%4.4x\n", client_info->hw_version);
    tr_core_printf("Query Delay:      %d\n", client_info->query_delay);
    tr_core_printf("Server:           0x%4.4x\n", server_info->short_addr);
    tr_core_printf("Error Threshold:  %d\n", 5);
    return 0;
}

zb_int_t cli_ota_upgrade_verify(zb_int_t  argc,
                                zb_char_t *argv[])
{
    zb_zcl_ota_upgrade_file_header_t *ota_header;
    ota_header = tr_ota_upgrade_client_get_ota_header();

    // is there a valid ota header?
    if (ota_header->file_id == ZB_ZCL_OTA_UPGRADE_FILE_HEADER_FILE_ID)
    {
        zb_osif_ota_verify_integrity(0, ota_header->total_image_size);
    }
    else
    {
        tr_core_printf("No ota upgrade header found\n");
    }
    return 0;
}

zb_int_t cli_ota_upgrade_bootload(zb_int_t  argc,
                                  zb_char_t *argv[])
{
    tr_ota_server_info_t *server_info = tr_ota_upgrade_client_get_server_info();

    tr_osif_ota_mark_fw_ready(0, server_info->image_size, server_info->fw_version);

    return 0;
}

zb_int_t cli_ota_upgrade_print(zb_int_t  argc,
                               zb_char_t *argv[])
{
    zb_uint8_t                       i;
    zb_zcl_ota_upgrade_file_header_t *ota_header;
    zb_zcl_ota_upgrade_sub_element_t *tag_header;

    ota_header = tr_ota_upgrade_client_get_ota_header();

    // is there a valid ota header?
    if (ota_header->file_id == ZB_ZCL_OTA_UPGRADE_FILE_HEADER_FILE_ID)
    {
        // print header info from downloaded image if any
        tr_core_printf("Header Magic:         0x%8.8x\n", ota_header->file_id);
        tr_core_printf("Header Version:       0x%4.4x\n", ota_header->header_version);
        tr_core_printf("Header Length:        %d 0x%4.4x\n", ota_header->header_length, ota_header->header_length);
        tr_core_printf("Header Field Control: 0x%4.4x\n", ota_header->fc);
        tr_core_printf("Manufacturer ID:      0x%4.4x\n", ota_header->manufacturer_code);
        tr_core_printf("Image Type:           0x%4.4x\n", ota_header->image_type);
        tr_core_printf("Firmware Version:     0x%8.8x\n", ota_header->file_version);
        tr_core_printf("Stack Version:        0x%4.4x\n", ota_header->stack_version);
        tr_core_printf("Header String:        ");

        for (i = 0 ; i < 32 ; i++)
        {
            tr_core_printf("%c", ota_header->header_string[i]);
        }
        tr_core_printf("\n");
        tr_core_printf("Image Size:           %d 0x%8.8x\n", ota_header->total_image_size, ota_header->total_image_size);

        // find tag headers
        tag_header = (zb_zcl_ota_upgrade_sub_element_t*)((zb_uint32_t)ota_header + ota_header->header_length);

        do
        {
            tr_core_printf("\n   Tag ID:            0x%4.4x\n", tag_header->tag_id);
            tr_core_printf("   Tag Length:        %d 0x%8.8x\n", tag_header->length, tag_header->length);
            tag_header = (zb_zcl_ota_upgrade_sub_element_t*)((zb_uint32_t)tag_header + tag_header->length + 6);
        }
        while ((zb_uint32_t)tag_header < ota_header->total_image_size);
    }
    else
    {
        tr_core_printf("No valid upgrade image in flash\n");
    }

    return 0;
}

TR_CLI_COMMAND_TABLE(zcl_ota_upgrade_c_cluster_commands) =
{
    { "status",   cli_ota_upgrade_status,   "display ota upgrade status"             },
    { "stop",     cli_ota_upgrade_stop,     "stop the ota upgrade client"            },
    { "start",    cli_ota_upgrade_start,    "start the ota upgrade client"           },
    { "pause",    cli_ota_upgrade_pause,    "pause the ota upgrade client"           },
    { "info",     cli_ota_upgrade_info,     "display the ota upgrade client info"    },
    { "verify",   cli_ota_upgrade_verify,   "verify the downloaded image signature"  },
    { "bootload", cli_ota_upgrade_bootload, "bootload the downloaded image"          },
    { "print",    cli_ota_upgrade_print,    "print the downloaded image header info" },
    TR_CLI_COMMAND_TABLE_END
};
