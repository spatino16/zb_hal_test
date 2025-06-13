/// ****************************************************************************
/// @file tr_debug_print_cli.c
///
/// @brief CLI commands for controlling debug print groups
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include <unistd.h>
#include "tr_af.h"
#include "tr_cli_argument_parser.h"

zb_int_t cli_cmd_print_status(zb_int_t  argc,
                              zb_char_t *argv[])
{
    tr_core_printf("Group mask 0x%8.8x\n", tr_get_print_group_mask());
    tr_core_printf("   Stack:   ");

    if (tr_check_print_group(TR_DEBUG_PRINT_STACK))
    {
        tr_core_printf("enabled\n");
    }
    else
    {
        tr_core_printf("disabled\n");
    }

    tr_core_printf("   Core:    ");

    if (tr_check_print_group(TR_DEBUG_PRINT_CORE))
    {
        tr_core_printf("enabled\n");
    }
    else
    {
        tr_core_printf("disabled\n");
    }

    tr_core_printf("   App:     ");

    if (tr_check_print_group(TR_DEBUG_PRINT_APP))
    {
        tr_core_printf("enabled\n");
    }
    else
    {
        tr_core_printf("disabled\n");
    }

    tr_core_printf("   ZCL:     ");

    if (tr_check_print_group(TR_DEBUG_PRINT_ZCL))
    {
        tr_core_printf("enabled\n");
    }
    else
    {
        tr_core_printf("disabled\n");
    }

    tr_core_printf("   RX msgs: ");

    if (tr_check_print_group(TR_DEBUG_PRINT_RX_MSGS))
    {
        tr_core_printf("enabled\n");
    }
    else
    {
        tr_core_printf("disabled\n");
    }
    return 0;
}

zb_int_t cli_cmd_enable_group(zb_int_t  argc,
                              zb_char_t *argv[])
{
    zb_char_t  *option_argument;
    zb_uint8_t ret_val = ZB_FALSE;

    if (tr_cli_get_option(argc, argv, "s", &option_argument))
    {
        tr_enable_print_group(TR_DEBUG_PRINT_STACK);
        ret_val = ZB_TRUE;
    }

    if (tr_cli_get_option(argc, argv, "c", &option_argument))
    {
        tr_enable_print_group(TR_DEBUG_PRINT_CORE);
        ret_val = ZB_TRUE;
    }

    if (tr_cli_get_option(argc, argv, "a", &option_argument))
    {
        tr_enable_print_group(TR_DEBUG_PRINT_APP);
        ret_val = ZB_TRUE;
    }

    if (tr_cli_get_option(argc, argv, "z", &option_argument))
    {
        tr_enable_print_group(TR_DEBUG_PRINT_ZCL);
        ret_val = ZB_TRUE;
    }

    if (tr_cli_get_option(argc, argv, "r", &option_argument))
    {
        tr_enable_print_group(TR_DEBUG_PRINT_RX_MSGS);
        ret_val = ZB_TRUE;
    }

    if (tr_cli_get_option(argc, argv, "l", &option_argument))
    {
        tr_enable_print_group(TR_DEBUG_PRINT_ALL);
        ret_val = ZB_TRUE;
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (!ret_val)
    {
        tr_core_printf("usage: debug enable [-s (stack)] [-c (core)] [-a (app)] [-z (zcl)] [-r (rx msgs)] [-l (all)]\n");
    }
    return 0;
}

zb_int_t cli_cmd_disable_group(zb_int_t  argc,
                               zb_char_t *argv[])
{
    zb_char_t  *option_argument;
    zb_uint8_t ret_val = ZB_FALSE;

    if (tr_cli_get_option(argc, argv, "s", &option_argument))
    {
        tr_disable_print_group(TR_DEBUG_PRINT_STACK);
        ret_val = ZB_TRUE;
    }

    if (tr_cli_get_option(argc, argv, "c", &option_argument))
    {
        tr_disable_print_group(TR_DEBUG_PRINT_CORE);
        ret_val = ZB_TRUE;
    }

    if (tr_cli_get_option(argc, argv, "a", &option_argument))
    {
        tr_disable_print_group(TR_DEBUG_PRINT_APP);
        ret_val = ZB_TRUE;
    }

    if (tr_cli_get_option(argc, argv, "z", &option_argument))
    {
        tr_disable_print_group(TR_DEBUG_PRINT_ZCL);
        ret_val = ZB_TRUE;
    }

    if (tr_cli_get_option(argc, argv, "r", &option_argument))
    {
        tr_disable_print_group(TR_DEBUG_PRINT_RX_MSGS);
        ret_val = ZB_TRUE;
    }

    if (tr_cli_get_option(argc, argv, "l", &option_argument))
    {
        tr_disable_print_group(TR_DEBUG_PRINT_ALL);
        ret_val = ZB_TRUE;
    }

    // help option
    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    if (!ret_val)
    {
        tr_core_printf("usage: debug disable [-s (stack)] [-c (core)] [-a (app)] [-z (zcl)] [-r (rx msgs)] [-l (all)]\n");
    }
    return 0;
}

TR_CLI_COMMAND_TABLE(debug_print_commands) =
{
    { "status",  cli_cmd_print_status,  "Print the debug group status"       },
    { "enable",  cli_cmd_enable_group,  "Enable a debug print group"         },
    { "disable", cli_cmd_disable_group, "Disable a debug print group"        },
    TR_CLI_COMMAND_TABLE_END
};
