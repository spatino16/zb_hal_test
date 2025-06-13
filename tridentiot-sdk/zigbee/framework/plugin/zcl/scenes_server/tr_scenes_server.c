/// ****************************************************************************
/// @file tr_scenes_server.c
///
/// @brief ZCL SCENES cluster server implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_scenes_server.h"

#define PLUGIN_NAME (zb_char_t*)("Scenes Server")

// TODO: used to wrap zboss plugin for now
void tr_scenes_server_init(void)
{
    zb_zcl_scenes_init_server();
}
