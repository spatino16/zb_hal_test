/// ****************************************************************************
/// @file tr_bind.c
///
/// @brief binding plugin that extends binding capabilities especially for
/// groups related behavior.
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_af.h"
#include "tr_bind.h"

zb_bool_t tr_bind_check_groups_binding_exists(zb_uint8_t  endpoint,
                                              zb_uint16_t group_id)
{
    zb_uint8_t dst_index = 0, src_index = 0;

    for (dst_index = 0 ; dst_index < ZG->aps.binding.dst_n_elements ; dst_index++)
    {
        src_index = ZG->aps.binding.dst_table[dst_index].src_table_index;

        if (ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT == ZG->aps.binding.dst_table[dst_index].dst_addr_mode)
        {
            if (ZG->aps.binding.src_table[src_index].src_end == endpoint &&
                ZG->aps.binding.dst_table[dst_index].u.group_addr == group_id)
            {
                return ZB_TRUE;
            }
        }
    }

    return ZB_FALSE;
}

// returns ZB_TRUE if binding created, ZB_FALSE if binding already exists
zb_bool_t tr_bind_add_group_binding(zb_uint8_t  endpoint,
                                    zb_uint16_t group_id)
{
    if (!tr_bind_check_groups_binding_exists(endpoint, group_id))
    {
        zb_bufid_t             bind_buf = zb_buf_get_out();
        zb_apsme_binding_req_t *aps_bind_req;

        aps_bind_req = ZB_BUF_GET_PARAM(bind_buf, zb_apsme_binding_req_t);

        aps_bind_req->src_endpoint        = endpoint;
        aps_bind_req->clusterid           = TR_ZCL_CLUSTER_GROUPS_ID;
        aps_bind_req->addr_mode           = ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT;
        aps_bind_req->dst_addr.addr_short = group_id;
        aps_bind_req->confirm_cb          = NULL;

        zb_apsme_bind_request(bind_buf);

        return ZB_TRUE;
    }
    else
    {
        return ZB_FALSE;
    }
}

// returns ZB_TRUE if binding removed, ZB_FALSE if binding does not exist
zb_bool_t tr_bind_remove_group_binding(zb_uint8_t  endpoint,
                                       zb_uint16_t group_id)
{
    zb_uindex_t s = 0, d = 0;
    zb_uint8_t  found   = 0;
    zb_uint8_t  deleted = 0;
    zb_ret_t    status  = RET_OK;

    /* remove all bindings with this dst and src */
    do
    {
        s = ZG->aps.binding.dst_table[d].src_table_index;

        if (ZG->aps.binding.src_table[s].src_end == endpoint
            && (ZG->aps.binding.dst_table[d].dst_addr_mode == ZB_APS_BIND_DST_ADDR_GROUP
                && ZG->aps.binding.dst_table[d].u.group_addr == group_id))
        {

            /* move all records left */
            ZG->aps.binding.dst_n_elements--;
            zb_apsme_move_dst_bind_table(d, d + 1, ZG->aps.binding.dst_n_elements - d);

            deleted = 1;
        }
        else
        {
            if (ZG->aps.binding.dst_table[d].src_table_index == s)
            {
                found = 1;
            }
            d++;
        }
    }
    while (d < ZG->aps.binding.dst_n_elements);

    if (found == 0U)
    {
        /* remove from src table useless binding record */
        ZG->aps.binding.src_n_elements--;
        ZB_MEMMOVE(&ZG->aps.binding.src_table[s],
                   &ZG->aps.binding.src_table[s + 1U],
                   sizeof(zb_aps_bind_src_table_t) * (ZG->aps.binding.src_n_elements - s));

        /* correct dst table indexes */
        d = 0;

        do
        {
            if (ZG->aps.binding.dst_table[d].src_table_index > s)
            {
                ZG->aps.binding.dst_table[d].src_table_index--;
            }
            d++;
        }
        while (d < ZG->aps.binding.dst_n_elements);
    }
    else
    {
        /* check that binding for dst_table has been deleted */
        status = (deleted == 1U) ? RET_OK : ERROR_CODE(ERROR_CATEGORY_APS, ZB_APS_STATUS_INVALID_BINDING);
    }

    if (status == RET_OK)
    {
#ifdef ZB_USE_NVRAM
        zb_nvram_transaction_start();
        /* If we fail, trace is given and assertion is triggered */
        (void)zb_nvram_write_dataset(ZB_NVRAM_ADDR_MAP);
        (void)zb_nvram_write_dataset(ZB_NVRAM_APS_BINDING_DATA);
        zb_nvram_transaction_commit();
#endif /* ZB_USE_NVRAM */

        return ZB_TRUE;
    }
    else
    {
        return ZB_FALSE;
    }
}

void tr_bind_remove_all_groups_bindings(zb_uint8_t endpoint)
{
    zb_uint8_t dst_index    = 0;
    zb_uint8_t src_index    = 0;
    zb_uint8_t num_bindings = 0;

    num_bindings = ZG->aps.binding.dst_n_elements;

    /* must go through this backwards since the list shrinks as you remove entries */
    for (dst_index = num_bindings ; dst_index != 0 ; dst_index--)
    {
        src_index = ZG->aps.binding.dst_table[dst_index - 1].src_table_index;

        if (ZB_APS_ADDR_MODE_16_GROUP_ENDP_NOT_PRESENT == ZG->aps.binding.dst_table[dst_index - 1].dst_addr_mode)
        {
            if (ZG->aps.binding.src_table[src_index].src_end == endpoint)
            {
                tr_bind_remove_group_binding(endpoint, ZG->aps.binding.dst_table[dst_index - 1].u.group_addr);
            }
        }
    }
}
