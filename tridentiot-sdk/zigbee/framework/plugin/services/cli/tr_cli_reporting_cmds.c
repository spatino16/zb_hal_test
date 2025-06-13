/// ****************************************************************************
/// @file tr_cli_reporting_cmds.c
///
/// @brief CLI reporting commands
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_af.h"
#include "tr_cli_argument_parser.h"
#include "tr_cli_zcl_cmds.h"

extern zb_int_t cli_cmd_print_reporting(zb_int_t  argc,
                                        zb_char_t *argv[]);
extern void tr_zcl_endpoint_config_reporting_init(void);

zb_int_t cli_rep_print_command(zb_int_t  argc,
                               zb_char_t *argv[])
{
    cli_cmd_print_reporting(argc, argv);
    return 0;
}

zb_int_t cli_rep_clear_command(zb_int_t  argc,
                               zb_char_t *argv[])
{
    zb_zcl_init_reporting_info();
    return 0;
}

zb_int_t cli_rep_delete_command(zb_int_t  argc,
                                zb_char_t *argv[])
{
    zb_uint8_t ret_val = ZB_TRUE;
    zb_uint8_t ep      = 1;
    zb_uint8_t index;
    zb_uint8_t i = 0;
    zb_char_t  *option_argument;

    // get the endpoint
    if (tr_cli_get_option(argc, argv, "e:", &option_argument))
    {
        ep = (zb_uint8_t)tr_dec_or_hex_string_to_int(option_argument);
    }

    // get the report table index
    if (tr_cli_get_option(argc, argv, "i:", &option_argument))
    {
        index = (zb_uint8_t)tr_dec_or_hex_string_to_int(option_argument);
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
        // figure out endpoint index from the given endpoint
        while (i < ZCL_CTX().device_ctx->ep_count)
        {
            if (ep == ZCL_CTX().device_ctx->ep_desc_list[i]->ep_id)
            {
                // we found the endpoint, is the index in range?
                if (index < ZCL_CTX().device_ctx->ep_desc_list[i]->rep_info_count)
                {
                    // clear the table entry
                    ZB_BZERO(&(ZCL_CTX().device_ctx->ep_desc_list[i]->reporting_info[index]),
                             sizeof(zb_zcl_reporting_info_t));
#ifdef ZB_USE_NVRAM

                    if (ZB_NVRAM().inited)
                    {
                        /* If we fail, trace is given and assertion is triggered */
                        (void)zb_nvram_write_dataset(ZB_NVRAM_ZCL_REPORTING_DATA);
                    }
#endif
                }
                else
                {
                    tr_core_printf("Error: table index %d out of range. Should be 0-%d\n",
                                   index,
                                   ZCL_CTX().device_ctx->ep_desc_list[i]->rep_info_count - 1);
                }
                break;
            }
            i++;
        }

        if (i == ZCL_CTX().device_ctx->ep_count)
        {
            tr_core_printf("Error: endpoint %d not found\n", ep);
        }
    }
    else
    {
        tr_core_printf("usage: delete -i table_index [-e ep (default 1)]\n");
    }
    return 0;
}

zb_int_t cli_rep_add_command(zb_int_t  argc,
                             zb_char_t *argv[])
{
    zb_uint8_t ret_val = ZB_TRUE;
    zb_ret_t   status;
    zb_bool_t  override = ZB_FALSE;
    zb_char_t  *option_argument;

    zb_zcl_reporting_info_t rep_info;

    // setup the report defaults
    rep_info.cluster_role   = TR_ZCL_CLUSTER_SERVER_ROLE;
    rep_info.direction      = 0;
    rep_info.ep             = 1;
    rep_info.manuf_code     = ZB_ZCL_NON_MANUFACTURER_SPECIFIC;
    rep_info.dst.profile_id = ZB_AF_HA_PROFILE_ID;

    // get the endpoint
    if (tr_cli_get_option(argc, argv, "e:", &option_argument))
    {
        rep_info.ep = (zb_uint8_t)tr_dec_or_hex_string_to_int(option_argument);
    }

    // get the cluster id
    if (tr_cli_get_option(argc, argv, "c:", &option_argument))
    {
        rep_info.cluster_id = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // get the attribute id
    if (tr_cli_get_option(argc, argv, "a:", &option_argument))
    {
        rep_info.attr_id = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // get the min reporting interval
    if (tr_cli_get_option(argc, argv, "n:", &option_argument))
    {
        rep_info.u.send_info.def_min_interval = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // get the max reporting interval
    if (tr_cli_get_option(argc, argv, "x:", &option_argument))
    {
        rep_info.u.send_info.def_max_interval = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // get the reportable change value
    if (tr_cli_get_option(argc, argv, "d:", &option_argument))
    {
        rep_info.u.send_info.delta.u32 = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
    }
    else
    {
        ret_val = ZB_FALSE;
    }

    // is it for the server or client?
    if (tr_cli_get_option(argc, argv, "s:", &option_argument))
    {
        rep_info.cluster_role = (zb_uint8_t)tr_dec_or_hex_string_to_int(option_argument);
    }

    // get the manufacturer id
    if (tr_cli_get_option(argc, argv, "m:", &option_argument))
    {
        rep_info.manuf_code = (zb_uint16_t)tr_dec_or_hex_string_to_int(option_argument);
    }

    // get the override option
    if (tr_cli_get_option(argc, argv, "o", &option_argument))
    {
        override = ZB_TRUE;
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        // update the reportable flag for the attribute
        zb_zcl_attr_t *attr_desc = zb_zcl_get_attr_desc_a(
            rep_info.ep,
            rep_info.cluster_id,
            rep_info.cluster_role,
            rep_info.attr_id);

        if (attr_desc != NULL)
        {
            attr_desc->access |= ZB_ZCL_ATTR_ACCESS_REPORTING;
            status             = zb_zcl_put_reporting_info(&rep_info, override);
            tr_core_printf("report add status = 0x%x\n", status);
        }
        else
        {
            tr_core_printf("report add: attribute does not exist\n");
        }
    }
    else
    {
        tr_core_printf(
            "usage: add -c cluster_id -a attr_id -d delta_value -n min_time -x max_time [-o override (default=false)] [-e ep (default=1)] [-s 1 or 0 (default=1)] [-m mfg_id (default=not mfg specific)]\n");
    }

    ZVUNUSED(status);

    return 0;
}

zb_int_t cli_rep_reset_command(zb_int_t  argc,
                               zb_char_t *argv[])
{
    tr_zcl_endpoint_config_reporting_init();
    return 0;
}

TR_CLI_COMMAND_TABLE(plugin_reporting_commands) =
{
    { "print",  cli_rep_print_command,  "print the reporting table"               },
    { "clear",  cli_rep_clear_command,  "clear the reporting table"               },
    { "delete", cli_rep_delete_command, "delete a table entry"                    },
    { "add",    cli_rep_add_command,    "add or change a table entry"             },
    { "reset",  cli_rep_reset_command,  "reset reporting to factory defaults"     },
    TR_CLI_COMMAND_TABLE_END
};
