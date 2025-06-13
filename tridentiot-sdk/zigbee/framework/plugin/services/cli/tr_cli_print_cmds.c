/// ****************************************************************************
/// @file tr_cli_print_cmds.c
///
/// @brief this contains the implementation for CLI PRINT commands
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include <unistd.h>
#include "tr_af.h"
#include "tr_cli_argument_parser.h"
#include "tr_mfg_tokens.h"

#ifdef TR_GROUPS_SERVER_CLI_ENABLE
#include "tr_groups_server_cli.h"
#endif

// Print all attributes for all endpoints
zb_int_t cli_cmd_print_attr(zb_int_t  argc,
                            zb_char_t *argv[])
{
    zb_uint8_t            ep_index;
    zb_uint8_t            cluster_index;
    zb_uint8_t            attr_index;
    zb_af_endpoint_desc_t **ep_desc_list;
    zb_zcl_cluster_desc_t *cluster_desc;
    zb_zcl_attr_t         *attr_desc;
    zb_char_t             *option_argument;
    zb_uint8_t            *ext_attr_data_p = NULL;
    zb_uint16_t           cluster_id       = 0xFFFF;
    zb_uint8_t            ep               = 0xFF;

    if (tr_cli_get_option(argc, argv, "c:", &option_argument))
    {
        cluster_id = tr_dec_or_hex_string_to_int(option_argument);
    }

    if (tr_cli_get_option(argc, argv, "e:", &option_argument))
    {
        ep = tr_dec_or_hex_string_to_int(option_argument);
    }

    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        tr_core_printf("usage: print attr [-c cluster] [-e endpoint]\n");
        return 0;
    }

    zb_zcl_diagnostics_sync_counters(0, NULL);

    // walk the endpoints
    for (ep_index = 0 ; ep_index < ZCL_CTX().device_ctx->ep_count ; ep_index++)
    {
        ep_desc_list = ZCL_CTX().device_ctx->ep_desc_list;

        if ((ep == 0xFF) || (ep == ep_desc_list[ep_index]->ep_id))
        {
            tr_core_printf("Endpoint %d:\n", ep_desc_list[ep_index]->ep_id);
            tr_core_printf("   Clust  Role   Attr   Type Acc  Value\n");

            // loop over clusters
            for (cluster_index = 0 ; cluster_index < ep_desc_list[ep_index]->cluster_count ; cluster_index++)
            {
                cluster_desc = &ep_desc_list[ep_index]->cluster_desc_list[cluster_index];

                if ((cluster_id == 0xFFFF) || (cluster_id == cluster_desc->cluster_id))
                {
                    // loop over attributes
                    for (attr_index = 0 ; attr_index < cluster_desc->attr_count - 1 ; attr_index++)
                    {
                        attr_desc = &cluster_desc->attr_desc_list[attr_index];
                        tr_core_printf("   0x%4.4x ", cluster_desc->cluster_id);

                        if (cluster_desc->role_mask == TR_ZCL_CLUSTER_SERVER_ROLE)
                        {
                            tr_core_printf("server ");
                        }
                        else
                        {
                            tr_core_printf("client ");
                        }
                        tr_core_printf("0x%4.4x 0x%2.2x 0x%2.2x ", attr_desc->id, attr_desc->type, attr_desc->access);
#if 0 // TODO: Disable bug keep this around for now in case we want to change access printing

                        switch (attr_desc->access & 0x03)
                        {
                            case ZB_ZCL_ATTR_ACCESS_READ_ONLY:
                                tr_core_printf("RO  ");
                                break;

                            case ZB_ZCL_ATTR_ACCESS_WRITE_ONLY:
                                tr_core_printf("WO  ");
                                break;

                            case ZB_ZCL_ATTR_ACCESS_READ_WRITE:
                                tr_core_printf("RW  ");
                                break;
                        }
#endif /* if 0 */
                        ext_attr_data_p = tr_zcl_external_attribute_read_cb(ep_desc_list[ep_index]->ep_id,
                                                                            cluster_desc->cluster_id,
                                                                            cluster_desc->role_mask,
                                                                            attr_desc->id,
                                                                            attr_desc->manuf_code);
                        tr_print_attr_value(attr_desc, ext_attr_data_p);
                        tr_core_printf(" (%s)\n", tr_find_cluster_name(cluster_desc->cluster_id));
                    }
                }
            }
            tr_core_printf("\n");
        }
    }

    return 0;
}

// Print the binding table
zb_int_t cli_cmd_print_bind(zb_int_t  argc,
                            zb_char_t *argv[])
{
    zb_uint8_t     dst_index;
    zb_uint8_t     src_index;
    zb_uint8_t     i;
    zb_ieee_addr_t dst_eui64;

    tr_core_printf("dst#  type  src#  clus  src ep  dst(ep/eui or group)\n");

    for (dst_index = 0 ; dst_index < ZG->aps.binding.dst_n_elements ; dst_index++)
    {
        src_index = ZG->aps.binding.dst_table[dst_index].src_table_index;

        if (ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT == ZG->aps.binding.dst_table[dst_index].dst_addr_mode)
        {
            tr_core_printf(" %2d:   GRP   %2d   %4.4x     %d     %4.4x\n",
                           dst_index,
                           // bnd_type,
                           ZG->aps.binding.dst_table[dst_index].src_table_index,
                           ZG->aps.binding.src_table[src_index].cluster_id,
                           ZG->aps.binding.src_table[src_index].src_end,
                           ZG->aps.binding.dst_table[dst_index].u.group_addr);
        }
        else
        {
            tr_core_printf(" %2d:   L+E   %2d   %4.4x     %d     %d ",
                           dst_index,
                           // bnd_type,
                           ZG->aps.binding.dst_table[dst_index].src_table_index,
                           ZG->aps.binding.src_table[src_index].cluster_id,
                           ZG->aps.binding.src_table[src_index].src_end,
                           ZG->aps.binding.dst_table[dst_index].u.long_addr.dst_end);

            zb_address_ieee_by_ref(dst_eui64, ZG->aps.binding.dst_table[dst_index].u.long_addr.dst_addr);

            for (i = 0 ; i < 8 ; i++)
            {
                tr_core_printf("%2.2x", dst_eui64[7 - i]);
            }
            tr_core_printf("\n");
        }
    }

    ZVUNUSED(src_index);

    return 0;
}

#if defined ZB_COORDINATOR_ROLE || defined ZB_ROUTER_ROLE

zb_uint8_t tr_print_neighbor_zed_entries(void)
{
    zb_uint8_t            i;
    zb_uint8_t            used = 0;
    zb_neighbor_tbl_ent_t *entry;
    zb_ret_t              ret;
    zb_uint16_t           short_addr;
    zb_ieee_addr_t        ieee_address;

    tr_core_printf("   ZEDs and SZEDs:\n");
    tr_core_printf("       type short  lqa rssi timeout remaining   frame counter eui\n");

    for (i = 0 ; i < ZB_NEIGHBOR_TABLE_SIZE ; i++)
    {
        ret = zb_nwk_neighbor_get_by_idx(i, &entry);

        if ((ret == RET_OK) && (entry->device_type == ZB_NWK_DEVICE_TYPE_ED))
        {
            zb_address_ieee_by_ref(ieee_address, entry->addr_ref);
            zb_address_short_by_ref(&short_addr, entry->addr_ref);
            tr_core_printf("   %2d: ", i);

            if (entry->dev.ed.rx_on_when_idle)
            {
                tr_core_printf("ZED  ");
            }
            else
            {
                tr_core_printf("SZED ");
            }
            tr_core_printf("0x%4.4x %3d %4d   %2d    %2d      0x%8.8x    ",
                           short_addr,
                           entry->lqa,
                           entry->rssi,
                           entry->dev.ed.nwk_timeout,
                           entry->dev.ed.time_to_expire,
                           entry->incoming_frame_counter);

            tr_print_eui64(ieee_address);
            tr_core_printf("\n");
            used++;
        }
    }
    tr_core_printf("\n");
    return used;
}

uint8_t tr_print_neighbor_zc_zr_entries(void)
{
    zb_uint8_t            i;
    zb_uint8_t            used = 0;
    zb_neighbor_tbl_ent_t *entry;
    zb_ret_t              ret;
    zb_uint16_t           short_addr;
    zb_ieee_addr_t        ieee_address;

    tr_core_printf("   ZCs and ZRs:\n");
    tr_core_printf("       type short  lqa rssi in out age frame counter eui\n");

    for (i = 0 ; i < ZB_NEIGHBOR_TABLE_SIZE ; i++)
    {
        ret = zb_nwk_neighbor_get_by_idx(i, &entry);

        if ((ret == RET_OK) &&
            ((entry->device_type == ZB_NWK_DEVICE_TYPE_COORDINATOR) || (entry->device_type == ZB_NWK_DEVICE_TYPE_ROUTER)))
        {
            zb_address_ieee_by_ref(ieee_address, entry->addr_ref);
            zb_address_short_by_ref(&short_addr, entry->addr_ref);

            if (entry->device_type == ZB_NWK_DEVICE_TYPE_COORDINATOR)
            {
                tr_core_printf("   %2d: ZC   ", i);
            }
            else
            {
                tr_core_printf("   %2d: ZR   ", i);
            }
            tr_core_printf("0x%4.4x %3d %4d %2d %2d  %2d  0x%8.8x    ",
                           short_addr,
                           entry->lqa,
                           entry->rssi,
                           ZB_NWK_NEIGHBOUR_GET_PATH_COST(entry),
                           entry->dev.r.outgoing_cost,
                           entry->dev.r.age,
                           entry->incoming_frame_counter);

            tr_print_eui64(ieee_address);
            tr_core_printf("\n");
            used++;
        }
    }
    tr_core_printf("\n");
    return used;
}

zb_int_t cli_cmd_print_neigh_table(zb_int_t  argc,
                                   zb_char_t *argv[])
{
    zb_uint8_t used       = 0;
    zb_uint8_t print_mask = 0;
    zb_char_t  *option_argument;

    if (tr_cli_get_option(argc, argv, "n", &option_argument))
    {
        print_mask |= 0x01;
    }

    if (tr_cli_get_option(argc, argv, "c", &option_argument))
    {
        print_mask |= 0x02;
    }

    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        tr_core_printf("usage: print neighbor [-n only print neighbors] [-c only print children]\n");
        return 0;
    }

    if (print_mask == 0)
    {
        print_mask = 0x03;
    }

    if (print_mask & 0x01)
    {
        used += tr_print_neighbor_zc_zr_entries();
    }

    if (print_mask & 0x02)
    {
        used += tr_print_neighbor_zed_entries();
    }
    tr_core_printf("   %d of %d entries used\n", used, ZB_NEIGHBOR_TABLE_SIZE);
    return 0;
}

#endif /* if defined ZB_COORDINATOR_ROLE || defined ZB_ROUTER_ROLE */

// Print the network and link keys
zb_int_t cli_cmd_print_keys(zb_int_t  argc,
                            zb_char_t *argv[])
{
    zb_uint8_t i;

    // network key
    tr_core_printf("Network Key:\n");
    tr_core_printf("  Seq Num: 0x%2.2x\n", ZG->nwk.nib.secur_material_set->key_seq_number);
    tr_core_printf("  FC: 0x%8.8x\n", ZG->nwk.nib.outgoing_frame_counter);
    tr_core_printf("  Key:");

    for (i = 0 ; i < ZB_CCM_KEY_SIZE ; i++)
    {
        tr_core_printf(" %2.2x", ZG->nwk.nib.secur_material_set->key[i]);
    }
    tr_core_printf("\n\n");

    // link key
    // tr_core_printf("LNK Key Incoming FC: 0x%8.8x\n", ZG->aps.aib.aps_device_key_pair_storage.cached.device_address);
    tr_core_printf("TC Link Key:\n");
    // tr_core_printf("  Out FC: 0x%8.8x\n", ZG->aps.aib.outgoing_frame_counter); // TODO: r23 migration issue - Arch 07/10/2024
    tr_core_printf("  EUI64: ");

    for (i = 0 ; i < 8 ; i++)
    {
        tr_core_printf("%2.2x", ZG->aps.aib.aps_device_key_pair_storage.cached.device_address[7 - i]);
    }
    tr_core_printf("\n  Key:");

    for (i = 0 ; i < ZB_CCM_KEY_SIZE ; i++)
    {
        tr_core_printf(" %2.2x", ZG->aps.aib.aps_device_key_pair_storage.cached.link_key[i]);
    }
    tr_core_printf("\n");
    return 0;
}

zb_int_t cli_cmd_print_reporting(zb_int_t  argc,
                                 zb_char_t *argv[])
{
    zb_uint8_t              ep;
    zb_uint8_t              i;
    zb_uint8_t              num_used = 0;
    zb_zcl_reporting_info_t *rep_info;

    // walk the endpoints
    for (ep = 0 ; ep < ZCL_CTX().device_ctx->ep_count ; ep++)
    {
        for (i = 0 ; i < TR_TOTAL_ATTR_REPORT_TABLE_SIZE ; i++)
        {
            rep_info = &ZCL_CTX().device_ctx->ep_desc_list[ep]->reporting_info[i];

            if (ZB_ZCL_GET_REPORTING_FLAG(rep_info, ZB_ZCL_REPORTING_SLOT_BUSY))
            {
                if (rep_info->ep == ZCL_CTX().device_ctx->ep_desc_list[ep]->ep_id)
                {
                    tr_core_printf("  %d: ep %d clus 0x%4.4x attr 0x%4.4x role %d mfg 0x%4.4x ",
                                   i,
                                   rep_info->ep,
                                   rep_info->cluster_id,
                                   rep_info->attr_id,
                                   rep_info->cluster_role,
                                   rep_info->manuf_code);
                    tr_core_printf("min %d max %d chg %8.8x flags 0x%2.2x\n",
                                   rep_info->u.send_info.min_interval,
                                   rep_info->u.send_info.max_interval,
                                   rep_info->u.send_info.delta.u32,
                                   rep_info->flags);
                    num_used++;
                }
            }
        }
    }
    tr_core_printf("%d of %d entries used\n", num_used, TR_TOTAL_ATTR_REPORT_TABLE_SIZE);
    return 0;
}

// print buffer usage info
zb_int_t cli_cmd_buffer_info(zb_int_t  argc,
                             zb_char_t *argv[])
{
    tr_core_printf("Buffer usage: %d in, %d out, total %d of %d\n",
                   ZG->bpool.bufs_allocated[0],
                   ZG->bpool.bufs_allocated[1],
                   (ZG->bpool.bufs_allocated[0] + ZG->bpool.bufs_allocated[1]),
                   ZB_IOBUF_POOL_SIZE);

    return 0;
}

static void print_counters_sync_cb(zb_uint8_t param)
{
    tr_core_printf("\nMAC RX Bcast:         %d\n", diagnostics_ctx_zcl.mac_data.mac_rx_bcast);
    tr_core_printf("MAC TX Bcast:         %d\n", diagnostics_ctx_zcl.mac_data.mac_tx_bcast);
    tr_core_printf("MAC RX Ucast:         %d\n", diagnostics_ctx_zcl.mac_data.mac_rx_ucast);
    tr_core_printf("MAC TX Ucast:         %d\n", diagnostics_ctx_zcl.mac_data.mac_tx_ucast_total_zcl);
    tr_core_printf("MAC TX Ucast retries: %d\n", diagnostics_ctx_zcl.mac_data.mac_tx_ucast_retries_zcl);
    tr_core_printf("MAC TX Ucast fail:    %d\n\n", diagnostics_ctx_zcl.mac_data.mac_tx_ucast_failures_zcl);

    tr_core_printf("APS RX Bcast:         %d\n", 0);
    tr_core_printf("APS TX Bcast:         %d\n", diagnostics_ctx_zcl.zdo_data.aps_tx_bcast);
    tr_core_printf("APS RX Ucast:         %d\n", 0);
    tr_core_printf("APS TX Ucast:         %d\n", diagnostics_ctx_zcl.zdo_data.aps_tx_ucast_success);
    tr_core_printf("APS TX Ucast retries: %d\n", diagnostics_ctx_zcl.zdo_data.aps_tx_ucast_retry);
    tr_core_printf("APS TX Ucast fail:    %d\n\n", diagnostics_ctx_zcl.zdo_data.aps_tx_ucast_fail);

    tr_core_printf("Route Disc Initiated: %d\n", diagnostics_ctx_zcl.zdo_data.route_disc_initiated);
    tr_core_printf("Neighbor Added:       %d\n", diagnostics_ctx_zcl.zdo_data.nwk_neighbor_added);
    tr_core_printf("Neighbor Removed:     %d\n", diagnostics_ctx_zcl.zdo_data.nwk_neighbor_removed);
    tr_core_printf("Neighbor Stale:       %d\n", diagnostics_ctx_zcl.zdo_data.nwk_neighbor_stale);
    tr_core_printf("Join Indication:      %d\n", diagnostics_ctx_zcl.zdo_data.join_indication);
    tr_core_printf("Child Moved:          %d\n", diagnostics_ctx_zcl.zdo_data.childs_removed);
    tr_core_printf("NWK FC Fail:          %d\n", diagnostics_ctx_zcl.zdo_data.nwk_fc_failure);
    tr_core_printf("APS FC Fail:          %d\n", diagnostics_ctx_zcl.zdo_data.aps_fc_failure);
    tr_core_printf("APS Unauth Key:       %d\n", diagnostics_ctx_zcl.zdo_data.aps_unauthorized_key);
    tr_core_printf("NWK Decrypt Fail:     %d\n", diagnostics_ctx_zcl.zdo_data.nwk_decrypt_failure);
    tr_core_printf("APS Decrypt Fail:     %d\n", diagnostics_ctx_zcl.zdo_data.aps_decrypt_failure);
    tr_core_printf("Pkt Buf Alloc Fail:   %d\n", diagnostics_ctx_zcl.zdo_data.packet_buffer_allocate_failures);
    tr_core_printf("Relayed Ucast:        %d\n", 0);
    tr_core_printf("Phy To MAC Queue Lim: %d\n", diagnostics_ctx_zcl.mac_data.phy_to_mac_que_lim_reached);
    tr_core_printf("Pkt Validate Drop:    %d\n", diagnostics_ctx_zcl.mac_data.mac_validate_drop_cnt);
    tr_core_printf("Avg MAC Retry/APS:    %d\n\n",
                   diagnostics_ctx_zcl.zdo_data.average_mac_retry_per_aps_message_sent);

    tr_core_printf("MAC CCA Retries:      %d\n", diagnostics_ctx_zcl.mac_data.cca_retries);
    tr_core_printf("MAC PHY CCA Fail:     %d\n", diagnostics_ctx_zcl.mac_data.phy_cca_fail_count);
    tr_core_printf("MAC PHY Queue Full:   %d\n", diagnostics_ctx_zcl.mac_data.phy_to_mac_que_lim_reached);
    tr_core_printf("MAC Validate Fail:    %d\n", diagnostics_ctx_zcl.mac_data.mac_validate_drop_cnt);
    tr_core_printf("Number Of Resets:     %d\n", diagnostics_ctx_zcl.zdo_data.number_of_resets);
    tr_core_printf("NWK Bcast Table Full: %d\n", diagnostics_ctx_zcl.zdo_data.nwk_bcast_table_full);

    tr_core_printf("MAC last msg lqi:     %d\n", diagnostics_ctx_zcl.mac_data.last_msg_lqi);
    tr_core_printf("MAC last msg rssi:   %d\n", diagnostics_ctx_zcl.mac_data.last_msg_rssi);

    // since this is run in a callback, force the prompt
    tr_cli_prompt();
}

zb_int_t cli_cmd_print_counters(zb_int_t  argc,
                                zb_char_t *argv[])
{
    zb_ret_t ret;

    ret = zb_zcl_diagnostics_sync_counters(0, print_counters_sync_cb);

    if (RET_OK != ret)
    {
        tr_core_printf("Failed to request diagnostic counters, status 0x%x\n", ret);
    }
    return 0;
}

static zb_uint8_t g_token_data[200];

static void tr_print_token_data(zb_uint8_t token_len)
{
    zb_uint8_t i;

    for (i = 0 ; i < token_len ; i++)
    {
        tr_core_printf("%2.2x ", g_token_data[i]);
    }
    tr_core_printf("\n");
}

int cli_cmd_print_mfg_tokens(zb_int_t  argc,
                             zb_char_t *argv[])
{
    tr_core_printf("TR_MFG_TOKEN_VERSION: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_VERSION);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_VERSION));

    tr_core_printf("TR_MFG_TOKEN_CUSTOM_EUI: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_CUSTOM_EUI);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_CUSTOM_EUI));

    tr_core_printf("TR_MFG_TOKEN_MFG_NAME: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_MFG_NAME);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_MFG_NAME));

    tr_core_printf("TR_MFG_TOKEN_MODEL_NAME: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_MODEL_NAME);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_MODEL_NAME));

    tr_core_printf("TR_MFG_TOKEN_HW_VERSION: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_HW_VERSION);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_HW_VERSION));

    tr_core_printf("TR_MFG_TOKEN_MANUF_ID: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_MANUF_ID);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_MANUF_ID));

    tr_core_printf("TR_MFG_TOKEN_SERIAL_NUM: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_SERIAL_NUM);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_SERIAL_NUM));

    tr_core_printf("TR_MFG_TOKEN_XTAL_TRIM: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_XTAL_TRIM);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_XTAL_TRIM));

    tr_core_printf("TR_MFG_TOKEN_PHY_CONFIG: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_PHY_CONFIG);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_PHY_CONFIG));

    tr_core_printf("TR_MFG_TOKEN_CCA_THRESHOLD: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_CCA_THRESHOLD);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_CCA_THRESHOLD));

    tr_core_printf("TR_MFG_TOKEN_CBKE_DATA: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_CBKE_DATA);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_CBKE_DATA));

    tr_core_printf("TR_MFG_TOKEN_INSTALLATION_CODE: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_INSTALLATION_CODE);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_INSTALLATION_CODE));

    tr_core_printf("TR_MFG_TOKEN_DISTRIBUTED_KEY: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_DISTRIBUTED_KEY);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_DISTRIBUTED_KEY));

    tr_core_printf("TR_MFG_TOKEN_SECURITY_CONFIG: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_SECURITY_CONFIG);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_SECURITY_CONFIG));

    tr_core_printf("TR_MFG_TOKEN_CBKE_283K1_DATA: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_CBKE_283K1_DATA);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_CBKE_283K1_DATA));

    tr_core_printf("TR_MFG_TOKEN_NVM_CRYPTO_KEY: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_NVM_CRYPTO_KEY);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_NVM_CRYPTO_KEY));

    tr_core_printf("TR_MFG_TOKEN_BOOTLOAD_AES_KEY: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_BOOTLOAD_AES_KEY);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_BOOTLOAD_AES_KEY));

    tr_core_printf("TR_MFG_TOKEN_SECURE_BOOTLOADER_KEY: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_SECURE_BOOTLOADER_KEY);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_SECURE_BOOTLOADER_KEY));

    tr_core_printf("TR_MFG_TOKEN_SIGNED_BOOTLOADER_KEY_X: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_SIGNED_BOOTLOADER_KEY_X);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_SIGNED_BOOTLOADER_KEY_X));

    tr_core_printf("TR_MFG_TOKEN_SIGNED_BOOTLOADER_KEY_Y: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_SIGNED_BOOTLOADER_KEY_Y);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_SIGNED_BOOTLOADER_KEY_Y));

    tr_core_printf("TR_MFG_TOKEN_SERIAL_BOOT_DELAY_SEC: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_SERIAL_BOOT_DELAY_SEC);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_SERIAL_BOOT_DELAY_SEC));

    tr_core_printf("TR_MFG_TOKEN_THREAD_JOIN_KEY: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_THREAD_JOIN_KEY);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_THREAD_JOIN_KEY));

    tr_core_printf("TR_MFG_TOKEN_ZWAVE_COUNTRY_FREQ: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_ZWAVE_COUNTRY_FREQ);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_ZWAVE_COUNTRY_FREQ));

    tr_core_printf("TR_MFG_TOKEN_ZWAVE_INITIALIZED: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_ZWAVE_INITIALIZED);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_ZWAVE_INITIALIZED));

    tr_core_printf("TR_MFG_TOKEN_ZWAVE_QR_CODE: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_ZWAVE_QR_CODE);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_ZWAVE_QR_CODE));

    tr_core_printf("TR_MFG_TOKEN_ZWAVE_PUK: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_ZWAVE_PUK);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_ZWAVE_PUK));

    tr_core_printf("TR_MFG_TOKEN_ZWAVE_PRK: ");
    tr_get_mfg_token(g_token_data, TR_MFG_TOKEN_ZWAVE_PRK);
    tr_print_token_data(tr_get_mfg_token_len(TR_MFG_TOKEN_ZWAVE_PRK));

    return 0;
}

int cli_cmd_print_addr_table(zb_int_t  argc,
                             zb_char_t *argv[])
{
    zb_ushort_t i;
    zb_uint8_t  x;

    for (i = 0 ; i < ZB_IEEE_ADDR_TABLE_SIZE ; i++)
    {
        zb_address_map_t *ent;
        zb_ieee_addr_t   ieee_addr;

        ent = &ZG->addr.addr_map[i];

        if (ZB_U2B(ZG->addr.addr_map[i].used))
        {
            if (ent->redirect_type == ZB_ADDR_REDIRECT_NONE)
            {
                zb_ieee_addr_decompress(ieee_addr, &ent->ieee_addr);

                tr_core_printf("REGULAR: idx %2d, short 0x%4.4x, ieee 0x",
                               i,
                               ent->addr);

                for (x = 0 ; x < 8 ; x++)
                {
                    tr_core_printf("%2.2x", ieee_addr[7 - x]);
                }
                tr_core_printf(", back_ref %d, lock %d\n",
                               ent->redirect_ref,
                               ent->lock_cnt);
            }
            else if (ent->redirect_type == ZB_ADDR_REDIRECT_SHORT)
            {
                tr_core_printf("REDIR_SHORT: idx %2d, short 0x%x, redirect_ref %d, lock %d\n",
                               i,
                               ent->addr,
                               ent->redirect_ref,
                               ent->lock_cnt);
            }
            else
            {
                zb_ieee_addr_decompress(ieee_addr, &ent->ieee_addr);

                tr_core_printf("REDIR_IEEE: idx %2d, ieee 0x", i);

                for (x = 0 ; x < 8 ; x++)
                {
                    tr_core_printf("%2.2x", ieee_addr[7 - x]);
                }
                tr_core_printf(", redirect_ref %d, lock %d\n",
                               ent->redirect_ref,
                               ent->lock_cnt);
            }
        }
    }
    return 0;
}

int cli_cmd_print_alarms(zb_int_t  argc,
                         zb_char_t *argv[])
{
    zb_tm_q_ent_t *ent;
    zb_uint8_t    i;
    zb_uint8_t    nexti;
    zb_uint32_t   curr_time   = ZB_TIMER_GET();
    zb_uint8_t    alarm_count = 0;

    for (i = ZB_POOLED_LIST8_GET_HEAD(ZG->sched.tm_buffer, ZG->sched.tm_queue, next) ;
         i != ZP_NULL8 ; i = nexti)
    {
        nexti = ZB_POOLED_LIST8_NEXT(ZG->sched.tm_buffer, i, next);
        ent   = ZG->sched.tm_buffer + i;

        tr_core_printf("%d: function ptr: 0x%8.8x, time remaining %8d ms, param 0x%8.8x\n",
                       ++alarm_count,
                       (zb_uint32_t)(ent->func) - 1,
                       ZB_TIME_BEACON_INTERVAL_TO_MSEC(ent->run_time - curr_time),
                       ent->param);
    }
    tr_core_printf("Alarms used %d of %d (app alarms total %d, remaining %d)\n",
                   alarm_count,
                   ZB_SCHEDULER_Q_SIZE,
                   (ZB_SCHEDULER_Q_SIZE - ZB_SCHEDULER_Q_SIZE_PROTECTED_STACK_POOL),
                   ((ZB_SCHEDULER_Q_SIZE - ZB_SCHEDULER_Q_SIZE_PROTECTED_STACK_POOL) - alarm_count) > 0 ?
                   (ZB_SCHEDULER_Q_SIZE - ZB_SCHEDULER_Q_SIZE_PROTECTED_STACK_POOL) - alarm_count :
                   0);
    ZVUNUSED(ent);
    ZVUNUSED(curr_time);
    ZVUNUSED(alarm_count);
    return 0;
}

zb_char_t g_task_print_buffer[256];

int cli_cmd_print_tasks(zb_int_t  argc,
                        zb_char_t *argv[])
{
    vTaskList(g_task_print_buffer);
    tr_core_printf("Task          State   Prio    Stack    Num\n");
    tr_core_printf("%s\n", g_task_print_buffer);
    return 0;
}

TR_CLI_COMMAND_TABLE(print_commands) =
{
    { "attr",       cli_cmd_print_attr,                "Print all attributes for the device"                      },
    { "bind",       cli_cmd_print_bind,                "Print the binding table"                                  },
    { "buffers",    cli_cmd_buffer_info,               "Print current buffer usage"                               },
#if defined ZB_COORDINATOR_ROLE || defined ZB_ROUTER_ROLE
    { "neighbor",   cli_cmd_print_neigh_table,         "Print the neighbor table"                                 },
#endif
    { "keys",       cli_cmd_print_keys,                "Print the security keys"                                  },
    { "reporting",  cli_cmd_print_reporting,           "Print attribute reporting"                                },
    { "counters",   cli_cmd_print_counters,            "Print stack counters"                                     },
    { "mfg_tokens", cli_cmd_print_mfg_tokens,          "Print manufacturing tokens"                               },
    { "addr_table", cli_cmd_print_addr_table,          "Print the address table"                                  },
    { "alarms",     cli_cmd_print_alarms,              "Print the schedudled alarms"                              },
    { "tasks",      cli_cmd_print_tasks,               "Print the task information"                               },
#ifdef TR_GROUPS_SERVER_CLI_ENABLE
    { "groups",     cli_cmd_groups_server_print_table, "Print the groups table"                                   },
#endif
    TR_CLI_COMMAND_TABLE_END
};
