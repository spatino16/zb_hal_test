/// ****************************************************************************
/// @file tr_cli_argument_parser.c
///
/// @brief Functions used for parsing cli command arguments
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include <unistd.h>
#include <stdlib.h>
#include "tr_cli_argument_parser.h"

// print cli command return status in a nice way
void tr_print_cli_cmd_status(char     *cmd_string,
                             zb_ret_t status)
{
    if (status == 0)
    {
        // if success, just print a 0x0
        tr_core_printf("%s status: 0x%x\n", cmd_string, status);
    }
    else
    {
        // if error, print the category and the code
        tr_core_printf("%s status: category %d, code %d\n",
                       cmd_string,
                       ERROR_GET_CATEGORY(status),
                       ERROR_GET_CODE(status));
    }
}

zb_uint8_t tr_cli_parse_standard_args(tr_standard_args_s *parsed_args_ptr,
                                      zb_int_t           argc,
                                      zb_char_t          **argv)
{
    zb_int_t opt;

    optind = 1;     // set the option indicator to the beginning
    opterr = 0;     // disable error printing for unkown options

    if (parsed_args_ptr == NULL)
    {
        return ZB_FALSE;
    }

    memset(parsed_args_ptr->value, 0, sizeof(parsed_args_ptr->value));
    parsed_args_ptr->num_cluster_ids = 0;
    parsed_args_ptr->num_attr_ids    = 0;
    parsed_args_ptr->value_entered   = 0;
    parsed_args_ptr->data_type       = 0xff;
    parsed_args_ptr->mfg_id          = 0xffff;
    parsed_args_ptr->aps_encryption  = 0;
    parsed_args_ptr->cmd_dir         = 0;

    while ((opt = getopt(argc, argv, ":c:a:t:v:m:d:e")) != -1)
    {
        switch (opt)
        {
            case 'c':
                // add a cluster id to the array
                parsed_args_ptr->num_cluster_ids++;
                parsed_args_ptr->cluster_ids[parsed_args_ptr->num_cluster_ids - 1] = tr_dec_or_hex_string_to_int(optarg);

                if (parsed_args_ptr->num_cluster_ids > sizeof(parsed_args_ptr->cluster_ids))
                {
                    parsed_args_ptr->num_cluster_ids = sizeof(parsed_args_ptr->cluster_ids);
                }
                break;

            case 'a':
                // add an attribute id to the array
                parsed_args_ptr->num_attr_ids++;
                parsed_args_ptr->attr_ids[parsed_args_ptr->num_attr_ids - 1] = tr_dec_or_hex_string_to_int(optarg);

                if (parsed_args_ptr->num_attr_ids > sizeof(parsed_args_ptr->attr_ids))
                {
                    parsed_args_ptr->num_attr_ids = sizeof(parsed_args_ptr->attr_ids);
                }
                break;

            case 't':
                // set the data type
                parsed_args_ptr->data_type = tr_dec_or_hex_string_to_int(optarg);
                break;

            case 'v':
                // copy the value string for the moment
                strncpy(parsed_args_ptr->value, optarg, sizeof(parsed_args_ptr->value) - 1);
                parsed_args_ptr->value_entered = 1;
                break;

            case 'm':
                // set the manufacturer id
                parsed_args_ptr->mfg_id = tr_dec_or_hex_string_to_int(optarg);
                break;

            case 'd':
                // set the command direction
                parsed_args_ptr->cmd_dir = atoi(optarg);
                break;

            case 'e':
                // set the aps encryption flag
                parsed_args_ptr->aps_encryption = 1;
                break;
        }
    }
    optind = 1;
    return ZB_TRUE;
}

void tr_print_cli_arg_struct(tr_standard_args_s *args_struct_ptr)
{
    zb_int_t i;

    tr_core_printf("Arg struct:\n");
    tr_core_printf("   Cluster(s):\n");

    for (i = 0 ; i < args_struct_ptr->num_cluster_ids ; i++)
    {
        tr_core_printf("      0x%4.4x\n", args_struct_ptr->cluster_ids[i]);
    }
    tr_core_printf("   Attribute(s):\n");

    for (i = 0 ; i < args_struct_ptr->num_attr_ids ; i++)
    {
        tr_core_printf("      0x%4.4x\n", args_struct_ptr->attr_ids[i]);
    }
    tr_core_printf("   Data type: 0x%2.2x\n", args_struct_ptr->data_type);
    tr_core_printf("   Value:     0x");

    for (i = 0 ; i < sizeof(args_struct_ptr->value) - 1 ; i++)
    {
        tr_core_printf("%2.2x", args_struct_ptr->value[i]);
    }
    tr_core_printf("\n");

    tr_core_printf("   Mfg ID:    0x%4.4x\n", args_struct_ptr->mfg_id);
    tr_core_printf("   Direction: %d\n", args_struct_ptr->cmd_dir);
    tr_core_printf("   APS enc:   %d\n", args_struct_ptr->aps_encryption);
}

// print an attribute value based on the descriptor passed in and the external data pointer
void tr_print_attr_value(zb_zcl_attr_t *attr_desc,
                         zb_uint8_t    *ext_attr_data_p)
{
    zb_int8_t   byte_index;
    zb_uint8_t  byte_value;
    zb_uint8_t  *data_ptr;
    zb_uint16_t attr_size = 0U;

    if (attr_desc->type != TR_ZCL_NO_DATA_ATTR_TYPE)
    {
        // setup the data pointer to the normal location
        data_ptr = attr_desc->data_p;

        // if it is handled externally, change the data pointer
        if (ext_attr_data_p != NULL)
        {
            data_ptr = ext_attr_data_p;
        }

        attr_size = zb_zcl_get_attribute_size(attr_desc->type, data_ptr);

        switch (attr_desc->type)
        {
            case TR_ZCL_OCTET_STRING_ATTR_TYPE:
            case TR_ZCL_CHAR_STRING_ATTR_TYPE:
                for (byte_index = 0 ; byte_index < attr_size ; byte_index++)
                {
                    byte_value = ((zb_uint8_t*)data_ptr)[byte_index];
                    tr_core_printf("%.2x", byte_value);
                }

                if (attr_desc->type == TR_ZCL_CHAR_STRING_ATTR_TYPE)
                {
                    tr_core_printf(" \"");

                    for (byte_index = 1 ; byte_index < attr_size ; byte_index++)
                    {
                        byte_value = ((zb_uint8_t*)data_ptr)[byte_index];
                        tr_core_printf("%c", byte_value);
                    }
                    tr_core_printf("\"");
                }
                break;

            default:
                for (byte_index = attr_size - 1 ; byte_index >= 0 ; byte_index--)
                {
                    byte_value = ((zb_uint8_t*)data_ptr)[byte_index];
                    tr_core_printf("%.2x", byte_value);
                }
                break;
        }
    }
    ZVUNUSED(byte_value);
}

// print eui64s in a nice way
void tr_print_eui64(zb_ieee_addr_t eui64)
{
    zb_uint8_t i;

    for (i = 1 ; i <= sizeof(zb_ieee_addr_t) ; i++)
    {
        tr_core_printf("%2.2x", eui64[sizeof(zb_ieee_addr_t) - i]);
    }
}

void tr_print_tx_buffer(zb_bufid_t buf,
                        zb_uint8_t *ptr)
{
    zb_uint8_t *data       = zb_buf_begin(buf);
    zb_uint8_t tx_buf[255] = { 0 };
    ZB_ZCL_PACKET_GET_DATA_N(tx_buf,
                             data,
                             ZB_ZCL_GET_BYTES_WRITTEN(buf, ptr));

    tr_core_printf("TX BUF: ");

    for (zb_uint8_t i = 0 ; i < ZB_ZCL_GET_BYTES_WRITTEN(buf, ptr) ; i++)
    {
        tr_core_printf("%02X ", tx_buf[i]);
    }
    tr_core_printf("\n");
}
