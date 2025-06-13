/// ****************************************************************************
/// @file tr_nvram_attr.c
///
/// @brief TODO - documentation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_nvram_attr.h"

void tr_nvram_save_attributes(zb_uint8_t param);

// get the number of bytes needed for writing all attributes to nvram
zb_uint16_t tr_get_nvram_data_size(void)
{
    zb_uint8_t    i;
    zb_uint16_t   size;
    zb_zcl_attr_t *attr_info;
#if TR_NVRAM_STORAGE_CUR_VER == TR_NVRAM_ATTR_STORAGE_VER_1
    tr_nvram_attr_storage_v1_t nvram_attr_storage[] = TR_NVRAM_ATTR_STORAGE_CONFIG;
#endif

    // start with the size of the version, which is 1 byte
    size = 1;

    // now add in the size of the nvram attribute structure array
#if TR_NVRAM_STORAGE_CUR_VER == TR_NVRAM_ATTR_STORAGE_VER_1
    size += (TR_NUM_NVRAM_ATTR * sizeof(tr_nvram_attr_storage_v1_t));
#endif

    // now get the size of each of the attributes that will be stored
    for (i = 0 ; i < TR_NUM_NVRAM_ATTR ; i++)
    {
        // figure out the data size of the attribute itself
        attr_info = zb_zcl_get_attr_desc_manuf_a(
            nvram_attr_storage[i].endpoint,
            nvram_attr_storage[i].cluster_id,
            nvram_attr_storage[i].cluster_role,
            nvram_attr_storage[i].attr_id,
            nvram_attr_storage[i].manuf_code);

        size += zb_zcl_get_attribute_size(attr_info->type, attr_info->data_p);
    }

    // return the size rounded up to a 4 byte boundary
    return ((size + 3) & ~0x03);
}

// callback that writes the nvram attribute data
zb_ret_t tr_nvram_write_app_data_cb(zb_uint8_t  page,
                                    zb_uint32_t pos)
{
    zb_ret_t      ret      = ERROR_CODE(ERROR_CATEGORY_NVRAM, ZB_ERROR_NVRAM_WRITE_FAILURE);
    zb_uint8_t    *ds      = NULL;
    zb_uint8_t    ds_index = 0;
    zb_uint8_t    i;
    zb_zcl_attr_t *attr_info;
    zb_uint8_t    attr_len;
#if TR_NVRAM_STORAGE_CUR_VER == TR_NVRAM_ATTR_STORAGE_VER_1
    tr_nvram_attr_storage_v1_t nvram_attr_storage[] = TR_NVRAM_ATTR_STORAGE_CONFIG;
#endif

    ds = (zb_uint8_t*)TR_MALLOC(tr_get_nvram_data_size());

    if (ds != NULL)
    {
        // start with the nvram attribute dataset version byte
        ds[ds_index] = TR_NVRAM_STORAGE_CUR_VER;
        ds_index    += 1;

        // create the dataset from the nvram attribute structure
        for (i = 0 ; i < TR_NUM_NVRAM_ATTR ; i++)
        {
            attr_info = zb_zcl_get_attr_desc_manuf_a(
                nvram_attr_storage[i].endpoint,
                nvram_attr_storage[i].cluster_id,
                nvram_attr_storage[i].cluster_role,
                nvram_attr_storage[i].attr_id,
                nvram_attr_storage[i].manuf_code);

            // make sure attribute really exists
            if (attr_info != NULL)
            {
                memcpy(&ds[ds_index], &nvram_attr_storage[i].endpoint, 1);
                ds_index += 1;
                memcpy(&ds[ds_index], &nvram_attr_storage[i].cluster_id, 2);
                ds_index += 2;
                memcpy(&ds[ds_index], &nvram_attr_storage[i].attr_id, 2);
                ds_index += 2;
                memcpy(&ds[ds_index], &nvram_attr_storage[i].cluster_role, 1);
                ds_index += 1;
                memcpy(&ds[ds_index], &nvram_attr_storage[i].manuf_code, 2);
                ds_index += 2;
                memcpy(&ds[ds_index], &nvram_attr_storage[i].data_type, 1);
                ds_index += 1;

                // figure out the data size of the attribute and copy the value here
                attr_len = zb_zcl_get_attribute_size(attr_info->type, attr_info->data_p);
                memcpy(&ds[ds_index], attr_info->data_p, attr_len);
                ds_index += attr_len;
            }
        }

#if 0

        for (i = 0 ; i < ds_index ; i++)
        {
            tr_core_printf("%2.2x ", ds[i]);

            if (((i + 1) % 16) == 0)
            {
                tr_core_printf("\n");
            }
        }
        tr_core_printf("\n");
        tr_core_printf("Writing %d bytes to page %d, pos 0x%x\n", ((ds_index + 3) & ~0x03), page, pos);
#endif /* if 0 */

        // write the dataset to nvram, but make sure the length is modulo 4
        ret = zb_nvram_write_data(page, pos, ds, ((ds_index + 3) & ~0x03));
        TR_FREE(ds);
    }
    else
    {
        tr_core_printf("%s: failed to malloc dataset buffer\n", __func__);
    }
    return ret;
}

// this will set the attribute value on other endpoints
// it is only called if the attribute is a singleton
static void tr_conform_singleton(zb_uint8_t  ep_first,
                                 zb_uint16_t cluster_id,
                                 zb_uint8_t  cluster_role,
                                 zb_uint16_t attr_id,
                                 zb_uint8_t  *value,
                                 zb_uint16_t manuf_code)
{
    zb_uint8_t            ep_index;
    zb_af_endpoint_desc_t **ep_desc_list;

    // walk the endpoints
    for (ep_index = 0 ; ep_index < ZCL_CTX().device_ctx->ep_count ; ep_index++)
    {
        ep_desc_list = ZCL_CTX().device_ctx->ep_desc_list;

        // we have already taken care of the "first" endpoint before calling this function
        if (ep_desc_list[ep_index]->ep_id != ep_first)
        {
            zb_zcl_attr_t         *attr_desc;
            zb_zcl_cluster_desc_t *cluster_desc = get_cluster_desc(
                ep_desc_list[ep_index],
                cluster_id,
                cluster_role);

            // make sure the cluster is supported on this endpoint
            if (cluster_desc == NULL)
            {
                continue;
            }

            attr_desc = zb_zcl_get_attr_desc_manuf(cluster_desc, attr_id, manuf_code);

            // make sure the attribute is supported on this endpoint
            if (attr_desc == NULL)
            {
                continue;
            }

            // write the nvram value to this attribute
            memcpy(attr_desc->data_p, value, zb_zcl_get_attribute_size(attr_desc->type, value));

        }
    }
}

// callback when nvram attribute data is read
void tr_nvram_read_app_data_cb(zb_uint8_t  page,
                               zb_uint32_t pos,
                               zb_uint16_t payload_length)
{
    zb_ret_t      ret;
    zb_uint8_t    *ds      = NULL;
    zb_uint8_t    ds_index = 0;
    zb_uint8_t    ds_version;
    zb_uint8_t    endpoint;
    zb_uint16_t   cluster_id;
    zb_uint16_t   attr_id;
    zb_uint8_t    is_server;
    zb_uint16_t   manuf_code;
    zb_zcl_attr_t *attr_info;
    zb_uint8_t    attr_len;

    // get memory for reading the dataset
    ds = (zb_uint8_t*)TR_MALLOC(payload_length);

    if (ds != NULL)
    {
        // go read the data from nvram
        ret = zb_nvram_read_data(page, pos, ds, payload_length);

        if (ret == RET_OK)
        {
#if 0
            zb_uint8_t i;

            for (i = 0 ; i < payload_length ; i++)
            {
                tr_core_printf("%2.2x ", ds[i]);

                if (((i + 1) % 16) == 0)
                {
                    tr_core_printf("\n");
                }
            }
            tr_core_printf("\n");
#endif /* if 0 */

            ds_version = ds[ds_index];
            ds_index  += 1;

            switch (ds_version)
            {
                case TR_NVRAM_ATTR_STORAGE_VER_1:
                {
                    zb_uint8_t data_type;

                    // do this loop while the payload left if big enough to handle at least 1 entry
                    // we have to get fancy here because the payload may have pad bytes at the end
                    while ((ds_index < payload_length) &&
                           ((payload_length - ds_index) >= (sizeof(tr_nvram_attr_storage_v1_t) + 1)))
                    {
                        memcpy(&endpoint, &ds[ds_index], 1);
                        ds_index += 1;
                        memcpy(&cluster_id, &ds[ds_index], 2);
                        ds_index += 2;
                        memcpy(&attr_id, &ds[ds_index], 2);
                        ds_index += 2;
                        memcpy(&is_server, &ds[ds_index], 1);
                        ds_index += 1;
                        memcpy(&manuf_code, &ds[ds_index], 2);
                        ds_index += 2;
                        memcpy(&data_type, &ds[ds_index], 1);
                        ds_index += 1;

                        // look up the attribute with the cluster info from the dataset
                        attr_info = zb_zcl_get_attr_desc_manuf_a(
                            endpoint,
                            cluster_id,
                            is_server,
                            attr_id,
                            manuf_code);

                        // get the attribute length
                        attr_len = zb_zcl_get_attribute_size(data_type, &ds[ds_index]);
#if 0
                        tr_core_printf("ep %d, clust 0x%4.4x, attr 0x%4.4x, len %d, srvr %d, mfg code 0x%4.4x, type 0x%2.2x\n",
                                       endpoint,
                                       cluster_id,
                                       attr_id,
                                       attr_len,
                                       is_server,
                                       manuf_code,
                                       data_type);
#endif

                        // make sure the attribute exists
                        if (attr_info != NULL)
                        {
                            // copy the nvram value to the current attribute value
                            memcpy(attr_info->data_p, &ds[ds_index], attr_len);

                            if (ZB_ZCL_IS_ATTR_SINGLETON(attr_info))
                            {
                                tr_conform_singleton(endpoint, cluster_id, is_server, attr_id, &ds[ds_index], manuf_code);
                            }
                        }
                        ds_index += attr_len;
                    }
                    break;
                }
            }

            if (ds_version < TR_NVRAM_STORAGE_CUR_VER)
            {
                // the stored version is older than the current version, schedule a write
                ZB_SCHEDULE_APP_ALARM_CANCEL(tr_nvram_save_attributes, ZB_ALARM_ANY_PARAM);
                ZB_SCHEDULE_APP_ALARM(tr_nvram_save_attributes, 0, TR_NVRAM_WRITE_DELAY);
            }
        }
        TR_FREE(ds);
    }
    else
    {
        tr_core_printf("%s: failed to malloc dataset buffer\n", __func__);
    }
}

// kick off the write process
void tr_nvram_save_attributes(zb_uint8_t param)
{
    ZB_SCHEDULE_APP_ALARM_CANCEL(tr_nvram_save_attributes, ZB_ALARM_ANY_PARAM);
    (void)zb_nvram_write_dataset(ZB_NVRAM_APP_DATA4);
}

// an attribute was just written, see if it is stored in nvram
void tr_check_for_attr_nvram_update(zb_uint8_t  ep,
                                    zb_uint16_t cluster_id,
                                    zb_uint8_t  cluster_role,
                                    zb_uint16_t attr_id,
                                    zb_uint16_t manuf_code)
{
    zb_uint8_t i;
#if TR_NVRAM_STORAGE_CUR_VER == TR_NVRAM_ATTR_STORAGE_VER_1
    tr_nvram_attr_storage_v1_t nvram_attr_storage[] = TR_NVRAM_ATTR_STORAGE_CONFIG;
#endif

    // see if this attribute lives in nvram
    for (i = 0 ; i < TR_NUM_NVRAM_ATTR ; i++)
    {
        if ((nvram_attr_storage[i].endpoint == ep) &&
            (nvram_attr_storage[i].cluster_id == cluster_id) &&
            (nvram_attr_storage[i].attr_id == attr_id) &&
            (nvram_attr_storage[i].cluster_role == cluster_role) &&
            (nvram_attr_storage[i].manuf_code == manuf_code))
        {
            // we need to save attributes to nvram, schedule it for 2 seconds from now
            ZB_SCHEDULE_APP_ALARM_CANCEL(tr_nvram_save_attributes, ZB_ALARM_ANY_PARAM);
            ZB_SCHEDULE_APP_ALARM(tr_nvram_save_attributes, 0, TR_NVRAM_WRITE_DELAY);
            break;
        }
    }
}

// an attribute was just written, see if it is stored in nvram and if so, save it NOW
void tr_check_for_attr_nvram_update_and_force_save(zb_uint8_t  ep,
                                                   zb_uint16_t cluster_id,
                                                   zb_uint8_t  cluster_role,
                                                   zb_uint16_t attr_id,
                                                   zb_uint16_t manuf_code)
{
    zb_uint8_t i;
#if TR_NVRAM_STORAGE_CUR_VER == TR_NVRAM_ATTR_STORAGE_VER_1
    tr_nvram_attr_storage_v1_t nvram_attr_storage[] = TR_NVRAM_ATTR_STORAGE_CONFIG;
#endif

    // see if this attribute lives in nvram
    for (i = 0 ; i < TR_NUM_NVRAM_ATTR ; i++)
    {
        if ((nvram_attr_storage[i].endpoint == ep) &&
            (nvram_attr_storage[i].cluster_id == cluster_id) &&
            (nvram_attr_storage[i].attr_id == attr_id) &&
            (nvram_attr_storage[i].cluster_role == cluster_role) &&
            (nvram_attr_storage[i].manuf_code == manuf_code))
        {
            // we need to save attributes to nvram, schedule it for 2 seconds from now
            ZB_SCHEDULE_APP_ALARM_CANCEL(tr_nvram_save_attributes, ZB_ALARM_ANY_PARAM);
            ZB_SCHEDULE_APP_ALARM(tr_nvram_save_attributes, 0, 0);
            break;
        }
    }
}
