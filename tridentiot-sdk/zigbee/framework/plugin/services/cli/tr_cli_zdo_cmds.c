/// ****************************************************************************
/// @file tr_cli_zdo_cmds.c
///
/// @brief CLI for ZDO commands
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include <string.h>
#include "tr_af.h"
#include "tr_cli_argument_parser.h"

// TODO: This might be a selection in the tool and then this define would be generated
#define TR_VERBOSE_ZDO_CLI
// #undef TR_VERBOSE_ZDO_CLI

static void zdo_callback(zb_bufid_t buf)
{
    zb_uint8_t                 *zdp_cmd = zb_buf_begin(buf);
    zb_apsde_data_indication_t *ind     = (zb_apsde_data_indication_t*)ZB_BUF_GET_PARAM(buf, zb_apsde_data_indication_t);

#ifndef TR_VERBOSE_ZDO_CLI
    zb_uint8_t   i;
    zb_buf_ent_t *buf_p = zb_bufpool_storage_bufid_to_buf((buf) - 1);
    tr_core_printf("ZDO cmd 0x%4.4x: ", ind->clusterid);

    for (i = 0 ; i < buf_p->hdr.len ; i++)
    {
        tr_core_printf("%2.2x ", zdp_cmd[i]);
    }
    tr_core_printf("\n");
#else

    switch (ind->clusterid)
    {
        case ZDO_NWK_ADDR_RESP_CLID:
        {
            zb_zdo_nwk_addr_resp_head_t *resp = (zb_zdo_nwk_addr_resp_head_t*)(zdp_cmd);
            tr_core_printf("Network address response: 0x%2.2x\n", resp->status);

            if (resp->status == ZB_ZDP_STATUS_SUCCESS)
            {
                tr_core_printf("   IEEE addr ");
                tr_print_eui64(resp->ieee_addr);
                tr_core_printf(" = 0x%4.4x\n", resp->nwk_addr);
            }
            break;
        }

        case ZDO_IEEE_ADDR_RESP_CLID:
        {
            zb_zdo_ieee_addr_resp_t *resp = (zb_zdo_ieee_addr_resp_t*)(zdp_cmd);
            tr_core_printf("IEEE address response: 0x%2.2x\n", resp->status);

            if (resp->status == ZB_ZDP_STATUS_SUCCESS)
            {
                tr_core_printf("   Short addr 0x%4.4x = ", resp->nwk_addr_remote_dev);
                tr_print_eui64(resp->ieee_addr_remote_dev);
                tr_core_printf("\n");
            }
            break;
        }

        case ZDO_NODE_DESC_RESP_CLID:
        {
            zb_zdo_node_desc_resp_t *resp = (zb_zdo_node_desc_resp_t*)(zdp_cmd);
            tr_core_printf("Node descriptor response: 0x%2.2x\n", resp->hdr.status);

            if (resp->hdr.status == ZB_ZDP_STATUS_SUCCESS)
            {
                tr_core_printf("   Flags             0x%4.4x\n", resp->node_desc.node_desc_flags);
                tr_core_printf("   Capability        0x%2.2x\n", resp->node_desc.mac_capability_flags);
                tr_core_printf("   Mfg code          0x%4.4x\n", resp->node_desc.manufacturer_code);
                tr_core_printf("   Max buf size      0x%4.4x\n", resp->node_desc.max_buf_size);
                tr_core_printf("   Max in tran size  0x%4.4x\n", resp->node_desc.max_incoming_transfer_size);
                tr_core_printf("   Max out tran size 0x%4.4x\n", resp->node_desc.max_outgoing_transfer_size);
                tr_core_printf("   Server mask       0x%4.4x\n", resp->node_desc.server_mask);
                tr_core_printf("   Desc capability   0x%2.2x\n", resp->node_desc.desc_capability_field);
            }
            break;
        }

        case ZDO_POWER_DESC_RESP_CLID:
        {
            zb_zdo_power_desc_resp_t *resp = (zb_zdo_power_desc_resp_t*)(zdp_cmd);
            tr_core_printf("Power descriptor response: 0x%2.2x\n", resp->hdr.status);

            if (resp->hdr.status == ZB_ZDP_STATUS_SUCCESS)
            {
                tr_core_printf("   Flags 0x%4.4x\n", resp->power_desc.power_desc_flags);
            }
            break;
        }

        case ZDO_SIMPLE_DESC_RESP_CLID:
        {
            zb_uint8_t                i;
            zb_zdo_simple_desc_resp_t *resp = (zb_zdo_simple_desc_resp_t*)(zdp_cmd);
            tr_core_printf("Simple descriptor response: 0x%2.2x\n", resp->hdr.status);

            if (resp->hdr.status == ZB_ZDP_STATUS_SUCCESS)
            {
                tr_core_printf("   Endpoint      0x%2.2x\n", resp->simple_desc.endpoint);
                tr_core_printf("   Profile ID    0x%4.4x\n", resp->simple_desc.app_profile_id);
                tr_core_printf("   Device ID     0x%4.4x\n", resp->simple_desc.app_device_id);
                tr_core_printf("   Device Ver    0x%4.4x\n", resp->simple_desc.app_device_version);
                tr_core_printf("   Num in clust  %d:", resp->simple_desc.app_input_cluster_count);

                for (i = 0 ; i < resp->simple_desc.app_input_cluster_count ; i++)
                {
                    tr_core_printf(" 0x%4.4x", resp->simple_desc.app_cluster_list[i]);
                }
                tr_core_printf("\n   Num out clust %d:", resp->simple_desc.app_output_cluster_count);

                for (i = resp->simple_desc.app_input_cluster_count ;
                     i < resp->simple_desc.app_input_cluster_count + resp->simple_desc.app_output_cluster_count ;
                     i++)
                {
                    tr_core_printf(" 0x%4.4x", resp->simple_desc.app_cluster_list[i]);
                }
                tr_core_printf("\n");
            }
            break;
        }

        case ZDO_ACTIVE_EP_RESP_CLID:
        {
            zb_uint8_t       i;
            zb_uint8_t       *ptr;
            zb_zdo_ep_resp_t *resp = (zb_zdo_ep_resp_t*)(zdp_cmd);
            tr_core_printf("Active endpoint response: 0x%2.2x\n", resp->status);

            if (resp->status == ZB_ZDP_STATUS_SUCCESS)
            {
                tr_core_printf("   Count %d:", resp->ep_count);
                ptr = &resp->ep_count + 1;

                for (i = 0 ; i < resp->ep_count ; i++)
                {
                    tr_core_printf(" %d", ptr[i]);
                }
                tr_core_printf("\n");
            }
            ZVUNUSED(ptr);
            break;
        }

        case ZDO_MATCH_DESC_RESP_CLID:
        {
            zb_uint8_t               entry;
            zb_uint8_t               *ptr;
            zb_zdo_match_desc_resp_t *resp = (zb_zdo_match_desc_resp_t*)(zdp_cmd);
            tr_core_printf("Match descriptor response: 0x%2.2x\n", resp->status);

            if (resp->status == ZB_ZDP_STATUS_SUCCESS)
            {
                tr_core_printf("   Short addr: 0x%4.4x\n", resp->nwk_addr);
                tr_core_printf("   Match len:  0x%2.2x\n", resp->match_len);
                ptr = &resp->match_len + 1;
                tr_core_printf("   Ep:");

                for (entry = 0 ; entry < resp->match_len ; entry++)
                {
                    tr_core_printf(" %d", *ptr);
                    ptr++;
                }
                tr_core_printf("\n");
            }
            ZVUNUSED(ptr);
            break;
        }

        case ZDO_BIND_RESP_CLID:
        {
            zb_zdo_bind_resp_t *resp = (zb_zdo_bind_resp_t*)(zdp_cmd);
            tr_core_printf("Bind response: 0x%2.2x\n", resp->status);
            ZVUNUSED(resp);
            break;
        }

        case ZDO_UNBIND_RESP_CLID:
        {
            zb_zdo_bind_resp_t *resp = (zb_zdo_bind_resp_t*)(zdp_cmd);
            tr_core_printf("Unbind response: 0x%2.2x\n", resp->status);
            ZVUNUSED(resp);
            break;
        }

        case ZDO_MGMT_LQI_RESP_CLID:
        {
            zb_zdo_mgmt_lqi_resp_t *resp = (zb_zdo_mgmt_lqi_resp_t*)(zdp_cmd);
            tr_core_printf("Mgmt LQI response: 0x%2.2x\n", resp->status);

            if (resp->status == ZB_ZDP_STATUS_SUCCESS)
            {
                zb_uint8_t                     entry;
                void                           *neighbors_list = (void*)(resp + 1);
                zb_zdo_neighbor_table_record_t *neighbor;
                tr_core_printf("   Total neigh table entries: %d\n", resp->neighbor_table_entries);
                tr_core_printf("   Table list count:          %d\n", resp->neighbor_table_list_count);

                for (entry = 0 ; entry < resp->neighbor_table_list_count ; entry++)
                {
                    neighbor = &((zb_zdo_neighbor_table_record_t*)neighbors_list)[entry];
                    tr_core_printf("   Entry: %d\n", entry + resp->start_index);
                    tr_core_printf("      Short: 0x%4.4x\n", neighbor->network_addr);
                    tr_core_printf("      EUI:   ");
                    tr_print_eui64(neighbor->ext_addr);
                    tr_core_printf("\n      LQI:   %d\n", neighbor->lqa);
                }
            }
            break;
        }

        case ZDO_MGMT_BIND_RESP_CLID:
        {
            zb_uint8_t                    entry;
            zb_zdo_mgmt_bind_resp_t       *resp = (zb_zdo_mgmt_bind_resp_t*)(zdp_cmd);
            zb_zdo_binding_table_record_t *binding_table_records;
            tr_core_printf("Mgmt bind response: 0x%2.2x\n", resp->status);

            if (resp->status == ZB_ZDP_STATUS_SUCCESS)
            {
                void                          *binding_list = (void*)(resp + 1);
                zb_zdo_binding_table_record_t *binding_record;
                binding_table_records = (zb_zdo_binding_table_record_t*)binding_list;

                tr_core_printf("   Total table entries %d\n", resp->binding_table_entries);
                tr_core_printf("   Num table entries %d\n", resp->binding_table_list_count);
                tr_core_printf("   Start index %d\n", resp->start_index);

                for (entry = 0 ; entry < resp->binding_table_list_count ; entry++)
                {
                    binding_record = &binding_table_records[entry];
                    tr_core_printf("   Entry %d:\n", entry + resp->start_index);
                    tr_core_printf("      Src eui:       ");
                    tr_print_eui64(binding_record->src_address);
                    tr_core_printf("\n      Src ep:        %d\n", binding_record->src_endp);
                    tr_core_printf("      Cluster id:    0x%4.4x\n", binding_record->cluster_id);
                    tr_core_printf("      Dst addr mode: 0x%2.2x\n", binding_record->dst_addr_mode);

                    if (binding_record->dst_addr_mode == ZB_BIND_DST_ADDR_MODE_64_BIT_EXTENDED)
                    {
                        tr_core_printf("      Dst eui:       ");
                        tr_print_eui64(binding_record->dst_address.addr_long);
                    }
                    else
                    {
                        tr_core_printf("      \nDst short: %d\n", binding_record->dst_address.addr_short);
                    }
                    tr_core_printf("\n      Dst ep:        %d\n", binding_record->dst_endp);
                }
            }
            break;
        }

        case ZDO_MGMT_LEAVE_RESP_CLID:
        {
            zb_zdo_mgmt_leave_res_t *resp = (zb_zdo_mgmt_leave_res_t*)(zdp_cmd);
            tr_core_printf("Mgmt leave response: 0x%2.2x\n", resp->status);
            ZVUNUSED(resp);
            break;
        }

        default:
            break;
    }
#endif /* ifndef TR_VERBOSE_ZDO_CLI */
    zb_buf_free(buf);
}

zb_int_t cli_cmd_zdo_power(zb_int_t  argc,
                           zb_char_t *argv[])
{
    zb_uint8_t  ret_val = ZB_TRUE;
    zb_uint16_t dest_addr;
    zb_char_t   *option_argument;

    if (tr_cli_get_option(argc, argv, "a:", &option_argument))
    {
        dest_addr = tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        zb_bufid_t              buf  = zb_buf_get_out();
        zb_zdo_power_desc_req_t *req = zb_buf_initial_alloc(buf, sizeof(zb_zdo_power_desc_req_t));
        req->nwk_addr                = dest_addr;
        zdo_send_req_by_short(ZDO_POWER_DESC_REQ_CLID, buf, zdo_callback, dest_addr, ZB_ZDO_CB_DEFAULT_COUNTER);
    }
    else
    {
        tr_core_printf("usage: power -a short addr\n");
    }
    return 0;
}

zb_int_t cli_cmd_zdo_lqi(zb_int_t  argc,
                         zb_char_t *argv[])
{
    zb_uint8_t  ret_val     = ZB_TRUE;
    zb_uint8_t  start_index = 0;
    zb_uint16_t dest_addr;
    zb_char_t   *option_argument;

    if (tr_cli_get_option(argc, argv, "a:", &option_argument))
    {
        dest_addr = tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    if (tr_cli_get_option(argc, argv, "i:", &option_argument))
    {
        start_index = tr_dec_or_hex_string_to_int(option_argument);
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        zb_bufid_t            buf  = zb_buf_get_out();
        zb_zdo_mgmt_lqi_req_t *req = zb_buf_initial_alloc(buf, sizeof(zb_zdo_mgmt_lqi_req_t));
        req->start_index           = start_index;
        zdo_send_req_by_short(ZDO_MGMT_LQI_REQ_CLID, buf, zdo_callback, dest_addr, ZB_ZDO_CB_DEFAULT_COUNTER);
    }
    else
    {
        tr_core_printf("usage: lqi -a short addr [-i start index <default 0>]\n");
    }
    return 0;
}

zb_int_t cli_cmd_zdo_mgmt_bind(zb_int_t  argc,
                               zb_char_t *argv[])
{
    zb_uint8_t  ret_val     = ZB_TRUE;
    zb_uint8_t  start_index = 0;
    zb_uint16_t dest_addr;
    zb_char_t   *option_argument;

    if (tr_cli_get_option(argc, argv, "a:", &option_argument))
    {
        dest_addr = tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    if (tr_cli_get_option(argc, argv, "i:", &option_argument))
    {
        start_index = tr_dec_or_hex_string_to_int(option_argument);
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        zb_bufid_t             buf  = zb_buf_get_out();
        zb_zdo_mgmt_bind_req_t *req = zb_buf_initial_alloc(buf, sizeof(zb_zdo_mgmt_bind_req_t));
        req->start_index            = start_index;
        zdo_send_req_by_short(ZDO_MGMT_BIND_REQ_CLID, buf, zdo_callback, dest_addr, ZB_ZDO_CB_DEFAULT_COUNTER);
    }
    else
    {
        tr_core_printf("usage: req_bind -a short addr [-i start index <default 0>]\n");
    }
    return 0;
}

zb_int_t cli_cmd_zdo_leave(zb_int_t  argc,
                           zb_char_t *argv[])
{
    zb_uint8_t  ret_val         = ZB_TRUE;
    zb_uint8_t  rejoin          = 0;
    zb_uint8_t  remove_children = 0;
    zb_uint16_t dest_addr;
    zb_uint64_t eui64 = 0;
    zb_char_t   *option_argument;

    if (tr_cli_get_option(argc, argv, "a:", &option_argument))
    {
        dest_addr = tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    if (tr_cli_get_option(argc, argv, "l:", &option_argument))
    {
        eui64 = tr_dec_or_hex_string_to_int(option_argument);
    }

    if (tr_cli_get_option(argc, argv, "c", &option_argument))
    {
        remove_children = 1;
    }

    if (tr_cli_get_option(argc, argv, "r", &option_argument))
    {
        rejoin = 1;
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        zb_bufid_t              buf  = zb_buf_get_out();
        zb_zdo_mgmt_leave_req_t *req = zb_buf_initial_alloc(buf, sizeof(zb_zdo_mgmt_leave_req_t));
        req->rejoin                  = rejoin;
        req->remove_children         = remove_children;
        ZB_MEMCPY(req->device_address, &eui64, sizeof(req->device_address));
        zdo_send_req_by_short(ZDO_MGMT_LEAVE_REQ_CLID, buf, zdo_callback, dest_addr, ZB_ZDO_CB_DEFAULT_COUNTER);
    }
    else
    {
        tr_core_printf("usage: leave -a short addr [-l eui64] [-c remove children <default don't remove children>] [-r rejoin <default don't rejoin>]\n");
    }
    return 0;
}

zb_int_t cli_cmd_zdo_endpoint(zb_int_t  argc,
                              zb_char_t *argv[])
{
    zb_uint8_t  ret_val = ZB_TRUE;
    zb_uint16_t dest_addr;
    zb_char_t   *option_argument;

    if (tr_cli_get_option(argc, argv, "a:", &option_argument))
    {
        dest_addr = tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        zb_bufid_t             buf  = zb_buf_get_out();
        zb_zdo_active_ep_req_t *req = zb_buf_initial_alloc(buf, sizeof(zb_zdo_active_ep_req_t));
        req->nwk_addr               = dest_addr;
        zdo_send_req_by_short(ZDO_ACTIVE_EP_REQ_CLID, buf, zdo_callback, dest_addr, ZB_ZDO_CB_DEFAULT_COUNTER);
    }
    else
    {
        tr_core_printf("usage: endpoint -a short addr\n");
    }
    return 0;
}

zb_int_t cli_cmd_zdo_bind(zb_int_t  argc,
                          zb_char_t *argv[])
{
    zb_uint8_t  ret_val    = ZB_TRUE;
    zb_uint8_t  dest_ep    = 1;
    zb_uint8_t  src_ep     = 1;
    zb_uint16_t dest_addr  = 0xFFFF;
    zb_uint16_t cluster_id = 0xFFFF;
    zb_uint64_t rem_eui64;
    zb_uint64_t targ_eui64;
    zb_char_t   *option_argument;

    if (tr_cli_get_option(argc, argv, "a:", &option_argument))
    {
        dest_addr = tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    if (tr_cli_get_option(argc, argv, "c:", &option_argument))
    {
        cluster_id = tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    if (tr_cli_get_option(argc, argv, "r:", &option_argument))
    {
        rem_eui64 = tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // TODO: figure out why we can't use the tartget eui64, but the cli doesn't accept it
    // if (tr_cli_get_option(argc, argv, "t:", &option_argument))
    if (0)
    {
        targ_eui64 = tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        zb_get_long_address((zb_uint8_t*)&targ_eui64);
    }

    if (tr_cli_get_option(argc, argv, "d:", &option_argument))
    {
        dest_ep = tr_dec_or_hex_string_to_int(option_argument);
    }

    if (tr_cli_get_option(argc, argv, "s:", &option_argument))
    {
        src_ep = tr_dec_or_hex_string_to_int(option_argument);
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        zb_uint8_t             *ptr;
        zb_bufid_t             buf  = zb_buf_get_out();
        zb_zdo_bind_req_head_t *req = zb_buf_initial_alloc(buf,
                                                           sizeof(zb_zdo_bind_req_head_t) +
                                                           sizeof(zb_ieee_addr_t) + 1);
        req->src_endp   = src_ep;
        req->cluster_id = cluster_id;
        ZB_MEMCPY(req->src_address, &rem_eui64, sizeof(req->src_address));
        req->dst_addr_mode = ZB_BIND_DST_ADDR_MODE_64_BIT_EXTENDED;
        ptr                = &req->dst_addr_mode + 1;
        ZB_MEMCPY(ptr, &targ_eui64, sizeof(targ_eui64));
        ptr += sizeof(targ_eui64);
        *ptr = dest_ep;
        zdo_send_req_by_short(ZDO_BIND_REQ_CLID, buf, zdo_callback, dest_addr, ZB_ZDO_CB_DEFAULT_COUNTER);
    }
    else
    {
        // TODO: figure out why we can't use the tartget eui64, but the cli doesn't accept it
        // tr_core_printf("usage: bind -a short_addr -c cluster_id -r remote_eui64 [-t target_eui64] [-s ep] [-d ep]\n");
        tr_core_printf("usage: bind -a short addr -c cluster id -r remote eui64 [-s source ep <default 1>] [-d dest ep <default 1>]\n");
    }
    return 0;
}

zb_int_t cli_cmd_zdo_node(zb_int_t  argc,
                          zb_char_t *argv[])
{
    zb_uint8_t  ret_val = ZB_TRUE;
    zb_uint16_t dest_addr;
    zb_char_t   *option_argument;

    if (tr_cli_get_option(argc, argv, "a:", &option_argument))
    {
        dest_addr = tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        zb_bufid_t             buf  = zb_buf_get_out();
        zb_zdo_node_desc_req_t *req = zb_buf_initial_alloc(buf, sizeof(zb_zdo_node_desc_req_t));
        req->nwk_addr               = dest_addr;
        zdo_send_req_by_short(ZDO_NODE_DESC_REQ_CLID, buf, zdo_callback, dest_addr, ZB_ZDO_CB_DEFAULT_COUNTER);
    }
    else
    {
        tr_core_printf("usage: node -a short addr\n");
    }
    return 0;
}

zb_int_t cli_cmd_zdo_match(zb_int_t  argc,
                           zb_char_t *argv[])
{
    zb_uint8_t  ret_val          = ZB_TRUE;
    zb_uint8_t  num_in_clusters  = 0;
    zb_uint8_t  num_out_clusters = 0;
    zb_uint16_t dest_addr        = ZB_NWK_BROADCAST_RX_ON_WHEN_IDLE;
    zb_uint16_t out_cluster_id;
    zb_uint16_t in_cluster_id;
    zb_uint16_t profile_id = ZB_AF_HA_PROFILE_ID;
    zb_char_t   *option_argument;

    if (tr_cli_get_option(argc, argv, "a:", &option_argument))
    {
        dest_addr = tr_dec_or_hex_string_to_int(option_argument);
    }

    if (tr_cli_get_option(argc, argv, "o:", &option_argument))
    {
        out_cluster_id   = tr_dec_or_hex_string_to_int(option_argument);
        num_out_clusters = 1;
    }

    if (tr_cli_get_option(argc, argv, "i:", &option_argument))
    {
        in_cluster_id   = tr_dec_or_hex_string_to_int(option_argument);
        num_in_clusters = 1;
    }

    if (tr_cli_get_option(argc, argv, "p:", &option_argument))
    {
        profile_id = tr_dec_or_hex_string_to_int(option_argument);
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        zb_uint8_t                   *ptr;
        zb_bufid_t                   buf  = zb_buf_get_out();
        zb_zdo_match_desc_req_head_t *req = zb_buf_initial_alloc(buf,
                                                                 sizeof(zb_zdo_match_desc_req_head_t) +
                                                                 num_in_clusters * sizeof(zb_uint16_t) +
                                                                 sizeof(zb_zdo_match_desc_req_tail_t) +
                                                                 num_out_clusters * sizeof(zb_uint16_t));
        zb_zdo_match_desc_req_tail_t *req_tail;
        req->nwk_addr        = dest_addr;
        req->profile_id      = profile_id;
        req->num_in_clusters = num_in_clusters;
        ptr                  = &req->num_in_clusters + 1;

        if (num_in_clusters != 0)
        {
            *(zb_uint16_t*)ptr = in_cluster_id;
            ptr               += sizeof(zb_uint16_t);
        }
        req_tail                   = (zb_zdo_match_desc_req_tail_t*)ptr;
        req_tail->num_out_clusters = num_out_clusters;
        ptr                        = &req_tail->num_out_clusters + 1;

        if (num_out_clusters != 0)
        {
            *(zb_uint16_t*)ptr = out_cluster_id;
        }
        zdo_send_req_by_short(ZDO_MATCH_DESC_REQ_CLID, buf, zdo_callback, dest_addr, ZB_ZDO_CB_DEFAULT_COUNTER);
    }
    else
    {
        tr_core_printf("usage: match [-a short addr] [-p profile id <default 0x104 (HA)>] [-i in cluster] [-o out cluster]\n");
    }
    return 0;
}

zb_int_t cli_cmd_zdo_simple(zb_int_t  argc,
                            zb_char_t *argv[])
{
    zb_uint8_t  ret_val = ZB_TRUE;
    zb_uint8_t  dest_ep = 1;
    zb_uint16_t dest_addr;
    zb_char_t   *option_argument;

    if (tr_cli_get_option(argc, argv, "a:", &option_argument))
    {
        dest_addr = tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    if (tr_cli_get_option(argc, argv, "e:", &option_argument))
    {
        dest_ep = tr_dec_or_hex_string_to_int(option_argument);
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        zb_bufid_t               buf  = zb_buf_get_out();
        zb_zdo_simple_desc_req_t *req = zb_buf_initial_alloc(buf, sizeof(zb_zdo_simple_desc_req_t));
        req->nwk_addr                 = dest_addr;
        req->endpoint                 = dest_ep;
        zdo_send_req_by_short(ZDO_SIMPLE_DESC_REQ_CLID, buf, zdo_callback, dest_addr, ZB_ZDO_CB_DEFAULT_COUNTER);
    }
    else
    {
        tr_core_printf("usage: simple -a short addr [-e dest ep <default 1>]\n");
    }
    return 0;
}

zb_int_t cli_cmd_zdo_ieee(zb_int_t  argc,
                          zb_char_t *argv[])
{
    zb_uint8_t  ret_val = ZB_TRUE;
    zb_uint16_t dest_addr;
    zb_char_t   *option_argument;

    if (tr_cli_get_option(argc, argv, "a:", &option_argument))
    {
        dest_addr = tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        zb_bufid_t                   buf  = zb_buf_get_out();
        zb_zdo_ieee_addr_req_param_t *req = zb_buf_initial_alloc(buf, sizeof(zb_zdo_ieee_addr_req_param_t));
        req->nwk_addr                     = dest_addr;
        req->dst_addr                     = dest_addr;
        req->request_type                 = ZB_ZDO_SINGLE_DEVICE_RESP;
        req->start_index                  = 0;
        zdo_send_req_by_short(ZDO_IEEE_ADDR_REQ_CLID, buf, zdo_callback, dest_addr, ZB_ZDO_CB_DEFAULT_COUNTER);
    }
    else
    {
        tr_core_printf("usage: ieee -a short addr\n");
    }
    return 0;
}

zb_int_t cli_cmd_zdo_nwk(zb_int_t  argc,
                         zb_char_t *argv[])
{
    zb_uint8_t  ret_val = ZB_TRUE;
    zb_uint64_t dest_eui64;
    zb_char_t   *option_argument;

    if (tr_cli_get_option(argc, argv, "l:", &option_argument))
    {
        dest_eui64 = tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        zb_bufid_t            buf  = zb_buf_get_out();
        zb_zdo_nwk_addr_req_t *req = zb_buf_initial_alloc(buf, sizeof(zb_zdo_nwk_addr_req_t));
        memcpy(req->ieee_addr, &dest_eui64, sizeof(req->ieee_addr));
        req->request_type = ZB_ZDO_SINGLE_DEVICE_RESP;
        req->start_index  = 0;
        zdo_send_req_by_short(ZDO_NWK_ADDR_REQ_CLID, buf, zdo_callback, ZB_NWK_BROADCAST_RX_ON_WHEN_IDLE, ZB_ZDO_CB_DEFAULT_COUNTER);
    }
    else
    {
        tr_core_printf("usage: nwk -l eui64\n");
    }
    return 0;
}

zb_int_t cli_cmd_zdo_unbind(zb_int_t  argc,
                            zb_char_t *argv[])
{
    zb_uint8_t  ret_val    = ZB_TRUE;
    zb_uint8_t  dest_ep    = 1;
    zb_uint8_t  src_ep     = 1;
    zb_uint16_t dest_addr  = 0xFFFF;
    zb_uint16_t cluster_id = 0xFFFF;
    zb_uint64_t rem_eui64;
    zb_uint64_t targ_eui64;
    zb_char_t   *option_argument;

    if (tr_cli_get_option(argc, argv, "a:", &option_argument))
    {
        dest_addr = tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    if (tr_cli_get_option(argc, argv, "c:", &option_argument))
    {
        cluster_id = tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    if (tr_cli_get_option(argc, argv, "d:", &option_argument))
    {
        dest_ep = tr_dec_or_hex_string_to_int(option_argument);
    }

    if (tr_cli_get_option(argc, argv, "s:", &option_argument))
    {
        src_ep = tr_dec_or_hex_string_to_int(option_argument);
    }

    if (tr_cli_get_option(argc, argv, "r:", &option_argument))
    {
        rem_eui64 = tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    if (tr_cli_get_option(argc, argv, "d:", &option_argument))
    {
        dest_ep = tr_dec_or_hex_string_to_int(option_argument);
    }

    if (tr_cli_get_option(argc, argv, "s:", &option_argument))
    {
        src_ep = tr_dec_or_hex_string_to_int(option_argument);
    }

    // TODO: figure out why we can't use the tartget eui64, but the cli doesn't accept it
    // if (tr_cli_get_option(argc, argv, "t:", &option_argument))
    if (0)
    {
        targ_eui64 = tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        zb_get_long_address((zb_uint8_t*)&targ_eui64);
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        zb_uint8_t             *ptr;
        zb_bufid_t             buf  = zb_buf_get_out();
        zb_zdo_bind_req_head_t *req = zb_buf_initial_alloc(buf,
                                                           sizeof(zb_zdo_bind_req_head_t) +
                                                           sizeof(zb_ieee_addr_t) + 1);
        req->src_endp   = src_ep;
        req->cluster_id = cluster_id;
        ZB_MEMCPY(req->src_address, &rem_eui64, sizeof(req->src_address));
        req->dst_addr_mode = ZB_BIND_DST_ADDR_MODE_64_BIT_EXTENDED;
        ptr                = &req->dst_addr_mode + 1;
        ZB_MEMCPY(ptr, &targ_eui64, sizeof(targ_eui64));
        ptr += sizeof(targ_eui64);
        *ptr = dest_ep;
        zdo_send_req_by_short(ZDO_UNBIND_REQ_CLID, buf, zdo_callback, dest_addr, ZB_ZDO_CB_DEFAULT_COUNTER);
    }
    else
    {
        // TODO: figure out why we can't use the tartget eui64, but the cli doesn't accept it
        // tr_core_printf("usage: unbind -a short_addr -c cluster_id -r remote_eui64 [-t target_eui64] [-s ep] [-d ep]\n");
        tr_core_printf("usage: unbind -a short addr -c cluster id -r remote eui64 [-s source ep <default 1>] [-d dest ep <default 1>]\n");
    }
    return 0;
}

zb_int_t cli_cmd_zdo_announce(zb_int_t  argc,
                              zb_char_t *argv[])
{
    zb_uint8_t  ret_val   = ZB_TRUE;
    zb_uint16_t dest_addr = ZB_PIBCACHE_NETWORK_ADDRESS();
    zb_char_t   *option_argument;

    if (tr_cli_get_option(argc, argv, "a:", &option_argument))
    {
        dest_addr = tr_dec_or_hex_string_to_int(option_argument);
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        zb_bufid_t            buf = zb_buf_get_out();
        zb_zdo_device_annce_t da;

        zdo_tsn_inc();
        da.tsn = ZDO_CTX().tsn;
        ZB_HTOLE16(&da.nwk_addr, &dest_addr);
        ZB_IEEE_ADDR_COPY(da.ieee_addr, ZB_PIBCACHE_EXTENDED_ADDRESS());
        da.capability = 0;
        ZB_MAC_CAP_SET_ALLOCATE_ADDRESS(da.capability, ZB_B2U(ZG->nwk.handle.rejoin_capability_alloc_address));

#ifdef ZB_ROUTER_ROLE

        if (ZB_IS_DEVICE_ZR())
        {
            ZB_MAC_CAP_SET_ROUTER_CAPS(da.capability);
            /* ZB_MAC_CAP_SET_SECURITY means high security mode - never set it */
        }
        else
#endif
        {
            if (ZB_U2B(ZB_PIBCACHE_RX_ON_WHEN_IDLE()))
            {
                ZB_MAC_CAP_SET_RX_ON_WHEN_IDLE(da.capability, 1U);
                ZB_MAC_CAP_SET_POWER_SOURCE(da.capability, 1U);
            }
        }

        zdo_send_device_annce_ex(buf,
                                 &da
#ifdef ZB_USEALIAS
                                 ,
                                 ZB_FALSE
#endif
                                 );

    }
    else
    {
        tr_core_printf("usage: announce [-a short addr]\n");
    }
    return 0;
}

TR_CLI_COMMAND_TABLE(zdo_commands) =
{
    { "power",     cli_cmd_zdo_power,     "Send a power descriptor request"           },
    { "lqi",       cli_cmd_zdo_lqi,       "Send an lqi request"                       },
    { "mgmt_bind", cli_cmd_zdo_mgmt_bind, "Send a binding table request"              },
    { "leave",     cli_cmd_zdo_leave,     "Send a leave request"                      },
    { "endpoint",  cli_cmd_zdo_endpoint,  "Send an active endpoint request"           },
    { "bind",      cli_cmd_zdo_bind,      "Create a binding on a remote device"       },
    { "node",      cli_cmd_zdo_node,      "Send a node descriptor request"            },
    { "match",     cli_cmd_zdo_match,     "Send a match descriptor request"           },
    { "simple",    cli_cmd_zdo_simple,    "Send a simple descriptor request"          },
    { "ieee",      cli_cmd_zdo_ieee,      "Send an IEEE address request"              },
    { "nwk",       cli_cmd_zdo_nwk,       "Send a network address request"            },
    { "unbind",    cli_cmd_zdo_unbind,    "Send an unbind request (group or unicast)" },
    { "announce",  cli_cmd_zdo_announce,  "Send a device announce command"            },
    TR_CLI_COMMAND_TABLE_END
};
