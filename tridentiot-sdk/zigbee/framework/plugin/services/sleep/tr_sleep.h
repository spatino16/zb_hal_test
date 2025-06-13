/// ****************************************************************************
/// @file tr_sleep.h
///
/// @brief plugin for allowing sleep and getting alerted about pre/post sleep
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_SLEEP_H
#define TR_SLEEP_H

/// ****************************************************************************
/// @defgroup zb_sleep_cb Sleep Callbacks
/// @ingroup services_app_callbacks
/// @{
/// ****************************************************************************

/// @brief Callback that fires before going to sleep
/// @param sleep_duration_ms sleep duration in milliseconds
/// @return ZB_TRUE if ok to sleep, ZB_FALSE if not
zb_bool_t tr_pre_sleep_cb(zb_uint32_t sleep_duration_ms);

/// @brief Callback that fires after waking from sleep
void tr_post_wake_cb(void);

/// @} // end of services_app_callbacks

/// ****************************************************************************
/// @defgroup services_api_sleep Sleep API References
/// @ingroup services_api_references
/// @{
/// ****************************************************************************

/// @brief API to allow app to disable or enable sleep
/// @param allow ZB_TRUE to allow device to sleep, ZB_FALSE to prevent sleep
void tr_allow_sleep(zb_bool_t allow);

/// @brief API to check to see if sleep is allowed
/// @param sleep_time_ms sleep duration in milliseconds
/// @return ZB_TRUE if ok to sleep, ZB_FALSE if not
zb_bool_t tr_check_for_sleep(zb_uint32_t sleep_time_ms);

/// @} // end of services_api_references

#endif // TR_SLEEP_H
