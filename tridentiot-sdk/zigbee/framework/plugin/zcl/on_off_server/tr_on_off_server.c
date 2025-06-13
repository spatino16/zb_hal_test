/// ****************************************************************************
/// @file tr_on_off_server.c
///
/// @brief ZCL ON/OFF cluster server implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_on_off_server.h"

#define PLUGIN_NAME (zb_char_t*)("On Off Server")

// TODO: used to wrap zboss plugin for now
void tr_on_off_server_init(void)
{
    zb_zcl_on_off_init_server();
}
