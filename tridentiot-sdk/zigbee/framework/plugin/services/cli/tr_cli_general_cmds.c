/// ****************************************************************************
/// @file tr_cli_general_cmds.c
///
/// @brief CLI general commands
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>
#include "tr_af.h"
#include "tr_cli_argument_parser.h"
#include "tr_cli_zcl_cmds.h"

void zb_mac_set_tx_power(zb_int8_t new_power);

extern const tr_command_s zcl_global_commands[];
extern const tr_command_s network_commands[];
extern const tr_command_s print_commands[];
extern const tr_command_s plugin_reporting_commands[];
extern const tr_command_s plugin_find_and_bind_commands[];
extern const tr_command_s plugin_binding_table_commands[];
extern const tr_command_s zdo_commands[];
extern const tr_command_s debug_print_commands[];

extern TR_CLI_COMMAND_TABLE(zcl_basic_c_cluster_commands);
extern TR_CLI_COMMAND_TABLE(zcl_power_configuration_s_plugin_commands);
extern TR_CLI_COMMAND_TABLE(zcl_identify_c_cluster_commands);
extern TR_CLI_COMMAND_TABLE(zcl_groups_c_cluster_commands);
extern TR_CLI_COMMAND_TABLE(zcl_on_off_c_cluster_commands);
#ifdef TR_ALARMS_CLIENT_CLI_ENABLE
extern TR_CLI_COMMAND_TABLE(zcl_alarms_c_cluster_commands);
#endif
#ifdef TR_OVER_THE_AIR_BOOTLOADING_CLIENT_CLI_ENABLE
extern TR_CLI_COMMAND_TABLE(zcl_ota_upgrade_c_cluster_commands);
#endif
#ifdef TR_MFG_LIB_CLI_ENABLE
extern TR_CLI_COMMAND_TABLE(mfg_lib_commands);
#endif
#ifdef TR_REMOTE_CLI_CLIENT_CLI_ENABLE
extern TR_CLI_COMMAND_TABLE(zcl_remote_cli_c_cluster_commands);
#endif
#ifdef CUSTOM_CLI_COMMANDS_ENABLE
extern TR_CLI_COMMAND_TABLE(custom_cli_commands);
#endif

// convert a user input string to an array of bytes ready for writing to an attribute
void tr_convert_attr_value(zb_char_t  *input,
                           zb_uint8_t data_type,
                           zb_uint8_t *output)
{
    switch (data_type)
    {
        case TR_ZCL_OCTET_STRING_ATTR_TYPE:
        case TR_ZCL_CHAR_STRING_ATTR_TYPE:
            output[0] = strlen(input);
            memcpy(&output[1], input, output[0]);
            break;

        case TR_ZCL_LONG_OCTET_STRING_ATTR_TYPE:
        case TR_ZCL_LONG_CHAR_STRING_ATTR_TYPE:
            // for long octet string and long char string put a 2 byte length at the beginning
            tr_core_printf("\nLong octet (0x43) and long char (0x44) string types not supported\n");
            break;

        default:
        {
            zb_uint64_t value;
            zb_uint8_t  attr_size = zb_zcl_get_attribute_size(data_type, (zb_uint8_t*)input);
            value                 = tr_dec_or_hex_string_to_int(input);
            memcpy(output, &value, attr_size);
            break;
        }
    }
}

// callback for the send command
void cli_msg_send_cb(zb_uint8_t param)
{
    zb_zcl_command_send_status_t *cmd_send_status =
        ZB_BUF_GET_PARAM(param, zb_zcl_command_send_status_t);
    tr_print_cli_cmd_status("Send", cmd_send_status->status);

    zb_buf_free(param);
    g_cli_zcl_cmd_creation_s.buffer  = 0;
    g_cli_zcl_cmd_creation_s.cmd_ptr = NULL;
    g_cli_zcl_cmd_creation_s.cb      = NULL;
}

zb_int_t cli_cmd_info(zb_int_t  argc,
                      zb_char_t *argv[])
{
    zb_int_t              i;
    zb_int_t              power;
    zb_ieee_addr_t        ieee_addr;
    zb_ext_pan_id_t       ext_pan_id;
    zb_af_endpoint_desc_t **ep_desc_list;

    ZB_TRANSCEIVER_GET_TX_POWER(&power);

    // TODO: get version from Trident SDK meta data file
    tr_core_printf("TSDK Ver:     2025.05.00\n");

    // device EUI64
    zb_get_long_address(ieee_addr);
    tr_core_printf("EUI64:        0x");

    for (i = 0 ; i < 8 ; i++)
    {
        tr_core_printf("%2.2x", ieee_addr[7 - i]);
    }
    tr_core_printf("\n");

    tr_core_printf("Chan:         %d\n", zb_get_current_channel());
    tr_core_printf("Pwr:          %d\n", power);
    tr_core_printf("PAN ID:       0x%4.4x\n", zb_get_pan_id());

    // extended PAN ID
    zb_get_extended_pan_id(ext_pan_id);
    tr_core_printf("Ext PAN ID:   0x");

    for (i = 0 ; i < 8 ; i++)
    {
        tr_core_printf("%2.2x", ext_pan_id[7 - i]);
    }
    tr_core_printf("\n");

    tr_core_printf("Node ID:      0x%4.4x\n", zb_get_short_address());
    tr_core_printf("Node type:    ");

    switch (zb_get_network_role())
    {
        case ZB_NWK_DEVICE_TYPE_COORDINATOR:
            tr_core_printf("Coordinator\n");
            break;

        case ZB_NWK_DEVICE_TYPE_ROUTER:
            tr_core_printf("Router\n");
            break;

        case ZB_NWK_DEVICE_TYPE_ED:
            if (zb_get_rx_on_when_idle())
            {
                tr_core_printf("Non-sleepy ");
            }
            else
            {
                tr_core_printf("Sleepy ");
            }
            tr_core_printf("end device\n");
            break;
    }

    tr_core_printf("Network state: %d\n", tr_get_connection_state());
    tr_core_printf("Buffer usage: %d in, %d out, total %d of %d\n",
                   ZG->bpool.bufs_allocated[0],
                   ZG->bpool.bufs_allocated[1],
                   (ZG->bpool.bufs_allocated[0] + ZG->bpool.bufs_allocated[1]),
                   ZB_IOBUF_POOL_SIZE);
    tr_core_printf("\n");
    tr_core_printf("Endpoint count %d\n", ZCL_CTX().device_ctx->ep_count);

    // walk the endpoints
    for (i = 0 ; i < ZCL_CTX().device_ctx->ep_count ; i++)
    {
        int x;

        ep_desc_list = ZCL_CTX().device_ctx->ep_desc_list;
        tr_core_printf("   ep %d: profile 0x%4.4x, dev id 0x%4.4x\n",
                       ep_desc_list[i]->ep_id,
                       ep_desc_list[i]->profile_id,
                       ep_desc_list[i]->simple_desc->app_device_id);

        // display cluster info by endpoint
        for (x = 0 ; x < ep_desc_list[i]->cluster_count ; x++)
        {
            tr_core_printf("      cluster 0x%4.4x ", ep_desc_list[i]->cluster_desc_list[x].cluster_id);

            if (ep_desc_list[i]->cluster_desc_list[x].role_mask == TR_ZCL_CLUSTER_SERVER_ROLE)
            {
                tr_core_printf("out (server)\n");
            }
            else
            {
                tr_core_printf("in  (client)\n");
            }
        }
        tr_core_printf("\n");
    }
    return 0;
}

// reset the device
zb_int_t cli_cmd_reset(zb_int_t  argc,
                       zb_char_t *argv[])
{
    zb_uint8_t ret_val = ZB_TRUE;
    zb_char_t  *option_argument;

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        // reset the chip
        zb_reset(0);
    }
    else
    {
        tr_core_printf("usage: reset [-h print help]\n");
    }
    return 0;
}

// print the Trident SDK version
zb_int_t cli_cmd_version(zb_int_t  argc,
                         zb_char_t *argv[])
{
    // TODO: get version from Trident SDK meta data file
    tr_core_printf("TSDK Ver: 2025.05.00\n");
    return 0;
}

// send a zcl command that has already been built
zb_int_t cli_cmd_send(zb_int_t  argc,
                      zb_char_t *argv[])
{
    zb_uint8_t ret_val     = ZB_TRUE;
    zb_uint8_t dest_ep     = 1;
    zb_uint8_t src_ep      = 1;
    zb_bool_t  aps_encrypt = ZB_FALSE;
    zb_bool_t  no_aps_ack  = ZB_FALSE;
    zb_addr_u  dest_addr;
    zb_char_t  *option_argument;

    // get the destination short address
    dest_addr.addr_short = 0xFFFF;  // set the destination short address to cause an error

    if (tr_cli_get_option(argc, argv, "a:", &option_argument))
    {
        dest_addr.addr_short = tr_dec_or_hex_string_to_int(option_argument);
    }

    // get the destination endpoint
    if (tr_cli_get_option(argc, argv, "d:", &option_argument))
    {
        dest_ep = tr_dec_or_hex_string_to_int(option_argument);
    }

    // get the source endpoint
    if (tr_cli_get_option(argc, argv, "s:", &option_argument))
    {
        src_ep = tr_dec_or_hex_string_to_int(option_argument);
    }

    // is this to be aps encrypted?
    if (tr_cli_get_option(argc, argv, "e", &option_argument))
    {
        aps_encrypt = ZB_TRUE;
    }

    // don't request and APS ACK?
    if (tr_cli_get_option(argc, argv, "n", &option_argument))
    {
        no_aps_ack = ZB_TRUE;
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if ((dest_addr.addr_short != 0xFFFF) && ret_val)
    {
        // make sure a command is setup
        if (g_cli_zcl_cmd_creation_s.cmd_ptr == NULL)
        {
            tr_core_printf("No command ready to send\n");
            return -1;
        }

        if (g_cli_zcl_cmd_creation_s.cb == NULL)
        {
            // set the default callback for the send function
            g_cli_zcl_cmd_creation_s.cb = cli_msg_send_cb;
        }
        zb_zcl_finish_and_send_packet_new(g_cli_zcl_cmd_creation_s.buffer,
                                          g_cli_zcl_cmd_creation_s.cmd_ptr,
                                          &(dest_addr),
                                          ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                          dest_ep,
                                          src_ep,
                                          g_cli_zcl_cmd_creation_s.prof_id,
                                          g_cli_zcl_cmd_creation_s.cluster_id,
                                          g_cli_zcl_cmd_creation_s.cb,
                                          aps_encrypt,
                                          no_aps_ack,
                                          0);
    }
    else
    {
        tr_core_printf(
            "usage: send -a short addr [-d dest ep <default 1>] [-s src ep <default 1>] [-e aps encrypt <default don't aps encrypt>] [-n no aps ack <default aps ack>]\n");
    }
    return 0;
}

// send a zcl command that has already been built to a binding
zb_int_t cli_cmd_bsend(zb_int_t  argc,
                       zb_char_t *argv[])
{
    zb_uint8_t ret_val = ZB_TRUE;
    zb_uint8_t index;
    zb_addr_u  dest_addr;
    zb_bool_t  aps_encrypt = ZB_FALSE;
    zb_char_t  *option_argument;

    // get the binding table index
    if (tr_cli_get_option(argc, argv, "b:", &option_argument))
    {
        index = tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // is this to be aps encrypted?
    if (tr_cli_get_option(argc, argv, "e", &option_argument))
    {
        aps_encrypt = ZB_TRUE;
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        // make sure a command is setup
        if (g_cli_zcl_cmd_creation_s.cmd_ptr == NULL)
        {
            tr_core_printf("No command ready to send\n");
            return -1;
        }

        if (g_cli_zcl_cmd_creation_s.cb == NULL)
        {
            // set the default callback for the send function
            g_cli_zcl_cmd_creation_s.cb = cli_msg_send_cb;
        }

        zb_address_ieee_by_ref(dest_addr.addr_long, ZG->aps.binding.dst_table[index].u.long_addr.dst_addr);
        zb_zcl_finish_and_send_packet_new(g_cli_zcl_cmd_creation_s.buffer,
                                          g_cli_zcl_cmd_creation_s.cmd_ptr,
                                          &(dest_addr),
                                          ZB_APS_ADDR_MODE_64_ENDP_PRESENT,
                                          ZG->aps.binding.dst_table[index].u.long_addr.dst_end,
                                          ZG->aps.binding.src_table[ZG->aps.binding.dst_table[index].src_table_index].src_end,
                                          g_cli_zcl_cmd_creation_s.prof_id,
                                          g_cli_zcl_cmd_creation_s.cluster_id,
                                          g_cli_zcl_cmd_creation_s.cb,
                                          aps_encrypt,
                                          ZB_FALSE,
                                          0);

    }
    else
    {
        tr_core_printf("usage: bsend -b binding table index [-e aps encrypt <default don't aps encrypt>]\n");
    }
    return 0;
}

// send a zcl command that has already been built to a group
zb_int_t cli_cmd_gsend(zb_int_t  argc,
                       zb_char_t *argv[])
{
    tr_core_printf("gsend - send a command to a group. Coming soon to an SDK near you!\n");
    return 0;
}

// read a local attribute
zb_int_t cli_cmd_read(zb_int_t  argc,
                      zb_char_t *argv[])
{
    zb_uint8_t  ret_val          = ZB_TRUE;
    zb_uint8_t  ep               = 1;
    zb_uint16_t cluster_id       = 0xFFFF;
    zb_uint16_t attr_id          = 0xFFFF;
    zb_uint16_t mfg_id           = ZB_ZCL_NON_MANUFACTURER_SPECIFIC;
    zb_uint8_t  cluster_role     = ZB_ZCL_CLUSTER_SERVER_ROLE;
    zb_uint8_t  *ext_attr_data_p = NULL;
    zb_char_t   *option_argument;

    // get the cluster id
    if (tr_cli_get_option(argc, argv, "c:", &option_argument))
    {
        cluster_id = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // get the attribute id
    if (tr_cli_get_option(argc, argv, "a:", &option_argument))
    {
        attr_id = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // get the endpoint
    if (tr_cli_get_option(argc, argv, "e:", &option_argument))
    {
        ep = (zb_uint8_t)tr_dec_or_hex_string_to_int(option_argument);
    }

    // is it for the server or client?
    if (tr_cli_get_option(argc, argv, "s:", &option_argument))
    {
        if ((zb_uint8_t)tr_dec_or_hex_string_to_int(option_argument) == 0)
        {
            cluster_role = TR_ZCL_CLUSTER_CLIENT_ROLE;
        }
    }

    // get the manufacturer id
    if (tr_cli_get_option(argc, argv, "m:", &option_argument))
    {
        mfg_id = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        // do the read
        zb_zcl_attr_t *attr_info;

        attr_info =  zb_zcl_get_attr_desc_manuf_a(
            ep,
            cluster_id,
            cluster_role,
            attr_id,
            mfg_id);

        if (attr_info != NULL)
        {
            ext_attr_data_p = tr_zcl_external_attribute_read_cb(ep, cluster_id, cluster_role, attr_id, mfg_id);
            tr_core_printf("Ep: %d, Cl: 0x%4.4x Attr: 0x%4.4x, Srv %d, Value: 0x", ep, cluster_id, attr_id, cluster_role);
            tr_print_attr_value(attr_info, ext_attr_data_p);
            tr_core_printf("\n");
        }
        else
        {
            tr_core_printf("Ep: %d, Cl: 0x%4.4x Attr: 0x%4.4x, Srv %d, Value: not found\n", ep, cluster_id, attr_id, cluster_role);
        }
    }
    else
    {
        tr_core_printf(
            "usage: read -c cluster id -a attr id [-e ep <default 1>] [-s 1 for server <default> or 0 for client] [-m mfg id <default not mfg specific>]\n");
    }
    return 0;
}

// write a local attribute
zb_int_t cli_cmd_write(zb_int_t  argc,
                       zb_char_t *argv[])
{
    zb_uint8_t      ret_val      = ZB_TRUE;
    zb_uint8_t      ep           = 1;
    zb_uint8_t      cluster_role = ZB_ZCL_CLUSTER_SERVER_ROLE;
    zb_uint16_t     cluster_id   = 0xFFFF;
    zb_uint16_t     attr_id      = 0xFFFF;
    zb_uint16_t     mfg_id       = ZB_ZCL_NON_MANUFACTURER_SPECIFIC;
    zb_char_t       *option_argument;
    zb_bool_t       check_access = ZB_TRUE;
    zb_zcl_status_t status;

    // get the cluster id
    if (tr_cli_get_option(argc, argv, "c:", &option_argument))
    {
        cluster_id = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // get the attribute id
    if (tr_cli_get_option(argc, argv, "a:", &option_argument))
    {
        attr_id = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // get the endpoint
    if (tr_cli_get_option(argc, argv, "e:", &option_argument))
    {
        ep = (zb_uint8_t)tr_dec_or_hex_string_to_int(option_argument);
    }

    // is it for the server or client?
    if (tr_cli_get_option(argc, argv, "s:", &option_argument))
    {
        if ((zb_uint8_t)tr_dec_or_hex_string_to_int(option_argument) == 0)
        {
            cluster_role = TR_ZCL_CLUSTER_CLIENT_ROLE;
        }
    }

    // are we overriding the read-only limitation?
    if (tr_cli_get_option(argc, argv, "o", &option_argument))
    {
        check_access = ZB_FALSE;
    }

    // get the manufacturer id
    if (tr_cli_get_option(argc, argv, "m:", &option_argument))
    {
        mfg_id = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
    }

    // get the value
    // NOTE: DO THIS LAST SO THAT option_argument still points to the value string
    if (!tr_cli_get_option(argc, argv, "v:", &option_argument))
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
        zb_uint8_t    value[33];
        zb_zcl_attr_t *attr_info;

        attr_info =  zb_zcl_get_attr_desc_manuf_a(
            ep,
            cluster_id,
            cluster_role,
            attr_id,
            mfg_id);

        if (attr_info != NULL)
        {
            // convert the value string entered into the proper type
            tr_convert_attr_value(option_argument, attr_info->type, value);
            status = zb_zcl_set_attr_val_manuf(ep, cluster_id, cluster_role, attr_id, mfg_id, value, check_access);
            tr_core_printf("Status = 0x%x\n", status);
        }
        else
        {
            tr_core_printf("Ep: %d, Cl: 0x%4.4x Attr: 0x%4.4x, Srv %d attr not found\n", ep, cluster_id, attr_id, cluster_role);
        }
    }
    else
    {
        tr_core_printf(
            "usage: write -c cluster id -a attr id -v value [-e ep <default 1>] [-s 1 for server <default> or 0 for client] [-m mfg id <default not mfg specific>] [-o override RO]\n");
    }

    ZVUNUSED(status);

    return 0;
}

// set the tx power
zb_int_t cli_cmd_set_power(zb_int_t  argc,
                           zb_char_t *argv[])
{
    int       tx_power;
    int       ret_val = ZB_FALSE;
    zb_char_t *option_argument;

    // get the transmit power
    if (tr_cli_get_option(argc, argv, "p:", &option_argument))
    {
        tx_power = tr_dec_or_hex_string_to_int(option_argument);
        ret_val  = ZB_TRUE;
    }

    if (ret_val)
    {
        tr_core_printf("Setting TX power to %d\n", tx_power);
        zb_mac_set_tx_power((zb_int8_t)tx_power);
    }
    else
    {
        tr_core_printf("usage: tx_power set -p power in dB\n");
    }

    return 0;
}

// get the tx power
zb_int_t cli_cmd_get_power(zb_int_t  argc,
                           zb_char_t *argv[])
{
    tr_core_printf("TX Power: %d dB\n", tr_get_tx_power());
    return 0;
}

// need to do this in a scheduled callback because we cannot call tr_set_cca_threshold from our cli
void cli_set_cca_threshold(zb_uint8_t threshold)
{
    tr_set_cca_threshold(threshold);
}

zb_int_t cli_cmd_set_cca(zb_int_t  argc,
                         zb_char_t *argv[])
{
    zb_bool_t ret_val   = ZB_TRUE;
    zb_uint_t threshold = 72;
    zb_char_t *option_argument;

    if (tr_cli_get_option(argc, argv, "c:", &option_argument))
    {
        threshold = abs(tr_dec_or_hex_string_to_int(option_argument));

        if (threshold > TR_CCA_MAX_THRESHOLD_MAGNITUDE)
        {
            ret_val = ZB_FALSE;
        }
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        ZB_SCHEDULE_CALLBACK(cli_set_cca_threshold, (zb_uint8_t)threshold);
    }
    else
    {
        tr_core_printf("usage: cca set -c cca threshold (0-%d, treated as 0 to -%d dB)\n",
                       TR_CCA_MAX_THRESHOLD_MAGNITUDE,
                       TR_CCA_MAX_THRESHOLD_MAGNITUDE);
    }
    return 0;
}

zb_int_t cli_cmd_get_cca(zb_int_t  argc,
                         zb_char_t *argv[])
{
    tr_core_printf("CCA Threshold: -%d dB\n", tr_get_cca_threshold());
    return 0;
}

#ifdef TR_IS_SLEEPY_ZED

// force a sleepy device to stay awake
zb_int_t cli_cmd_stay_awake(zb_int_t  argc,
                            zb_char_t *argv[])
{
    zb_bool_t allow_sleep = ZB_FALSE;
    int       ret_val     = ZB_TRUE;
    zb_char_t *option_argument;

    // get the sleep option
    if (tr_cli_get_option(argc, argv, "s", &option_argument))
    {
        allow_sleep = ZB_TRUE;
        ret_val     = ZB_TRUE;
    }

    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        tr_allow_sleep(allow_sleep);
    }
    else
    {
        tr_core_printf("usage: stay_awake [-s allow sleep <default is stay awake>]\n");
    }

    return 0;
}

#endif /* ifdef TR_IS_SLEEPY_ZED */

// convert a {} enclosed array of hex bytes to an array
// the challenge is dealing with space delimted bytes, non-delimted bytes, and nibbles
// example: {12 34 aBC D EF0} yields 12 34 AB 0C 0D EF 00
zb_uint8_t tr_hex_array_string_to_int(const zb_char_t *string,
                                      zb_uint8_t      *data)
{
    zb_uint8_t x          = 0;
    zb_uint8_t i          = 0;
    zb_uint8_t shift_bits = 0;

    // Check if the input string is valid
    if (string == NULL || string[0] == '\0' || data == NULL)
    {
        return 0;
    }

    // string must start with a '{'
    if (string[0] != '{')
    {
        return 0;
    }

    // make sure there is a closing '}'
    while ((string[x] != '}') && (x <= 32))
    {
        x++;
    }

    if (x >= 32)
    {
        // there wasn't a closing '}'
        return 0;
    }

    // reset the character index to the beginning
    x = 0;

    // skip everything up to the first hex digit
    while (!isxdigit((zb_uint8_t)string[x]))
    {
        x++;
    }

    data[i] = 0;

    do
    {
        // terminate on a '}'
        if (string[x] == '}')
        {
            break;
        }

        data[i] = data[i] << shift_bits;

        // do we have a 0-9?
        if (isdigit((zb_uint8_t)string[x]))
        {
            // convert the ascii to nibble
            data[i] |= string[x] - '0';
        }
        else if (isxdigit((zb_uint8_t)string[x]))
        {
            // it is a-f, convert ascii to nibble
            data[i] |= toupper(string[x]) - 'A' + 0x0A;
        }

        // move to the next character
        x++;

        // we got a nibble, is it time to move to the next byte?
        if (shift_bits != 0)
        {
            // yes move to the next output byte and skip non-hex digits
            i++;
            data[i]    = 0;
            shift_bits = 0;

            // skip over any non-hex digits, but don't skip the terminating }
            while (!isxdigit((zb_uint8_t)string[x]) && (string[x] != '}'))
            {
                x++;
            }
        }
        else
        {
            // get ready for the lower nibble
            shift_bits = 4;
        }

        if (!isxdigit((zb_uint8_t)string[x]))
        {
            if (string[x] != '}')
            {
                // the character is not a hex digit, move to next byte and skip non-hex digits
                i++;
                data[i]    = 0;
                shift_bits = 0;

                // skip over any non-hex digits, but don't skip the terminating }
                while (!isxdigit((zb_uint8_t)string[x]) && (string[x] != '}'))
                {
                    x++;
                }
            }
        }
    }
    while (string[x] != '}');

    if (shift_bits != 0)
    {
        // we have one extra nibble, return it as a full byte
        return (i + 1);
    }
    return i;
}

// build the zcl raw command
zb_int_t cli_cmd_raw(zb_int_t  argc,
                     zb_char_t *argv[])
{
    zb_uint8_t  ret_val = ZB_TRUE;
    zb_uint8_t  dir     = ZB_ZCL_FRAME_DIRECTION_TO_SRV;
    zb_uint8_t  seq_num;
    zb_uint8_t  cmd_id       = 0;
    zb_uint8_t  general      = ZB_FALSE;
    zb_uint8_t  mfg_specific = ZB_ZCL_NOT_MANUFACTURER_SPECIFIC;
    zb_uint8_t  data[32];
    zb_uint8_t  data_bytes = 0;
    zb_uint16_t mfg_id;
    zb_char_t   *option_argument;

    // setup the defaults for sending
    g_cli_zcl_cmd_creation_s.prof_id = ZB_AF_HA_PROFILE_ID;
    g_cli_zcl_cmd_creation_s.cb      = NULL;

    // get the cluster id
    if (tr_cli_get_option(argc, argv, "c:", &option_argument))
    {
        g_cli_zcl_cmd_creation_s.cluster_id = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // get the command id
    if (tr_cli_get_option(argc, argv, "o:", &option_argument))
    {
        cmd_id = (zb_uint8_t)tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // is it a general command (or cluster specific)
    if (tr_cli_get_option(argc, argv, "g", &option_argument))
    {
        general = ZB_TRUE;
    }

    // get the profile id if it is provided
    if (tr_cli_get_option(argc, argv, "p:", &option_argument))
    {
        g_cli_zcl_cmd_creation_s.prof_id = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
    }

    // get the direction if it is provided
    if (tr_cli_get_option(argc, argv, "d:", &option_argument))
    {
        dir = (zb_uint8_t)tr_dec_or_hex_string_to_int(option_argument);
    }

    // get the sequence number if it is provided
    if (tr_cli_get_option(argc, argv, "s:", &option_argument))
    {
        seq_num = (zb_uint8_t)tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        seq_num = ZB_ZCL_GET_SEQ_NUM();
    }

    // get the manufacturer id if it is provided
    if (tr_cli_get_option(argc, argv, "m:", &option_argument))
    {
        mfg_id       = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
        mfg_specific = ZB_ZCL_MANUFACTURER_SPECIFIC;
    }

    // get additional bytes if they are provided
    if (tr_cli_get_option(argc, argv, "a:", &option_argument))
    {
        data_bytes = tr_hex_array_string_to_int(option_argument, data);

        if (data_bytes == 0)
        {
            tr_core_printf("-a malformed byte array\n");
            ret_val = ZB_FALSE;
        }
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        // get a buffer and build the command
        if (g_cli_zcl_cmd_creation_s.buffer == 0)
        {
            g_cli_zcl_cmd_creation_s.buffer = zb_buf_get_out();
        }

        g_cli_zcl_cmd_creation_s.cmd_ptr = ZB_ZCL_START_PACKET(g_cli_zcl_cmd_creation_s.buffer);

        if (general)
        {
            ZB_ZCL_CONSTRUCT_GENERAL_COMMAND_REQ_FRAME_CONTROL_A(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                                                 dir,
                                                                 mfg_specific,
                                                                 tr_global_default_response_policy);
        }
        else
        {
            ZB_ZCL_CONSTRUCT_SPECIFIC_COMMAND_REQ_FRAME_CONTROL_A(g_cli_zcl_cmd_creation_s.cmd_ptr,
                                                                  dir,
                                                                  mfg_specific,
                                                                  tr_global_default_response_policy);
        }

        if (mfg_specific == ZB_ZCL_MANUFACTURER_SPECIFIC)
        {
            // add in the mfg id
            ZB_ZCL_PACKET_PUT_DATA16_VAL(g_cli_zcl_cmd_creation_s.cmd_ptr, mfg_id);
        }

        ZB_ZCL_CONSTRUCT_COMMAND_HEADER(g_cli_zcl_cmd_creation_s.cmd_ptr, seq_num, cmd_id);

        // add in payload bytes here
        for (uint8_t x = 0 ; x < data_bytes ; x++)
        {
            ZB_ZCL_PACKET_PUT_DATA8(g_cli_zcl_cmd_creation_s.cmd_ptr, data[x]);
        }

        // print the command buffer
        tr_print_tx_buffer(g_cli_zcl_cmd_creation_s.buffer, g_cli_zcl_cmd_creation_s.cmd_ptr);
    }
    else
    {
        tr_core_printf(
            "usage: raw -c cluster id -o cmd id [-d dir <default 0 (c->s)>] [-a {additional hex bytes}] [-s seq num <default next>] [-m mfg id <default not mfg specific>] [-p profile id <default 0x104 (HA)>]\n");
    }
    return 0;
}

/**********************************************/

TR_CLI_COMMAND_TABLE(zcl_commands) =
{
    { "raw",      cli_cmd_raw,         "build a raw zcl command"                                                      },
    { "global",   TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(zcl_global_commands)                                  },
#ifdef TR_BASIC_CLIENT_CLI_ENABLE
    { "basic",    TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(zcl_basic_c_cluster_commands)                         },
#endif
#ifdef TR_IDENTIFY_CLIENT_CLI_ENABLE
    { "identify", TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(zcl_identify_c_cluster_commands)                      },
#endif
#ifdef TR_ALARMS_CLIENT_CLI_ENABLE
    { "alarms",   TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(zcl_alarms_c_cluster_commands)                        },
#endif
#ifdef TR_GROUPS_CLIENT_CLI_ENABLE
    { "groups",   TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(zcl_groups_c_cluster_commands)                        },
#endif
#ifdef TR_ON_OFF_CLIENT_CLI_ENABLE
    { "on-off",   TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(zcl_on_off_c_cluster_commands)                        },
#endif
#ifdef TR_REMOTE_CLI_CLIENT_CLI_ENABLE
    { "rcli",     TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(zcl_remote_cli_c_cluster_commands)                    },
#endif
    TR_CLI_COMMAND_TABLE_END
};

TR_CLI_COMMAND_TABLE(plugin_commands) =
{
    { "reporting",     TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(plugin_reporting_commands)                 },
    { "find-and-bind", TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(plugin_find_and_bind_commands)             },
    { "binding-table", TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(plugin_binding_table_commands)             },
#ifdef TR_POWER_CONFIGURATION_SERVER_PLUGIN_CLI_ENABLE
    { "power-config",  TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(zcl_power_configuration_s_plugin_commands) },
#endif
#ifdef TR_OVER_THE_AIR_BOOTLOADING_CLIENT_CLI_ENABLE
    { "ota-client",    TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(zcl_ota_upgrade_c_cluster_commands)        },
#endif
    TR_CLI_COMMAND_TABLE_END
};

TR_CLI_COMMAND_TABLE(tx_power_commands) =
{
    { "set", cli_cmd_set_power, "Set the TX power" },
    { "get", cli_cmd_get_power, "Get the TX power" },
    TR_CLI_COMMAND_TABLE_END
};

TR_CLI_COMMAND_TABLE(cca_commands) =
{
    { "set", cli_cmd_set_cca, "Set the CCA threshold" },
    { "get", cli_cmd_get_cca, "Get the CCA threshold" },
    TR_CLI_COMMAND_TABLE_END
};

TR_CLI_COMMAND_TABLE(mac_commands) =
{
    { "tx_power", TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(tx_power_commands) },
    { "cca",      TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(cca_commands)      },
    TR_CLI_COMMAND_TABLE_END
};

TR_CLI_COMMAND_TABLE(general_commands) =
{
    { "info",              cli_cmd_info,        "Print device information"                                            },
    { "reset",             cli_cmd_reset,       "Reset the device"                                                    },
    { "version",           cli_cmd_version,     "Display the stack version"                                           },
    { "send",              cli_cmd_send,        "Send a pre-built zcl command"                                        },
    { "bsend",             cli_cmd_bsend,       "Send a pre-built zcl command to a binding"                           },
    { "gsend",             cli_cmd_gsend,       "Send a pre-built zcl command to a group"                             },
    { "read",              cli_cmd_read,        "Read a local attribute"                                              },
    { "write",             cli_cmd_write,       "Write a local attribute"                                             },
    { "mac",               TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(mac_commands)                                },
#ifdef TR_IS_SLEEPY_ZED
    { "stay_awake",        cli_cmd_stay_awake,  "Force a sleepy device to stay awake"                                 },
#endif
    { "network",           TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(network_commands)                            },
    { "zcl",               TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(zcl_commands)                                },
    { "print",             TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(print_commands)                              },
    { "plugin",            TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(plugin_commands)                             },
    { "zdo",               TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(zdo_commands)                                },
    { "debug",             TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(debug_print_commands)                        },
#ifdef TR_MFG_LIB_CLI_ENABLE
    { "mfg_lib",           TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(mfg_lib_commands)                            },
#endif
#ifdef CUSTOM_CLI_COMMANDS_ENABLE
    { CUSTOM_CLI_TOPLEVEL, TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(custom_cli_commands)                         },
#endif

    TR_CLI_COMMAND_TABLE_END
};
