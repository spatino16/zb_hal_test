/// ****************************************************************************
/// @file tr_door_lock_server_users.c
///
/// @brief TODO - documentation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_af.h"
#include "tr_door_lock_server.h"

// TODO: LCD 2/4/25 these need to get stored in nvram
tr_door_lock_server_user_t pin_users[TR_DOOR_LOCK_SERVER_MAX_NUM_USERS];

static zb_bool_t  in_lockout             = ZB_FALSE;
static zb_uint8_t wrong_code_entry_count = 0;

// API to add a pin user
tr_door_lock_set_pin_or_id_status_t tr_door_lock_server_add_user(zb_uint16_t                user,
                                                                 tr_door_lock_user_status_t status,
                                                                 tr_door_lock_user_type_t   type,
                                                                 zb_char_t                  *pin_code)
{
    zb_uint8_t i;

    // make sure the user is within range
    if (user >= TR_DOOR_LOCK_SERVER_MAX_NUM_USERS)
    {
        return TR_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_MEMORY_FULL;
    }

// ZCL8 says to overwrite an existing user so take this out for now
#if 0

    // make sure the specified user entry is empty
    if (pin_users[user].status != TR_ZCL_DOOR_LOCK_USER_STATUS_AVAILABLE)
    {
        // the requested entry is already in use
        return ZB_FALSE;
    }
#endif

    // make sure this isn't a duplicate code
    for (i = 0 ; i < TR_DOOR_LOCK_SERVER_MAX_NUM_USERS ; i++)
    {
        if (memcmp(pin_code, pin_users[i].code, pin_code[0] + 1) == 0)
        {
            // this is a duplicate code, return error
            return TR_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_DUPLICATE_CODE_ERROR;
        }
    }

    pin_users[user].status = status;
    pin_users[user].type   = type;
    memcpy(pin_users[user].code, pin_code, pin_code[0] + 1);
    return TR_ZCL_DOOR_LOCK_SET_PIN_OR_ID_STATUS_SUCCESS;
}

// API to get a pin user
zb_bool_t tr_door_lock_server_get_user(zb_uint16_t                        user,
                                       tr_door_lock_server_pin_code_arg_t *user_info)
{
    if (user >= TR_DOOR_LOCK_SERVER_MAX_NUM_USERS)
    {
        return ZB_FALSE;
    }

    user_info->user_id     = user;
    user_info->user_status = pin_users[user].status;
    user_info->user_type   = pin_users[user].type;
    memcpy(&user_info->pin_code, pin_users[user].code, TR_DOOR_LOCK_SERVER_MAX_PIN_LEN + 1);
    return ZB_TRUE;
}

// API to delete a pin user
zb_bool_t tr_door_lock_server_delete_user(zb_uint16_t user)
{
    // make sure the user is within range
    if (user >= TR_DOOR_LOCK_SERVER_MAX_NUM_USERS)
    {
        return ZB_FALSE;
    }

    // set the entry to defaults
    pin_users[user].status = TR_ZCL_DOOR_LOCK_USER_STATUS_AVAILABLE;
    pin_users[user].type   = TR_ZCL_DOOR_LOCK_USER_TYPE_NOT_SUPPORTED;
    memset(pin_users[user].code, 0, TR_DOOR_LOCK_SERVER_MAX_PIN_LEN + 1);
    return ZB_TRUE;
}

// API to delete all pin users
void tr_door_lock_server_delete_all_pin_users(void)
{
    zb_uint8_t i;

    // set the tables to default values
    for (i = 0 ; i < TR_DOOR_LOCK_SERVER_MAX_NUM_USERS ; i++)
    {
        pin_users[i].status = TR_ZCL_DOOR_LOCK_USER_STATUS_AVAILABLE;
        pin_users[i].type   = TR_ZCL_DOOR_LOCK_USER_TYPE_NOT_SUPPORTED;
        memset(pin_users[i].code, 0, TR_DOOR_LOCK_SERVER_MAX_PIN_LEN + 1);
    }
}

// API to set a user status
zb_bool_t tr_door_lock_server_set_user_status(zb_uint16_t user,
                                              zb_uint8_t  user_status)
{
    if ((user >= TR_DOOR_LOCK_SERVER_MAX_NUM_USERS) ||
        (user_status == 0))
    {
        return ZB_FALSE;
    }

    pin_users[user].status = user_status;
    return ZB_TRUE;
}

// API to get a user status
zb_bool_t tr_door_lock_server_get_user_status(zb_uint16_t                           user,
                                              tr_door_lock_server_user_status_arg_t *user_info)
{
    if (user >= TR_DOOR_LOCK_SERVER_MAX_NUM_USERS)
    {
        return ZB_FALSE;
    }

    user_info->user_id     = user;
    user_info->user_status = pin_users[user].status;
    return ZB_TRUE;
}

// API to set a user type
zb_bool_t tr_door_lock_server_set_user_type(zb_uint16_t user,
                                            zb_uint8_t  user_type)
{
    if (user >= TR_DOOR_LOCK_SERVER_MAX_NUM_USERS)
    {
        return ZB_FALSE;
    }

    pin_users[user].type = user_type;
    return ZB_TRUE;
}

// API to get a user type
zb_bool_t tr_door_lock_server_get_user_type(zb_uint16_t                         user,
                                            tr_door_lock_server_user_type_arg_t *user_info)
{
    if (user >= TR_DOOR_LOCK_SERVER_MAX_NUM_USERS)
    {
        return ZB_FALSE;
    }

    user_info->user_id   = user;
    user_info->user_type = pin_users[user].type;
    return ZB_TRUE;
}

// event used to clear out the wrong code entry lockout state
void wrong_code_timeout_handler(zb_uint8_t param)
{
    ZVUNUSED(param);
    in_lockout             = 0;
    wrong_code_entry_count = 0;
}

// API to verify a pin user code
zb_bool_t tr_door_lock_server_verify_user(zb_uint8_t  endpoint,
                                          zb_bool_t   rf_operation,
                                          zb_char_t   *pin_code,
                                          zb_uint16_t *user)

{
    zb_bool_t     require_pin = ZB_FALSE;
    zb_zcl_attr_t *attr_desc;
    zb_uint8_t    i;
    zb_zcl_attr_t *entry_limit_attr_desc;
    zb_uint8_t    wrong_code_entry_limit = 0;
    zb_zcl_attr_t *disable_time_attr_desc;
    zb_uint8_t    disable_time_seconds = 0;

    // if this is an rf operation, does it require a pin (attribute exists and is true)
    if (rf_operation)
    {
        // get the require pin for rf operation attribute
        attr_desc = zb_zcl_get_attr_desc_a(
            endpoint,
            TR_ZCL_CLUSTER_DOOR_LOCK_ID,
            TR_ZCL_CLUSTER_SERVER_ROLE,
            TR_ZCL_ATTR_DOOR_LOCK_REQUIRE_PIN_FOR_RF_OPERATION_ID);

        if (attr_desc)
        {
            // get the require pin setting
            require_pin = *(bool_t*)attr_desc->data_p;
        }

        if (!require_pin)
        {
            // no pin is required for an rf operation, return true, user is valid
            return ZB_TRUE;
        }
    }

    // get wrong entry limit attribute
    entry_limit_attr_desc = zb_zcl_get_attr_desc_a(
        endpoint,
        TR_ZCL_CLUSTER_DOOR_LOCK_ID,
        TR_ZCL_CLUSTER_SERVER_ROLE,
        TR_ZCL_ATTR_DOOR_LOCK_WRONG_CODE_ENTRY_LIMIT_ID);

    if (entry_limit_attr_desc != NULL)
    {
        wrong_code_entry_limit = *(zb_uint8_t*)entry_limit_attr_desc->data_p;
    }

    // get disable time attribute
    disable_time_attr_desc = zb_zcl_get_attr_desc_a(
        endpoint,
        TR_ZCL_CLUSTER_DOOR_LOCK_ID,
        TR_ZCL_CLUSTER_SERVER_ROLE,
        TR_ZCL_ATTR_DOOR_LOCK_USER_CODE_TEMPORARY_DISABLE_TIME_ID);

    if (disable_time_attr_desc != NULL)
    {
        disable_time_seconds = *(zb_uint8_t*)disable_time_attr_desc->data_p;
    }

    // go ahead and cancel any pending wrong code alarms, it will be rescheduled if needed
    ZB_SCHEDULE_APP_ALARM_CANCEL(wrong_code_timeout_handler, ZB_ALARM_ALL_CB);

    // a pin is required, look for the specified one in the table
    for (i = 0 ; i < TR_DOOR_LOCK_SERVER_MAX_NUM_USERS ; i++)
    {
        tr_door_lock_server_user_t *pin_entry = &pin_users[i];

        // compare the passed in pin_code with the one in the table, including the length byte
        // if the length of the entry in the table is 0, then skip it
        if ((pin_entry->code[0] != 0) && (memcmp(pin_code, pin_entry->code, pin_code[0] + 1) == 0))
        {
            // we have a match, are we in lockout?
            if (in_lockout)
            {
                // we are in lockout, restart the lockout timer and return ZB_FALSE
                ZB_SCHEDULE_APP_ALARM(wrong_code_timeout_handler, 0, ZB_SECONDS_TO_BEACON_INTERVAL(disable_time_seconds));
                return ZB_FALSE;
            }
            // we have a match, we are not in lockout, return it!!!
            ZB_SCHEDULE_APP_ALARM_CANCEL(wrong_code_timeout_handler, ZB_ALARM_ALL_CB);
            wrong_code_entry_count = 0;
            *user                  = i;
            return ZB_TRUE;
        }
    }

    // if we get here, pin code was invalid
    // check for wrong code entry limit support
    if ((wrong_code_entry_limit != 0) && (disable_time_seconds != 0))
    {
        // increment the wrong code entry count and see if we need to go into lockout
        if (++wrong_code_entry_count >= wrong_code_entry_limit)
        {
            wrong_code_entry_count = wrong_code_entry_limit;
            in_lockout             = ZB_TRUE;
            ZB_SCHEDULE_APP_ALARM(wrong_code_timeout_handler, 0, ZB_SECONDS_TO_BEACON_INTERVAL(disable_time_seconds));
        }
    }
    return ZB_FALSE;
}

// API to print the user table
void tr_door_lock_server_print_users(void)
{
    zb_uint8_t i;
    zb_uint8_t j;

    tr_door_lock_server_printf("User  Status  Type  Len  Code\n");

    for (i = 0 ; i < TR_DOOR_LOCK_SERVER_MAX_NUM_USERS ; i++)
    {
        tr_door_lock_server_printf(" %2d     %3d    %3d   %2d   ", i, pin_users[i].status, pin_users[i].type, pin_users[i].code[0]);

        for (j = 1 ; j <= pin_users[i].code[0] ; j++)
        {
            tr_door_lock_server_printf("%c", pin_users[i].code[j]);
        }
        tr_door_lock_server_printf("\n");
    }
}

void tr_door_lock_server_users_init(void)
{
    // set the tables to default values to start
    tr_door_lock_server_delete_all_pin_users();
}
