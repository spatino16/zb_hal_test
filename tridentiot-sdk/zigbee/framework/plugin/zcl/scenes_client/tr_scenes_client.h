/// ****************************************************************************
/// @file tr_scenes_client.h
///
/// @brief ZCL SCENES cluster client implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_SCENES_CLIENT_H
#define TR_SCENES_CLIENT_H

#include "tr_af.h"
#include "zb_zcl_scenes.h"

/// ****************************************************************************
///                                 debug prints
/// ****************************************************************************
#if defined(TR_SCENES_CLIENT_PLUGIN_PRINT_ENABLE) && (TR_SCENES_CLIENT_PLUGIN_PRINT_ENABLE == 1)
#define tr_scenes_client_printf(...)  tr_zcl_printf(__VA_ARGS__)
#define tr_scenes_client_println(...) tr_zcl_println(__VA_ARGS__)
#else
#define tr_scenes_client_printf(...)
#define tr_scenes_client_println(...)
#endif

/// ****************************************************************************
/// @defgroup zcl_scenes_client_cb Scenes Client Callbacks
/// @ingroup zcl_app_callbacks
/// @{
/// ****************************************************************************


/// @} // end of zcl_app_callbacks

/// ****************************************************************************
/// function prototypes consumed by framework
/// ****************************************************************************
void tr_scenes_client_init(void);

#endif // TR_SCENES_CLIENT_H
