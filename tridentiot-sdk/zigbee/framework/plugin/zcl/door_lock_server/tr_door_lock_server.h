/// ****************************************************************************
/// @file tr_door_lock_server.h
///
/// @brief ZCL DOOR LOCK cluster server implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_DOOR_LOCK_SERVER_H
#define TR_DOOR_LOCK_SERVER_H

#include "tr_af.h"

// TODO: LCD 2/6/25 These structures will go away once zap generates them
// door lock users structure
typedef ZB_PACKED_PRE struct
{
    tr_door_lock_user_status_t status;
    tr_door_lock_user_type_t   type;
    zb_uint8_t                 code[TR_DOOR_LOCK_SERVER_MAX_PIN_LEN + 1];
}tr_door_lock_server_user_t;

// door lock server user pin code command arg struct
typedef ZB_PACKED_PRE struct
{
    zb_uint16_t user_id;
    zb_uint8_t  user_status;
    zb_uint8_t  user_type;
    zb_uint8_t  pin_code[];
}tr_door_lock_server_pin_code_arg_t;

// door lock server set/get user status command struct
typedef ZB_PACKED_PRE struct
{
    zb_uint16_t user_id;
    zb_uint8_t  user_status;
}tr_door_lock_server_user_status_arg_t;

// door lock server set/get user type command struct
typedef ZB_PACKED_PRE struct
{
    zb_uint16_t user_id;
    zb_uint8_t  user_type;
}tr_door_lock_server_user_type_arg_t;

/// ****************************************************************************
///                                 debug prints
/// ****************************************************************************
#if defined(TR_DOOR_LOCK_SERVER_PLUGIN_PRINT_ENABLE) && (TR_DOOR_LOCK_SERVER_PLUGIN_PRINT_ENABLE == 1)
#define tr_door_lock_server_printf(...)  tr_zcl_printf(__VA_ARGS__)
#define tr_door_lock_server_println(...) tr_zcl_println(__VA_ARGS__)
#else
#define tr_door_lock_server_printf(...)
#define tr_door_lock_server_println(...)
#endif

/// ****************************************************************************
/// @defgroup zcl_door_lock_server_cb Door Lock Server Callbacks
/// @ingroup zcl_app_callbacks
/// @{
/// ****************************************************************************

/// @brief Callback fires when the door lock server cluster plugin is initialized
void tr_door_lock_server_init_cb(void);

/// @brief Callback that user can declare to handle door lock cluster lock door command
/// @param cmd_info struct that contains zcl header info
/// @param pin_code pointer to the pin code octect string
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_door_lock_server_lock_door_cb(zb_zcl_parsed_hdr_t *cmd_info,
                                           zb_uint8_t          *pin_code);

/// @brief Callback that user can declare to handle door lock cluster unlock door command
/// @param cmd_info struct that contains zcl header info
/// @param pin_code pointer to the pin code octect string
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_door_lock_server_unlock_door_cb(zb_zcl_parsed_hdr_t *cmd_info,
                                             zb_uint8_t          *pin_code);

/// @brief Callback that user can declare to handle door lock cluster set pin command
/// @param cmd_info    struct that contains zcl header info
/// @param user_id     id of the user for this pin code
/// @param user_status status for this pin code
/// @param user_type   type for this pin code
/// @param pin_code    pointer to the pin code
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_door_lock_server_set_pin_cb(zb_zcl_parsed_hdr_t *cmd_info,
                                         zb_uint16_t         user_id,
                                         zb_uint8_t          user_status,
                                         zb_uint8_t          user_type,
                                         zb_uint8_t          *pin_code);

/// @brief Callback that user can declare to handle door lock cluster get pin command
/// @param cmd_info struct that contains zcl header info
/// @param user_id  user id for pin to return
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_door_lock_server_get_pin_cb(zb_zcl_parsed_hdr_t *cmd_info,
                                         zb_uint16_t         user_id);

/// @brief Callback that user can declare to handle door lock cluster clear pin command
/// @param cmd_info struct that contains zcl header info
/// @param user_id  user id for pin to clear
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_door_lock_server_clear_pin_cb(zb_zcl_parsed_hdr_t *cmd_info,
                                           zb_uint16_t         user_id);

/// @brief Callback that user can declare to handle door lock cluster clear all pins command
/// @param cmd_info struct that contains zcl header info
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_door_lock_server_clear_all_pins_cb(zb_zcl_parsed_hdr_t *cmd_info);

/// @brief Callback that user can declare to handle door lock cluster set user status command
/// @param cmd_info    struct that contains zcl header info
/// @param user_id     user id for status to set
/// @param user_status status to set
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_door_lock_server_set_user_status_cb(zb_zcl_parsed_hdr_t *cmd_info,
                                                 zb_uint16_t         user_id,
                                                 zb_uint8_t          user_status);

/// @brief Callback that user can declare to handle door lock cluster get user status command
/// @param cmd_info struct that contains zcl header info
/// @param user_id  user id for status to get
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_door_lock_server_get_user_status_cb(zb_zcl_parsed_hdr_t *cmd_info,
                                                 zb_uint16_t         user_id);

/// @brief Callback that user can declare to handle door lock cluster set user type command
/// @param cmd_info  struct that contains zcl header info
/// @param user_id   user id for type to set
/// @param user_type type to set
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_door_lock_server_set_user_type_cb(zb_zcl_parsed_hdr_t *cmd_info,
                                               zb_uint16_t         user_id,
                                               zb_uint8_t          user_type);

/// @brief Callback that user can declare to handle door lock cluster get user type command
/// @param cmd_info struct that contains zcl header info
/// @param user_id  user id for type to get
/// @return ZB_FALSE to allow framework to continue processing the command
zb_bool_t tr_door_lock_server_get_user_type_cb(zb_zcl_parsed_hdr_t *cmd_info,
                                               zb_uint16_t         user_id);

/// @brief Callback fires when a door lock server attribute is about to be written
/// @param endpoint   device endpoint
/// @param attr_id    ZCL attribute id
/// @param new_value  pointer to the new attribute value
/// @param manuf_code manufacturer specific code
void tr_door_lock_server_write_attr_cb(zb_uint8_t  endpoint,
                                       zb_uint16_t attr_id,
                                       zb_uint8_t  *new_value,
                                       zb_uint16_t manuf_code);

/// @} // end of zcl_app_callbacks

/// ****************************************************************************
/// @defgroup zcl_door_lock_server_apis Door Lock Server APIs
/// @ingroup zcl_api_references
/// @{
/// ****************************************************************************

/// @brief API used to notify the door lock server plugin of lock state changes
/// @param endpoint device endpoint
/// @param lock_state new ZCL lock state
/// @param source ZCL source for change
/// @param user user id
/// @param pin_code user pin code
void tr_door_lock_server_update_lock_state(zb_uint8_t  endpoint,
                                           zb_uint8_t  lock_state,
                                           zb_uint8_t  source,
                                           zb_uint16_t user,
                                           zb_uint8_t  *pin_code);

/// @brief API used to send an operation event notification
/// @param endpoint device endpoint
/// @param source ZCL source of operation event notification
/// @param code ZCL operation event code
/// @param user user id
/// @param pin user pin code
/// @param mask_bit bit to compare against the event mask attribute
void tr_door_lock_server_send_op_event_notification(zb_uint8_t  endpoint,
                                                    zb_uint8_t  source,
                                                    zb_uint8_t  code,
                                                    zb_uint16_t user,
                                                    zb_uint8_t  *pin,
                                                    zb_uint8_t  mask_bit);

/// @brief API for getting the auto-relock time
/// @param endpoint
/// @return Auto-relock time in seconds. 0 indicates auto-relock is disabled
zb_uint32_t tr_get_auto_relock_time_seconds(zb_uint8_t endpoint);

/// @brief API used to verify a PIN code
/// @param endpoint device endpoint
/// @param rf_operation ZB_TRUE if this is an RF operation, ZB_FALSE is manual
/// @param pin_code pointer to ZCL octet string (length prefixed) pin code in ASCII
/// @param user pointer used to return the user associated with the pin_code. only valid if return is ZB_TRUE
/// @return ZB_TRUE if user is valid, ZB_FALSE if not
zb_bool_t tr_door_lock_server_verify_user(zb_uint8_t  endpoint,
                                          zb_bool_t   rf_operation,
                                          zb_char_t   *pin_code,
                                          zb_uint16_t *user);

/// @brief API used to add a user to the table
/// @param user index of user table entry
/// @param status ZCL user status
/// @param type ZCL user type
/// @param pin_code pointer to ZCL octect string (length prefixed) pin code in ASCII
/// @return ZCL set pin status code
tr_door_lock_set_pin_or_id_status_t tr_door_lock_server_add_user(zb_uint16_t                user,
                                                                 tr_door_lock_user_status_t status,
                                                                 tr_door_lock_user_type_t   type,
                                                                 zb_char_t                  *pin_code);

/// @brief API used to get a user table entry
/// @param user index of user table entry
/// @param user_info pointer to ZCL user table entry
/// @return ZB_TRUE if successful, ZB_FALSE if not
zb_bool_t tr_door_lock_server_get_user(zb_uint16_t                        user,
                                       tr_door_lock_server_pin_code_arg_t *user_info);

/// @brief API to delete an entry from the user table
/// @param user index of user table entry
/// @return ZB_TRUE if successful, ZB_FALSE if not
zb_bool_t tr_door_lock_server_delete_user(zb_uint16_t user);

/// @brief API to delete ALL entries from the user table
void tr_door_lock_server_delete_all_pin_users(void);

/// @brief API to set the status for a user in the user able
/// @param user index of user table entry
/// @param user_status ZCL user status
/// @return ZB_TRUE if successful, ZB_FALSE if not
zb_bool_t tr_door_lock_server_set_user_status(zb_uint16_t user,
                                              zb_uint8_t  user_status);

/// @brief API to get the status of a user in the user table
/// @param user index of user table entry
/// @param user_info pointer for returned user status info
/// @return ZB_TRUE if successful, ZB_FALSE if not
zb_bool_t tr_door_lock_server_get_user_status(zb_uint16_t                           user,
                                              tr_door_lock_server_user_status_arg_t *user_info);

/// @brief API to set the type of a user in the user table
/// @param user index of user table entry
/// @param user_type ZCL user type
/// @return ZB_TRUE if successful, ZB_FALSE if not
zb_bool_t tr_door_lock_server_set_user_type(zb_uint16_t user,
                                            zb_uint8_t  user_type);

/// @brief API to get the type of a user from the user table
/// @param user index of user table entry
/// @param user_info pointer for returned user type info
/// @return ZB_TRUE if successful, ZB_FALSE if not
zb_bool_t tr_door_lock_server_get_user_type(zb_uint16_t                         user,
                                            tr_door_lock_server_user_type_arg_t *user_info);

/// @brief API to print the user table
void tr_door_lock_server_print_users(void);

/// @} // end of zcl_api_references

/// ****************************************************************************
/// function prototypes consumed by framework
/// ****************************************************************************
void tr_door_lock_server_init(void);

// user management functions
void tr_door_lock_server_users_init(void);

#endif // TR_DOOR_LOCK_SERVER_H
