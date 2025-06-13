/// ****************************************************************************
/// @file tr_power_configuration_server_cli.c
///
/// @brief Contains CLI commands specific to the POWER CONFIGURATION server cluster
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_power_configuration_server.h"
#include "tr_cli_argument_parser.h"

zb_int_t cli_cmd_power_configuration_server_set_mains_voltage(zb_int_t  argc,
                                                              zb_char_t *argv[])
{
    zb_uint8_t      ep = 1;
    zb_uint16_t     voltage_100mv;
    zb_char_t       *option_argument;
    zb_uint8_t      ret_val = ZB_FALSE;
    zb_zcl_status_t status;

    // get the endpoint
    if (tr_cli_get_option(argc, argv, "e:", &option_argument))
    {
        ep = tr_dec_or_hex_string_to_int(option_argument);
    }

    // get the voltage
    if (tr_cli_get_option(argc, argv, "v:", &option_argument))
    {
        voltage_100mv = tr_dec_or_hex_string_to_int(option_argument);
        ret_val       = ZB_TRUE;
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        status = tr_power_configuration_server_set_mains_voltage(ep, voltage_100mv);
        tr_power_configuration_server_printf("Status = 0x%x\n", status);
    }
    else
    {
        tr_power_configuration_server_printf("usage: mains_voltage -v voltage in 100mV [-e endpoint (default=1)]\n");
    }
    return ZB_TRUE;
}

zb_int_t cli_cmd_power_configuration_server_set_mains_frequency(zb_int_t  argc,
                                                                zb_char_t *argv[])
{
    zb_uint8_t      ep = 1;
    zb_uint8_t      frequency_2hz;
    zb_char_t       *option_argument;
    zb_uint8_t      ret_val = ZB_FALSE;
    zb_zcl_status_t status;

    // get the endpoint
    if (tr_cli_get_option(argc, argv, "e:", &option_argument))
    {
        ep = tr_dec_or_hex_string_to_int(option_argument);
    }

    // get the frequency
    if (tr_cli_get_option(argc, argv, "f:", &option_argument))
    {
        frequency_2hz = tr_dec_or_hex_string_to_int(option_argument);
        ret_val       = ZB_TRUE;
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        status = tr_power_configuration_server_set_mains_frequency(ep, frequency_2hz);
        tr_power_configuration_server_printf("Status = 0x%x\n", status);
    }
    else
    {
        tr_power_configuration_server_printf("usage: mains_freq -f frequency in 2 Hz [-e endpoint (default=1)]\n");
    }
    return ZB_TRUE;
}

zb_int_t cli_cmd_power_configuration_server_set_battery_voltage(zb_int_t  argc,
                                                                zb_char_t *argv[])
{
    zb_uint8_t                                     ep = 1;
    zb_uint8_t                                     voltage_100mv;
    tr_power_configuration_server_battery_source_t battery_source = TR_POWER_CONFIGURATION_BATTERY_SOURCE_1;
    zb_char_t                                      *option_argument;
    zb_uint8_t                                     ret_val = ZB_FALSE;
    zb_zcl_status_t                                status;

    // get the endpoint
    if (tr_cli_get_option(argc, argv, "e:", &option_argument))
    {
        ep = tr_dec_or_hex_string_to_int(option_argument);
    }

    // get the voltage
    if (tr_cli_get_option(argc, argv, "v:", &option_argument))
    {
        voltage_100mv = tr_dec_or_hex_string_to_int(option_argument);
        ret_val       = ZB_TRUE;
    }

    // get the battery source
    if (tr_cli_get_option(argc, argv, "s:", &option_argument))
    {
        battery_source = tr_dec_or_hex_string_to_int(option_argument);
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        status = tr_power_configuration_server_set_battery_voltage(ep, battery_source, voltage_100mv);
        tr_power_configuration_server_printf("Status = 0x%x\n", status);
    }
    else
    {
        tr_power_configuration_server_printf("usage: set_battery_voltage -v voltage in 100mV [-e endpoint (default=1)] [-s battery source (default=0)]\n");
    }
    return ZB_TRUE;
}

zb_int_t cli_cmd_power_configuration_server_set_battery_percentage_remaining(zb_int_t  argc,
                                                                             zb_char_t *argv[])
{
    zb_uint8_t                                     ep = 1;
    zb_uint8_t                                     percentage;
    tr_power_configuration_server_battery_source_t battery_source = TR_POWER_CONFIGURATION_BATTERY_SOURCE_1;
    zb_char_t                                      *option_argument;
    zb_uint8_t                                     ret_val = ZB_FALSE;
    zb_zcl_status_t                                status;

    // get the endpoint
    if (tr_cli_get_option(argc, argv, "e:", &option_argument))
    {
        ep = tr_dec_or_hex_string_to_int(option_argument);
    }

    // get the voltage
    if (tr_cli_get_option(argc, argv, "p:", &option_argument))
    {
        percentage = tr_dec_or_hex_string_to_int(option_argument);
        ret_val    = ZB_TRUE;
    }

    // get the battery source
    if (tr_cli_get_option(argc, argv, "s:", &option_argument))
    {
        battery_source = tr_dec_or_hex_string_to_int(option_argument);
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        status = tr_power_configuration_server_set_battery_percentage_remaining(ep, battery_source, percentage);
        tr_power_configuration_server_printf("Status = 0x%x\n", status);
    }
    else
    {
        tr_power_configuration_server_printf(
            "usage: set_battery_percentage -p percentage in 1/2 percent increment [-e endpoint (default=1)] [-s battery source (default=0)]\n");
    }
    return ZB_TRUE;
}

zb_int_t cli_cmd_power_configuration_server_set_clear_mains_power_lost(zb_int_t  argc,
                                                                       zb_char_t *argv[])
{
    zb_uint8_t ep = 1;
    zb_bool_t  mains_power_lost;
    zb_char_t  *option_argument;
    zb_uint8_t ret_val = ZB_FALSE;

    // get the endpoint
    if (tr_cli_get_option(argc, argv, "e:", &option_argument))
    {
        ep = tr_dec_or_hex_string_to_int(option_argument);
    }

    // is power lost? should be a -t then
    if (tr_cli_get_option(argc, argv, "t", &option_argument))
    {
        mains_power_lost = ZB_TRUE;
        ret_val          = ZB_TRUE;
    }

    // is power not lost? should be a -f then
    if (tr_cli_get_option(argc, argv, "f", &option_argument))
    {
        if (ret_val)
        {
            // we already had a -t, can't have -f also, set ret_val to false
            ret_val = ZB_FALSE;
        }
        else
        {
            mains_power_lost = ZB_FALSE;
            ret_val          = ZB_TRUE;
        }
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        tr_power_configuration_server_set_clear_mains_power_lost(ep, mains_power_lost);
    }
    else
    {
        tr_power_configuration_server_printf("usage: mains_power_lost -t (true) -or- -f (false) [-e endpoint (default=1)]\n");
    }
    return ZB_TRUE;
}

#ifdef TR_POWER_CONFIGURATION_SERVER_BATTERY_LATCHING

zb_int_t cli_cmd_power_configuration_server_unlatch_battery(zb_int_t  argc,
                                                            zb_char_t *argv[])
{
    zb_uint8_t                                     ep             = 1;
    tr_power_configuration_server_battery_source_t battery_source = TR_POWER_CONFIGURATION_BATTERY_SOURCE_1;
    zb_char_t                                      *option_argument;
    zb_uint8_t                                     ret_val = ZB_TRUE;

    // get the endpoint
    if (tr_cli_get_option(argc, argv, "e:", &option_argument))
    {
        ep = tr_dec_or_hex_string_to_int(option_argument);
    }

    // get the battery source
    if (tr_cli_get_option(argc, argv, "s:", &option_argument))
    {
        battery_source = tr_dec_or_hex_string_to_int(option_argument);
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (ret_val)
    {
        tr_power_configuration_server_unlatch_battery(ep, battery_source);
        tr_power_configuration_server_printf("Status = 0x00\n");
    }
    else
    {
        tr_power_configuration_server_printf(
            "usage: unlatch_battery [-e endpoint (default=1)] [-s battery source 0-2 (default=0)]\n");
    }
    return ZB_TRUE;
}

#endif /* ifdef TR_POWER_CONFIGURATION_SERVER_BATTERY_LATCHING */

TR_CLI_COMMAND_TABLE(zcl_power_configuration_s_plugin_commands) =
{
    { "mains_voltage",    cli_cmd_power_configuration_server_set_mains_voltage,                "set the mains voltage in units of 100mV"                     },
    { "mains_freq",       cli_cmd_power_configuration_server_set_mains_frequency,              "set the mains frequency in units of 2 Hz (1 = 2Hz)"          },
    { "batt_voltage",     cli_cmd_power_configuration_server_set_battery_voltage,              "set the battery voltage in units of 100mV"                   },
    { "batt_percent",     cli_cmd_power_configuration_server_set_battery_percentage_remaining, "set the battery percent remaining in units of 1/2% (1=0.5%)" },
    { "mains_power_lost", cli_cmd_power_configuration_server_set_clear_mains_power_lost,       "set or clear mains power lost"                               },
#ifdef TR_POWER_CONFIGURATION_SERVER_BATTERY_LATCHING
    { "unlatch_battery",  cli_cmd_power_configuration_server_unlatch_battery,                  "unlatch the battery alarm state for a battery source"        },
#endif /* ifdef TR_POWER_CONFIGURATION_SERVER_BATTERY_LATCHING */
    TR_CLI_COMMAND_TABLE_END
};
