/// ****************************************************************************
/// @file tr_over_the_air_bootloading_client.h
///
/// @brief Structure, enum, default values, and API prototypes for the OTA
/// bootload client plugin.
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_OVER_THE_AIR_BOOTLOADING_CLIENT_H
#define TR_OVER_THE_AIR_BOOTLOADING_CLIENT_H

#ifndef TR_OTA_UPGRADE_QUERY_DELAY_MIN
#define TR_OTA_UPGRADE_QUERY_DELAY_MIN 5
#endif

#ifndef TR_OTA_UPGRADE_MAX_DATA_SIZE
#define TR_OTA_UPGRADE_MAX_DATA_SIZE 64
#endif

typedef enum
{
    TR_OTA_UPGRADE_STATE_IDLE,
    TR_OTA_UPGRADE_STATE_IN_PROGRESS,
    TR_OTA_UPGRADE_STATE_RESUME
}tr_ota_upgrade_client_state_t;

typedef struct
{
    zb_uint8_t  endpoint;
    zb_uint16_t short_addr;
    zb_uint32_t image_size;
    zb_uint8_t  param;
    zb_uint32_t fw_version;
}tr_ota_server_info_t;

typedef struct
{
    zb_uint32_t fw_version;
    zb_uint32_t download_file_size;
    zb_uint32_t offset_at_last_attr_save;
    zb_uint16_t image_type;
    zb_uint16_t mfg_id;
    zb_uint16_t hw_version;
    zb_uint8_t  query_delay;
    zb_uint8_t  img_block_req_sent;
    zb_uint8_t  pending_img_block_resp;
    zb_uint8_t  max_data_size;
}tr_ota_client_info_t;

/// ****************************************************************************
///                                 debug prints
/// ****************************************************************************
#if defined(TR_OVER_THE_AIR_BOOTLOADING_CLIENT_PLUGIN_PRINT_ENABLE) && \
    (TR_OVER_THE_AIR_BOOTLOADING_CLIENT_PLUGIN_PRINT_ENABLE == 1)
#define tr_ota_upgrade_client_printf(...)  tr_zcl_printf(__VA_ARGS__)
#define tr_ota_upgrade_client_println(...) tr_zcl_println(__VA_ARGS__)
#else
#define tr_ota_upgrade_client_printf(...)
#define tr_ota_upgrade_client_println(...)
#endif

/// ****************************************************************************
/// @defgroup zcl_ota_client_cb Over the Air Bootloading Client Callbacks
/// @ingroup zcl_app_callbacks
/// @{
/// ****************************************************************************

/// @brief Callback fires to allow app to provide information for a query next image request
/// @param endpoint device endpoint
/// @param fw_version pointer used to return FW version
/// @param mfg_id pointer used to return manufacturer id
/// @param image_type pointer used to return image type
/// @param hw_version pointer used to return hardware version
void tr_over_the_air_bootloading_client_version_cb(zb_uint8_t  endpoint,
                                                   zb_uint32_t *fw_version,
                                                   zb_uint16_t *mfg_id,
                                                   zb_uint16_t *image_type,
                                                   zb_uint16_t *hw_version);

/// @brief Callback fires to notify app that OTA download is about to start
/// @param image_version FW version of new image
/// @param image_size size of new image
void tr_over_the_air_bootloading_client_upgrade_start_cb(zb_uint32_t image_version,
                                                         zb_uint32_t image_size);

/// @brief Callback fires to alert app that an OTA upgrade server was not found
void tr_over_the_air_bootloading_client_server_not_found_cb(void);

/// @brief Callback fires when the OTA upgrade client is initialized
void tr_over_the_air_bootloading_client_init_cb(void);

/// @brief Callback fires when an OTA upgrade image notifiy is received
/// @return ZB_TRUE to allow image notify to be processed by the plugin, ZB_FALSE if app handled it
zb_bool_t tr_over_the_air_bootloading_client_image_notify_cb(void);

/// @brief Callback fires to allow app to decide if an image should be downloaded
/// @param fw_version FW version from the server
/// @param image_type Image type from the server
/// @param mfg_id Mfg ID form the server
/// @return ZB_TRUE to allow plugin to process query next image response, ZB_FALSE if app handled it
zb_bool_t tr_over_the_air_bootloading_client_query_next_image_resp_cb(zb_uint32_t fw_version,
                                                                      zb_uint16_t image_type,
                                                                      zb_uint16_t mfg_id);

/// @brief Callback fires to allow app
/// @return ZB_TRUE to allow plugin to process upgrade end response, ZB_FALSE if app handled it
zb_bool_t tr_over_the_air_bootloading_client_upgrade_end_resp_cb(void);

/// @} // end of zcl_app_callbacks

/// ****************************************************************************
/// @defgroup zcl_ota_client_apis Over the Air Bootloading Client APIs
/// @ingroup zcl_api_references
/// @{
/// ****************************************************************************

/// @brief API to get the OTA server info
/// @return pointer to server info structure
tr_ota_server_info_t *tr_ota_upgrade_client_get_server_info(void);

/// @brief API to get the OTA client info
/// @return pointer to client info structure
tr_ota_client_info_t *tr_ota_upgrade_client_get_client_info(void);

/// @brief API to stop the OTA upgrade client
void zb_zcl_ota_upgrade_stop_client(void);

/// @brief API to pause the OTA upgrade client
void zb_zcl_ota_upgrade_pause_client(void);

/// @} // end of zcl_api_references

/// ****************************************************************************
/// function prototypes consumed by framework
/// ****************************************************************************
void tr_over_the_air_bootloading_client_connection_state_cb(tr_conn_state_e conn_state);
void tr_ota_init(zb_uint8_t param);
void tr_over_the_air_bootloading_client_init(void);

#endif // TR_OVER_THE_AIR_BOOTLOADING_CLIENT_H
