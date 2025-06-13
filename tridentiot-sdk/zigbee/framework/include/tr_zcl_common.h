/// ****************************************************************************
/// @file tr_zcl_common.h
///
/// @brief entry point for all incoming ZCL messages
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_ZCL_COMMON_H
#define TR_ZCL_COMMON_H

/// ****************************************************************************
/// @defgroup zcl_common_cb ZCL Common Callbacks
/// @ingroup common_app_callbacks
/// @{
/// ****************************************************************************

/// @brief callback that can be defined in the user application to handle incoming
/// ZCL messages
/// @param param get ZCL packet info using: zb_zcl_parsed_hdr_t *cmd_info = ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t);
/// @return 0 to allow framework to continue processing the command,
/// 1 to prevent framework from processing the command
zb_uint8_t tr_zcl_command_cb(zb_uint8_t param);

/// @} // end of common_app_callbacks

zb_uint8_t tr_zcl_specific_cluster_cmd_handler(zb_uint8_t param);

#endif /* TR_ZCL_COMMON_H */
