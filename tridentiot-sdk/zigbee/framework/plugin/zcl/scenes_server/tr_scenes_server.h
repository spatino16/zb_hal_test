/// ****************************************************************************
/// @file tr_scenes_server.h
///
/// @brief ZCL SCENES cluster server implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_SCENES_SERVER_H
#define TR_SCENES_SERVER_H

#include "tr_af.h"
#include "zb_zcl_scenes.h"

/// ****************************************************************************
///                                 debug prints
/// ****************************************************************************
#if defined(TR_SCENES_SERVER_PLUGIN_PRINT_ENABLE) && (TR_SCENES_SERVER_PLUGIN_PRINT_ENABLE == 1)
#define tr_scenes_server_printf(...)  tr_zcl_printf(__VA_ARGS__)
#define tr_scenes_server_println(...) tr_zcl_println(__VA_ARGS__)
#else
#define tr_scenes_server_printf(...)
#define tr_scenes_server_println(...)
#endif

/// ****************************************************************************
/// @defgroup zcl_scenes_server_cb Scenes Server Callbacks
/// @ingroup zcl_app_callbacks
/// @{
/// ****************************************************************************


/// @} // end of zcl_app_callbacks

/// ****************************************************************************
/// function prototypes consumed by framework
/// ****************************************************************************
void tr_scenes_server_init(void);

#endif // TR_SCENES_SERVER_H
