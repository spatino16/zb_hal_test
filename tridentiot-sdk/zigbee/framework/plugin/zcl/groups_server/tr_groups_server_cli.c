/// ****************************************************************************
/// @file tr_groups_server_cli.c
///
/// @brief Contains CLI commands specific to the GROUPS server cluster
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_groups_server_cli.h"
#include "tr_cli_zcl_cmds.h"

zb_int_t cli_cmd_groups_server_print_table(zb_int_t  argc,
                                           zb_char_t *argv[])
{
    zb_uint16_t           groups[ZB_APS_GROUP_TABLE_SIZE] = { 0 };
    zb_ushort_t           group_count                     = 0;
    zb_uint8_t            total_group_table_index         = 0;
    zb_af_endpoint_desc_t **ep_desc_list;
    zb_char_t             group_name[17] = { 0 };

    ep_desc_list = ZCL_CTX().device_ctx->ep_desc_list;

    // disable application prints
    zb_bool_t app_prints_enabled = tr_check_print_group(TR_DEBUG_PRINT_APP);

    if (app_prints_enabled)
    {
        tr_disable_print_group(TR_DEBUG_PRINT_APP);
    }
    tr_core_printf("Group Table Size: %d\n", ZB_APS_GROUP_TABLE_SIZE);
    tr_core_printf(" idx  ep    id     name\n");

    for (zb_uint8_t ep_index = 0 ; ep_index < ZCL_CTX().device_ctx->ep_count ; ep_index++)
    {
        zb_apsme_get_groups_by_ep(ZB_APS_GROUP_TABLE_SIZE,
                                  groups,
                                  &group_count,
                                  ep_desc_list[ep_index]->ep_id);

        for (zb_uint8_t group_index = 0 ; group_index < group_count ; group_index++)
        {
            tr_core_printf("  %d   %d   0x%4.4x   ",
                           total_group_table_index,
                           ep_desc_list[ep_index]->ep_id,
                           groups[group_index]);

            tr_groups_server_view_group_cb(ep_desc_list[ep_index]->ep_id,
                                           groups[group_index],
                                           group_name);
            tr_core_printf("\"");

            for (zb_uint8_t i = 0 ; i < group_name[0] ; i++)
            {
                tr_core_printf("%c", group_name[i + 1]);
            }
            tr_core_printf("\"\n");

            total_group_table_index++;
        }
    }

    if (total_group_table_index == 0)
    {
        tr_core_printf("Group Table Empty!\n");
    }

    // re-enable application prints if they were enabled previously
    if (app_prints_enabled)
    {
        tr_enable_print_group(TR_DEBUG_PRINT_APP);
    }

    return ZB_TRUE;
}
