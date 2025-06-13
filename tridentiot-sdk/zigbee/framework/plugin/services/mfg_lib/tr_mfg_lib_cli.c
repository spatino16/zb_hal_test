/// ****************************************************************************
/// @file tr_mfg_lib_cli.c
///
/// @brief manufacturing library cli commands
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_af.h"
#include "tr_cli_argument_parser.h"
#include "tr_tlv_parser.h"
#include "flashctl.h"

#define XTAL_TRIM_TAG  0x01
#define XTAL_TRIM_SIZE 0x02

extern TR_CLI_COMMAND_TABLE(tx_power_commands);
extern TR_CLI_COMMAND_TABLE(cca_commands);

static bool g_enabled = false;

zb_int_t cli_mfg_start(zb_int_t  argc,
                       zb_char_t *argv[])
{
    zb_char_t *option_argument;

    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        tr_core_printf("usage: start\n");
    }
    else
    {
        zb_sched_stop();
        g_enabled = true;
    }

    return 0;
}

zb_int_t cli_mfg_stop(zb_int_t  argc,
                      zb_char_t *argv[])
{
    zb_char_t *option_argument;

    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        tr_core_printf("usage: stop\n");
    }
    else
    {
        zb_sched_init();
        g_enabled = false;
    }

    return 0;
}

zb_int_t cli_mfg_tx_tone(zb_int_t  argc,
                         zb_char_t *argv[])
{
    zb_uint32_t chan_or_freq = 0;
    zb_char_t   *option_argument;
    zb_int_t    status;

    if (!g_enabled)
    {
        tr_core_printf("Must start MFG library before using TX commands!\n");
        return 0;
    }

    if (tr_cli_get_option(argc, argv, "c:", &option_argument))
    {
        chan_or_freq = (zb_uint32_t)tr_dec_or_hex_string_to_int(option_argument);
    }

    if (tr_cli_get_option(argc, argv, "f:", &option_argument))
    {
        chan_or_freq = (zb_uint32_t)tr_dec_or_hex_string_to_int(option_argument);
    }

    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        tr_core_printf("usage: txtone [-c channel (0 to stop tone)] -or- [-f freq in MHz (0 to stop tone)]\n");
        return 0;
    }

    // start or stop the tone
    status = tr_tx_tone(chan_or_freq);

    if (0 == chan_or_freq)
    {
        if (0 == status)
        {
            tr_core_printf("TX tone stopped\n");
        }
        else
        {
            tr_core_printf("TX tone stop failed, status %d\n", status);
            tr_core_printf("Is the MFG library running?\n");
        }
    }
    else
    {
        if (0 == status)
        {
            tr_core_printf("TX tone started on %d, power %d\n", chan_or_freq, tr_get_tx_power());
        }
        else
        {
            tr_core_printf("TX tone start failed, status %d\n", status);
            tr_core_printf("Is the MFG library running?\n");
        }
    }

    return 0;
}

zb_int_t cli_mfg_xtal_try(zb_int_t  argc,
                          zb_char_t *argv[])
{
    zb_int_t  xtal_value = 0;
    zb_bool_t ret_val    = ZB_FALSE;
    zb_char_t *option_argument;

    if (tr_cli_get_option(argc, argv, "v:", &option_argument))
    {
        xtal_value = tr_dec_or_hex_string_to_int(option_argument);

#if defined(TR_PLATFORM_T32CM11C)

        if (xtal_value <= 1023)
#endif
        {
            ret_val = ZB_TRUE;
        }
    }

    if (ret_val)
    {
        tr_core_printf("Trying XTAL Calibration: %04X\n", (zb_uint16_t)xtal_value);
        tr_platform_token_process_xtal_trim(xtal_value);
    }
    else
    {
#if defined(TR_PLATFORM_T32CM11C)
        tr_core_printf("usage: xtal try -v value (0-1023)\n");
#else
        tr_core_printf("unknown platform!\n");
#endif
    }
    return 0;
}

zb_int_t cli_mfg_xtal_get(zb_int_t  argc,
                          zb_char_t *argv[])
{
    zb_bool_t ret_val  = ZB_TRUE;
    zb_bool_t internal = ZB_FALSE;
    zb_char_t *option_argument;

    if (tr_cli_get_option(argc, argv, "h", &option_argument))
    {
        ret_val = ZB_FALSE;
    }

    // internal use only
    if (tr_cli_get_option(argc, argv, "i", &option_argument))
    {
        internal = ZB_TRUE;
    }

    if (ret_val)
    {
        if (internal == ZB_FALSE)
        {
            tr_mfg_tok_type_xtal_trim xtal_trim;
            tr_get_mfg_token(&xtal_trim, TR_MFG_TOKEN_XTAL_TRIM);
            tr_core_printf("XTAL Calibration Value: %04X\n", (zb_uint16_t)xtal_trim.value);
        }
        else
        {
            zb_uint8_t sec_page_buf[LENGTH_PAGE];
            zb_uint8_t xtal_value[2];
            zb_uint8_t xtal_value_len;
            flash_read_sec_register((zb_uint32_t)sec_page_buf, FLASH_SECREG_R1_P2);

            if (!find_tlv_by_tag(sec_page_buf, LENGTH_PAGE, 0x01, xtal_value, &xtal_value_len) ||
                (xtal_value_len != sizeof(xtal_value)))
            {
                xtal_value[0] = 0xFF;
                xtal_value[1] = 0xFF;
            }
            zb_uint16_t xtal_cal = ((zb_uint16_t)xtal_value[1]) | ((zb_uint16_t)(xtal_value[0] << 8));
            tr_core_printf("Internal XTAL Calibration Value: %04X\n", (zb_uint16_t)xtal_cal);
        }
    }
    else
    {
        tr_core_printf("usage: mfg_lib xtal get\n");
    }

    return 0;
}

zb_int_t cli_mfg_xtal_set(zb_int_t  argc,
                          zb_char_t *argv[])
{
    zb_int_t  xtal_value = 0;
    zb_bool_t ret_val    = ZB_FALSE;
    zb_bool_t internal   = ZB_FALSE;
    zb_char_t *option_argument;

    if (tr_cli_get_option(argc, argv, "v:", &option_argument))
    {
        xtal_value = tr_dec_or_hex_string_to_int(option_argument);

#if defined(TR_PLATFORM_T32CM11C)

        if (xtal_value <= 1023)
#endif
        {
            ret_val = ZB_TRUE;
        }
    }

    // internal use only
    if (tr_cli_get_option(argc, argv, "i", &option_argument))
    {
        internal = ZB_TRUE;
    }

    if (ret_val)
    {
        tr_core_printf("Setting XTAL Calibration: %04X\n", (zb_uint16_t)xtal_value);

        if (internal == ZB_FALSE)
        {
            if (!tr_mfg_token_check_erased(TR_MFG_TOKEN_XTAL_TRIM))
            {
                tr_core_printf("XTAL token already written!\n");
            }
            else
            {
                tr_mfg_tok_type_xtal_trim xtal_trim;
                xtal_trim.value = xtal_value;
                tr_set_mfg_token((uint8_t*)&xtal_trim, sizeof(tr_mfg_tok_type_xtal_trim), TR_MFG_TOKEN_XTAL_TRIM);
                tr_core_printf("XTAL token write success\n");
            }
        }
        else
        {
            zb_uint8_t sec_page_buf[3][LENGTH_PAGE];
            zb_uint8_t xtal_trim[2];
            flash_read_sec_register((zb_uint32_t)&sec_page_buf[0], FLASH_SECREG_R1_P0);
            flash_read_sec_register((zb_uint32_t)&sec_page_buf[1], FLASH_SECREG_R1_P1);
            flash_read_sec_register((zb_uint32_t)&sec_page_buf[2], FLASH_SECREG_R1_P2);
            flash_erase(FLASH_ERASE_SECURE, FLASH_SECREG_R1_P0);
            // convert to big-endian order
            xtal_trim[0] = (zb_uint8_t)(xtal_value >> 8);
            xtal_trim[1] = (zb_uint8_t)(xtal_value & 0xFF);
            update_tlv_tag(sec_page_buf[2], LENGTH_PAGE, XTAL_TRIM_TAG, xtal_trim, XTAL_TRIM_SIZE);
            flash_write_sec_register((zb_uint32_t)&sec_page_buf[0], FLASH_SECREG_R1_P0);

            while (flash_check_busy()){ /*do nothing*/ };
            flash_write_sec_register((zb_uint32_t)&sec_page_buf[1], FLASH_SECREG_R1_P1);

            while (flash_check_busy()){ /*do nothing*/ };
            flash_write_sec_register((zb_uint32_t)&sec_page_buf[2], FLASH_SECREG_R1_P2);

            while (flash_check_busy()){ /*do nothing*/ };

            tr_core_printf("Internal XTAL token write success\n");
        }
    }
    else
    {
#if defined(TR_PLATFORM_T32CM11C)
        tr_core_printf("usage: xtal set -v value (0-1023)\n");
#else
        tr_core_printf("unknown platform!\n");
#endif
    }
    return 0;
}

TR_CLI_COMMAND_TABLE(xtal_commands) =
{
    { "try", cli_mfg_xtal_try, "Try XTAL calibration value without writing it "  },
    { "get", cli_mfg_xtal_get, "Get XTAL calibration value"                      },
    { "set", cli_mfg_xtal_set, "Write XTAL calibration value into a flash token" },
    TR_CLI_COMMAND_TABLE_END
};

TR_CLI_COMMAND_TABLE(mfg_lib_commands) =
{
    { "start",    cli_mfg_start,       "Start MFG library, MUST be done first!"    },
    { "stop",     cli_mfg_stop,        "Stop MFG library"                          },
    { "txtone",   cli_mfg_tx_tone,     "Start or stop continuous tone"             },
    { "tx_power", TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(tx_power_commands) },
    { "cca",      TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(cca_commands)      },
    { "xtal",     TR_CLI_SUB_COMMANDS, TR_CLI_SUB_COMMAND_TABLE(xtal_commands)     },
    TR_CLI_COMMAND_TABLE_END
};
