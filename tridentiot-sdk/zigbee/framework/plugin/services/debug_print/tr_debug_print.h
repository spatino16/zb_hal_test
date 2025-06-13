/// ****************************************************************************
/// @file tr_debug_print.h
///
/// @brief debug print groups, colors, and control functions
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_DEBUG_PRINT_H
#define TR_DEBUG_PRINT_H
#include "zb_common.h"
#include "tr_plugin_config.h"

// debug print colors
#define DEBUG_BLACK   "\033[30m"
#define DEBUG_RED     "\033[31m"
#define DEBUG_GREEN   "\033[32m"
#define DEBUG_YELLOW  "\033[33m"
#define DEBUG_BLUE    "\033[34m"
#define DEBUG_MAGENTA "\033[35m"
#define DEBUG_CYAN    "\033[36m"
#define DEBUG_WHITE   "\033[37m"
#define DEBUG_DEFAULT "\033[39m"
#define DEBUG_RESET   "\033[m"

// set default color(s) if color print is disabled
#ifndef TR_DEBUG_PRINT_COLOR_ENABLED
#ifdef TR_DEBUG_PRINT_STACK_ENABLED
#define TR_DEBUG_PRINT_STACK_COLOR DEBUG_DEFAULT
#endif
#ifdef TR_DEBUG_PRINT_CORE_ENABLED
#define TR_DEBUG_PRINT_CORE_COLOR DEBUG_DEFAULT
#endif
#ifdef TR_DEBUG_PRINT_APP_ENABLED
#define TR_DEBUG_PRINT_APP_COLOR DEBUG_DEFAULT
#endif
#ifdef TR_DEBUG_PRINT_ZCL_ENABLED
#define TR_DEBUG_PRINT_ZCL_COLOR DEBUG_DEFAULT
#endif
#ifdef TR_DEBUG_PRINT_RX_MSGS_ENABLED
#define TR_DEBUG_PRINT_RX_MSGS_COLOR DEBUG_DEFAULT
#endif
#endif /* ifndef TR_DEBUG_PRINT_COLOR_ENABLED */

// print groups
typedef enum
{
    TR_DEBUG_PRINT_STACK   = 0x01,
    TR_DEBUG_PRINT_CORE    = 0x02,
    TR_DEBUG_PRINT_APP     = 0x04,
    TR_DEBUG_PRINT_ZCL     = 0x08,
    TR_DEBUG_PRINT_RX_MSGS = 0x10,
    TR_DEBUG_PRINT_ALL     = 0x1F
} tr_debug_print_group_t;

#ifdef TR_DEBUG_PRINT_STACK_ENABLED
#define tr_stack_printf(...)  tr_debug_printf(TR_DEBUG_PRINT_STACK, TR_DEBUG_PRINT_STACK_COLOR, __VA_ARGS__)
#define tr_stack_println(...) tr_debug_println(TR_DEBUG_PRINT_STACK, TR_DEBUG_PRINT_STACK_COLOR, __VA_ARGS__)
#else
#define tr_stack_printf(...)
#define tr_stack_println(...)
#endif

#ifdef TR_DEBUG_PRINT_CORE_ENABLED
#define tr_core_printf(...)  tr_debug_printf(TR_DEBUG_PRINT_CORE, TR_DEBUG_PRINT_CORE_COLOR, __VA_ARGS__)
#define tr_core_println(...) tr_debug_println(TR_DEBUG_PRINT_CORE, TR_DEBUG_PRINT_CORE_COLOR, __VA_ARGS__)
#else
#define tr_core_printf(...)
#define tr_core_println(...)
#endif

#ifdef TR_DEBUG_PRINT_APP_ENABLED
#define tr_app_printf(...)  tr_debug_printf(TR_DEBUG_PRINT_APP, TR_DEBUG_PRINT_APP_COLOR, __VA_ARGS__)
#define tr_app_println(...) tr_debug_println(TR_DEBUG_PRINT_APP, TR_DEBUG_PRINT_APP_COLOR, __VA_ARGS__)
#else
#define tr_app_printf(...)
#define tr_app_println(...)
#endif

#ifdef TR_DEBUG_PRINT_ZCL_ENABLED
#define tr_zcl_printf(...)  tr_debug_printf(TR_DEBUG_PRINT_ZCL, TR_DEBUG_PRINT_ZCL_COLOR, __VA_ARGS__)
#define tr_zcl_println(...) tr_debug_println(TR_DEBUG_PRINT_ZCL, TR_DEBUG_PRINT_ZCL_COLOR, __VA_ARGS__)
#else
#define tr_zcl_printf(...)
#define tr_zcl_println(...)
#endif

#ifdef TR_DEBUG_PRINT_RX_MSGS_ENABLED
#define tr_rxmsgs_printf(...)  tr_debug_printf(TR_DEBUG_PRINT_RX_MSGS, TR_DEBUG_PRINT_RX_MSGS_COLOR, __VA_ARGS__)
#define tr_rxmsgs_println(...) tr_debug_println(TR_DEBUG_PRINT_RX_MSGS, TR_DEBUG_PRINT_RX_MSGS_COLOR, __VA_ARGS__)
#else
#define tr_rxmsgs_printf(...)
#define tr_rxmsgs_println(...)
#endif

/// ****************************************************************************
/// @defgroup services_api_debug_print Debug Print API References
/// @details This plugin utilizes print group definitions to enable control over
/// grouping similar types of prints together and being able to turn those print
/// groups on and off. See @ref tr_debug_print.h for defined print groups.
///
/// @ingroup services_api_references
/// @{
/// ****************************************************************************

/// @brief TODO - documentation
/// @return
zb_uint32_t tr_get_print_group_mask(void);

/// @brief TODO - documentation
/// @param group
void tr_enable_print_group(tr_debug_print_group_t group);

/// @brief TODO - documentation
/// @param group
void tr_disable_print_group(tr_debug_print_group_t group);

/// @brief TODO - documentation
/// @param group
/// @return
zb_bool_t tr_check_print_group(tr_debug_print_group_t group);

/// @} // end of services_api_references

/// ****************************************************************************
/// function prototypes consumed by framework
/// ****************************************************************************
void tr_debug_printf(tr_debug_print_group_t group,
                     const char             *color,
                     const char             *fmt,
                     ...);

void tr_debug_println(tr_debug_print_group_t group,
                      const char             *color,
                      const char             *fmt,
                      ...);

char *tr_find_cluster_name(zb_uint16_t cluster_id);

#endif // TR_DEBUG_PRINT_H
