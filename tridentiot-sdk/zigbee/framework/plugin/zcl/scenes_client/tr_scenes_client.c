/// ****************************************************************************
/// @file tr_scenes_client.c
///
/// @brief ZCL SCENES cluster client implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_scenes_client.h"

#define PLUGIN_NAME (zb_char_t*)("Scenes Client")

// TODO: used to wrap zboss plugin for now
void tr_scenes_client_init(void)
{
    zb_zcl_scenes_init_client();
}
