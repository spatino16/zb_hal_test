/// ****************************************************************************
/// @file tr_zcl_common.c
///
/// @brief entry point for all incoming ZCL messages
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_af.h"
#include "zb_bufpool.h"

zb_uint8_t tr_zcl_specific_cluster_cmd_handler(zb_uint8_t param)
{
    zb_uint8_t          i;
    zb_zcl_parsed_hdr_t *cmd_info = ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t);
    zb_buf_ent_t        *buf_p    = zb_bufpool_storage_bufid_to_buf((param) - 1);

#ifdef TR_REMOTE_CLI_CLIENT_PLUGIN_ENABLE

    // don't print received messages to the client on the remote cli cluster
    if ((cmd_info->cluster_id != TR_ZCL_CLUSTER_REMOTE_CLI_ID) || (cmd_info->cmd_direction != ZB_ZCL_FRAME_DIRECTION_TO_CLI))
#else

    if (1)
#endif
    {
        tr_rxmsgs_printf("RX len %d, ep %d, clus 0x%4.4x (%s), FC %2.2x, seq %2.2x, cmd %2.2x payload:",
                         buf_p->hdr.len,
                         cmd_info->addr_data.common_data.dst_endpoint,
                         cmd_info->cluster_id,
                         tr_find_cluster_name(cmd_info->cluster_id),
                         cmd_info->addr_data.common_data.fc,
                         cmd_info->seq_number,
                         cmd_info->cmd_id);

        for (i = 0 ; i < buf_p->hdr.len ; i++)
        {
            tr_rxmsgs_printf(" %2.2x", buf_p->buf[buf_p->hdr.data_offset + i]);
        }
        tr_rxmsgs_printf("\n");
    }

    ZVUNUSED(cmd_info);
    ZVUNUSED(buf_p);

    return tr_zcl_command_cb(param);
}
