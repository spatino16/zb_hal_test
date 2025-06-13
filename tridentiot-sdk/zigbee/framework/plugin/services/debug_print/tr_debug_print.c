/// ****************************************************************************
/// @file tr_debug_print.c
///
/// @brief debug print groups, colors, and control functions
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_printf.h"
#include "tr_af.h"

// mask used for enabling and disabling the print groups
static zb_uint32_t print_group_mask =
    (0
#ifdef TR_DEBUG_PRINT_STACK_ENABLED
     | TR_DEBUG_PRINT_STACK
#endif
#ifdef TR_DEBUG_PRINT_CORE_ENABLED
     | TR_DEBUG_PRINT_CORE
#endif
#ifdef TR_DEBUG_PRINT_APP_ENABLED
     | TR_DEBUG_PRINT_APP
#endif
#ifdef TR_DEBUG_PRINT_ZCL_ENABLED
     | TR_DEBUG_PRINT_ZCL
#endif
#ifdef TR_DEBUG_PRINT_RX_MSGS_ENABLED
     | TR_DEBUG_PRINT_RX_MSGS
#endif
    );

// get the print group mask
zb_uint32_t tr_get_print_group_mask(void)
{
    return print_group_mask;
}

// enable a print group
void tr_enable_print_group(tr_debug_print_group_t group)
{
    print_group_mask |= group;
}

// disable a print group
void tr_disable_print_group(tr_debug_print_group_t group)
{
    print_group_mask &= ~group;
}

// check to see if a print group is enabled
zb_bool_t tr_check_print_group(tr_debug_print_group_t group)
{
    if (print_group_mask & group)
    {
        return ZB_TRUE;
    }
    return ZB_FALSE;
}

// base printf command that includes color
void tr_debug_printf(tr_debug_print_group_t group,
                     const char             *color,
                     const char             *fmt,
                     ...)
{
    va_list va;

    // check the group
    if (tr_check_print_group(group))
    {
        // if all is well, do the print
        va_start(va, fmt);
#ifdef TR_DEBUG_PRINT_COLOR_ENABLED
        tr_vsprintf(0, 0, color, va);
#endif
        tr_vsprintf(0, 0, fmt, va);
#ifdef TR_DEBUG_PRINT_COLOR_ENABLED
        tr_vsprintf(0, 0, DEBUG_RESET, va);
#endif
        va_end(va);
    }
}

// this is the print used in any cli_common code and
// uses the CORE print group and color
void tr_cli_common_printf(const char *pFormat,
                          ...)
{
    va_list va;

    // check the group
    if (tr_check_print_group(TR_DEBUG_PRINT_CORE))
    {
        // if all is well, do the print
        va_start(va, pFormat);
#ifdef TR_DEBUG_PRINT_COLOR_ENABLED
        tr_vsprintf(0, 0, TR_DEBUG_PRINT_CORE_COLOR, va);
#endif
        tr_vsprintf(0, 0, pFormat, va);
#ifdef TR_DEBUG_PRINT_COLOR_ENABLED
        tr_vsprintf(0, 0, DEBUG_RESET, va);
#endif
        va_end(va);
    }
}

// base println command that includes color
void tr_debug_println(tr_debug_print_group_t group,
                      const char             *color,
                      const char             *fmt,
                      ...)
{
    va_list va;

    // check the group
    if (tr_check_print_group(group))
    {
        // if all is well, do the print
        va_start(va, fmt);
#ifdef TR_DEBUG_PRINT_COLOR_ENABLED
        tr_vsprintf(0, 0, color, va);
#endif
        tr_vsprintf(0, 0, fmt, va);
#ifdef TR_DEBUG_PRINT_COLOR_ENABLED
        tr_vsprintf(0, 0, DEBUG_RESET, va);
#endif
        tr_vsprintf(0, 0, "\n", va);
        va_end(va);
    }
}

char *tr_find_cluster_name(zb_uint16_t cluster_id)
{
    zb_uint8_t index = 0;

    while (tr_cluster_names[index].cluster_id != 0xFFFF)
    {
        if (tr_cluster_names[index].cluster_id == cluster_id)
        {
            break;
            // return tr_cluster_names[index].cluster_name;
        }
        index++;
    }
    return tr_cluster_names[index].cluster_name;
}
