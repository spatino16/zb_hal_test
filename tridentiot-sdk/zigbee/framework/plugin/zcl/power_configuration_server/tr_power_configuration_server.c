/// ****************************************************************************
/// @file tr_power_configuration_server.c
///
/// @brief ZCL POWER CONFIGURATION cluster server implementation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_power_configuration_server.h"
#include "zb_zcl_diagnostics.h"

#ifdef TR_POWER_CONFIGURATION_SERVER_BATTERY_LATCHING
static zb_bool_t g_power_configuration_server_unlatch_flag[3] = { ZB_FALSE, ZB_FALSE, ZB_FALSE };
#endif

#define PLUGIN_NAME (zb_char_t*)("Power Configuration Server")

#ifdef POWER_CONFIGURATION_SERVER_SUPPORTED_COMMANDS_RECEIVE
static zb_uint8_t gs_power_configuration_server_received_commands[] =
{
    POWER_CONFIGURATION_SERVER_SUPPORTED_COMMANDS_RECEIVE
};
#endif

#ifdef POWER_CONFIGURATION_SERVER_SUPPORTED_COMMANDS_SEND
static zb_uint8_t gs_power_configuration_server_generated_commands[] =
{
    POWER_CONFIGURATION_SERVER_SUPPORTED_COMMANDS_SEND
};
#endif

static zb_discover_cmd_list_t gs_power_configuration_server_cmd_list =
{
#ifdef POWER_CONFIGURATION_SERVER_SUPPORTED_COMMANDS_RECEIVE
    sizeof(gs_power_configuration_server_received_commands),  gs_power_configuration_server_received_commands,
#else
    0,                                                        NULL,
#endif
#ifdef POWER_CONFIGURATION_SERVER_SUPPORTED_COMMANDS_SEND
    sizeof(gs_power_configuration_server_generated_commands), gs_power_configuration_server_generated_commands
#else
    0,                                                        NULL
#endif
};

// get the value of a power configuration cluster u8 bit attribute, and return the default
// value if the attribute does not exist
static zb_uint8_t get_pwr_cfg_u8_attr_val(zb_uint16_t attr_id,
                                          zb_uint8_t  endpoint,
                                          zb_uint8_t  def_value)
{
    zb_zcl_attr_t *attr_desc;

    attr_desc = zb_zcl_get_attr_desc_a(
        endpoint,
        TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID,
        TR_ZCL_CLUSTER_SERVER_ROLE,
        attr_id);

    if (attr_desc)
    {
        return (ZB_ZCL_GET_ATTRIBUTE_VAL_8(attr_desc));
    }
    return def_value;
}

// get the value of a power configuration cluster u16 bit attribute, and return the default
// value if the attribute does not exist
static zb_uint16_t get_pwr_cfg_u16_attr_val(zb_uint16_t attr_id,
                                            zb_uint8_t  endpoint,
                                            zb_uint16_t def_value)
{
    zb_zcl_attr_t *attr_desc;

    attr_desc = zb_zcl_get_attr_desc_a(
        endpoint,
        TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID,
        TR_ZCL_CLUSTER_SERVER_ROLE,
        attr_id);

    if (attr_desc)
    {
        return (ZB_ZCL_GET_ATTRIBUTE_VAL_16(attr_desc));
    }
    return def_value;
}

// get the value of a power configuration cluster u32 bit attribute, and return the default
// value if the attribute does not exist
static zb_uint32_t get_pwr_cfg_u32_attr_val(zb_uint16_t attr_id,
                                            zb_uint8_t  endpoint,
                                            zb_uint32_t def_value)
{
    zb_zcl_attr_t *attr_desc;

    attr_desc = zb_zcl_get_attr_desc_a(
        endpoint,
        TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID,
        TR_ZCL_CLUSTER_SERVER_ROLE,
        attr_id);

    if (attr_desc)
    {
        return (ZB_ZCL_GET_ATTRIBUTE_VAL_32(attr_desc));
    }
    return def_value;
}

// check the new value of a battery threshold value against the next higher threshold
static zb_ret_t check_value_against_higher_thresh(zb_uint16_t attr_id,
                                                  zb_uint8_t  endpoint,
                                                  zb_uint8_t  *value)
{
    zb_uint8_t thresh;

    // make sure this value is less than the next higher threshold
    thresh = get_pwr_cfg_u8_attr_val(attr_id + 1,
                                     endpoint,
                                     0xFF);

    if ((thresh != 0xFF) &&
        (*(zb_uint8_t*)value >= thresh))
    {
        return RET_ERROR;
    }
    return RET_OK;
}

// check the new value of a battery threshold value against the next lower threshold
static zb_ret_t check_value_against_lower_thresh(zb_uint16_t attr_id,
                                                 zb_uint8_t  endpoint,
                                                 zb_uint8_t  *value)
{
    zb_uint8_t thresh;

    // make sure this value is greater than the next lower threshold
    thresh = get_pwr_cfg_u8_attr_val(attr_id - 1,
                                     endpoint,
                                     0xFF);

    if ((thresh != 0xFF) &&
        (*(zb_uint8_t*)value <= thresh))
    {
        return RET_ERROR;
    }
    return RET_OK;
}

// Check the value being written to an attribute
static zb_ret_t power_configuration_server_check_value(zb_uint16_t attr_id,
                                                       zb_uint8_t  endpoint,
                                                       zb_uint8_t  *value)
{
    zb_ret_t ret_val = RET_OK;

    switch (attr_id)
    {
        case TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_VOLTAGE_MIN_THRESHOLD_ID:
        {
            // make sure this value is lower than the max threshold attr
            zb_uint16_t max_threshold;

            max_threshold = get_pwr_cfg_u16_attr_val(TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_VOLTAGE_MAX_THRESHOLD_ID,
                                                     endpoint,
                                                     ZB_ZCL_POWER_CONFIG_THRESHOLD_ALARM_OMISSION_VALUE);

            if ((*(zb_uint16_t*)value >= max_threshold) &&
                (max_threshold != ZB_ZCL_POWER_CONFIG_THRESHOLD_ALARM_OMISSION_VALUE))
            {

                ret_val = RET_ERROR;
            }
            break;
        }

        case TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_VOLTAGE_MAX_THRESHOLD_ID:
        {
            // make sure this value is greater than the min threshold attr
            zb_uint16_t min_threshold;

            min_threshold = get_pwr_cfg_u16_attr_val(TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_VOLTAGE_MIN_THRESHOLD_ID,
                                                     endpoint,
                                                     ZB_ZCL_POWER_CONFIG_THRESHOLD_ALARM_OMISSION_VALUE);

            if ((*(zb_uint16_t*)value <= min_threshold) &&
                (min_threshold != ZB_ZCL_POWER_CONFIG_THRESHOLD_ALARM_OMISSION_VALUE))
            {
                ret_val = RET_ERROR;
            }
            break;
        }

        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_VOLTAGE_MIN_THRESHOLD_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_VOLTAGE_MIN_THRESHOLD_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_VOLTAGE_MIN_THRESHOLD_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_PERCENTAGE_MIN_THRESHOLD_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_PERCENTAGE_MIN_THRESHOLD_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_PERCENTAGE_MIN_THRESHOLD_ID:

        {
            // make sure this value is less than the threshold 1 attr
            ret_val = check_value_against_higher_thresh(attr_id, endpoint, value);
            break;
        }

        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_VOLTAGE_THRESHOLD_1_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_VOLTAGE_THRESHOLD_1_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_VOLTAGE_THRESHOLD_1_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_VOLTAGE_THRESHOLD_2_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_VOLTAGE_THRESHOLD_2_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_VOLTAGE_THRESHOLD_2_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_PERCENTAGE_THRESHOLD_1_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_PERCENTAGE_THRESHOLD_1_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_PERCENTAGE_THRESHOLD_1_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_PERCENTAGE_THRESHOLD_2_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_PERCENTAGE_THRESHOLD_2_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_PERCENTAGE_THRESHOLD_2_ID:
        {
            // make sure this value is greater than the next lower threshold attr
            ret_val = check_value_against_lower_thresh(attr_id, endpoint, value);

            if (RET_OK == ret_val)
            {
                // make sure this value is greater than the next higher threshold attr
                ret_val = check_value_against_higher_thresh(attr_id, endpoint, value);
            }
            break;
        }

        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_VOLTAGE_THRESHOLD_3_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_VOLTAGE_THRESHOLD_3_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_VOLTAGE_THRESHOLD_3_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_PERCENTAGE_THRESHOLD_3_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_PERCENTAGE_THRESHOLD_3_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_PERCENTAGE_THRESHOLD_3_ID:
        {
            // make sure this value is less than the threshold 2 attr
            ret_val = check_value_against_lower_thresh(attr_id, endpoint, value);
            break;
        }
    }

    return ret_val;
}

// send an alarm for mains or battery.
// any necessary dwell time for the mains has elapsed, make sure it is still valid
void send_alarm(zb_uint8_t param)
{
    zb_uint16_t *ep_and_alarm_code = ZB_BUF_GET_PARAM(param, zb_uint16_t);
    zb_uint8_t  endpoint           = *ep_and_alarm_code >> 8;
    zb_uint8_t  alarm_code         = *ep_and_alarm_code & 0xFF;
    zb_bool_t   ret                = ZB_FALSE;

    switch (alarm_code)
    {
        case ZB_ZCL_POWER_CONFIG_MAINS_VOLTAGE_ALARM_CODE_MIN_THRESHOLD:
        {
            zb_uint16_t mains_voltage;
            zb_uint16_t mains_min_thresh;

            // make sure the mains voltage is still below the min threshold
            mains_voltage = get_pwr_cfg_u16_attr_val(TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_VOLTAGE_ID,
                                                     endpoint,
                                                     ZB_ZCL_POWER_CONFIG_THRESHOLD_ALARM_OMISSION_VALUE);
            mains_min_thresh = get_pwr_cfg_u16_attr_val(TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_VOLTAGE_MIN_THRESHOLD_ID,
                                                        endpoint,
                                                        ZB_ZCL_POWER_CONFIG_THRESHOLD_ALARM_OMISSION_VALUE);

            if ((mains_min_thresh != ZB_ZCL_POWER_CONFIG_THRESHOLD_ALARM_OMISSION_VALUE) &&
                (mains_voltage != ZB_ZCL_POWER_CONFIG_THRESHOLD_ALARM_OMISSION_VALUE) &&
                (mains_voltage < mains_min_thresh))
            {
                // send the mains below low threshold alarm
                ret = ZB_TRUE;
            }
            break;
        }

        case ZB_ZCL_POWER_CONFIG_MAINS_VOLTAGE_ALARM_CODE_MAX_THRESHOLD:
        {
            zb_uint16_t mains_voltage;
            zb_uint16_t mains_max_thresh;

            // make sure the mains voltage is still above the max threshold
            mains_voltage = get_pwr_cfg_u16_attr_val(TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_VOLTAGE_ID,
                                                     endpoint,
                                                     ZB_ZCL_POWER_CONFIG_THRESHOLD_ALARM_OMISSION_VALUE);
            mains_max_thresh = get_pwr_cfg_u16_attr_val(TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_VOLTAGE_MAX_THRESHOLD_ID,
                                                        endpoint,
                                                        ZB_ZCL_POWER_CONFIG_THRESHOLD_ALARM_OMISSION_VALUE);

            if ((mains_max_thresh != ZB_ZCL_POWER_CONFIG_THRESHOLD_ALARM_OMISSION_VALUE) &&
                (mains_voltage != ZB_ZCL_POWER_CONFIG_THRESHOLD_ALARM_OMISSION_VALUE) &&
                (mains_voltage > mains_max_thresh))
            {
                // send the mains below low threshold alarm
                ret = ZB_TRUE;
            }
            break;
        }

        default:
            ret = ZB_TRUE;
            break;
    }

    if (ret)
    {
        zb_uint16_t addr = 0;

        if (!tr_power_configuration_server_pre_alarm_send_cb(endpoint, alarm_code))
        {
            ZB_ZCL_ALARMS_SEND_ALARM_RES(
                param,
                addr,
                ZB_APS_ADDR_MODE_DST_ADDR_ENDP_NOT_PRESENT,
                0,
                endpoint,
                ZB_AF_HA_PROFILE_ID,
                NULL,
                alarm_code,
                TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID);
        }
    }
    else
    {
        zb_buf_free(param);
    }
}

void get_send_alarm_buf(zb_uint8_t  param,
                        zb_uint16_t user_param)
{
    zb_uint8_t endpoint   = user_param >> 8;
    zb_uint8_t alarm_code = user_param & 0xFF;

    // verify that this device supports the alarms cluster as a server
    zb_af_endpoint_desc_t *endpoint_desc = zb_af_get_endpoint_desc(endpoint);
    zb_zcl_cluster_desc_t *cluster_desc  = get_cluster_desc(endpoint_desc, TR_ZCL_CLUSTER_ALARMS_ID, TR_ZCL_CLUSTER_SERVER_ROLE);

    if (!cluster_desc)
    {
        // the device does not have the alarms cluster server, no need to do anymore
        return;
    }

    if (!param)
    {
        zb_buf_get_out_delayed_ext(get_send_alarm_buf, user_param, 0);
    }
    else
    {
        zb_uint16_t dwell_period_seconds = 0;
        zb_uint16_t *u_param             = ZB_BUF_GET_PARAM(param, zb_uint16_t);

        // get the mains dwell trip point if this is a mains alarm
        if (alarm_code < ZB_ZCL_POWER_CONFIG_BATTERY_ALARM_CODE_SOURCE1_MIN_THRESHOLD)
        {
            dwell_period_seconds = get_pwr_cfg_u16_attr_val(TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_VOLTAGE_DWELL_TRIP_POINT_ID,
                                                            endpoint,
                                                            0);
        }

        *u_param = user_param;
        zb_schedule_alarm(send_alarm, param, ZB_TIME_ONE_SECOND * dwell_period_seconds);
    }
}

void check_mains_voltage_for_alarm(zb_uint8_t  endpoint,
                                   zb_uint16_t attr_id,
                                   zb_uint8_t  *new_value)
{
    zb_uint8_t  mains_alarm_mask;
    zb_uint16_t mains_voltage;
    zb_uint16_t mains_min_thresh;
    zb_uint16_t mains_max_thresh;

    // read the current attributes
    mains_alarm_mask = get_pwr_cfg_u8_attr_val(TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_ALARM_MASK_ID,
                                               endpoint,
                                               0);
    mains_voltage = get_pwr_cfg_u16_attr_val(TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_VOLTAGE_ID,
                                             endpoint,
                                             ZB_ZCL_POWER_CONFIG_THRESHOLD_ALARM_OMISSION_VALUE);
    mains_min_thresh = get_pwr_cfg_u16_attr_val(TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_VOLTAGE_MIN_THRESHOLD_ID,
                                                endpoint,
                                                ZB_ZCL_POWER_CONFIG_THRESHOLD_ALARM_OMISSION_VALUE);
    mains_max_thresh = get_pwr_cfg_u16_attr_val(TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_VOLTAGE_MAX_THRESHOLD_ID,
                                                endpoint,
                                                ZB_ZCL_POWER_CONFIG_THRESHOLD_ALARM_OMISSION_VALUE);

    // now figure out which attribute is being written and update it
    switch (attr_id)
    {
        case TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_ALARM_MASK_ID:
            mains_alarm_mask = *(zb_uint8_t*)new_value;
            break;

        case TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_VOLTAGE_ID:
            mains_voltage = *(zb_uint16_t*)new_value;
            break;

        case TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_VOLTAGE_MIN_THRESHOLD_ID:
            mains_min_thresh = *(zb_uint16_t*)new_value;
            break;

        case TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_VOLTAGE_MAX_THRESHOLD_ID:
            mains_max_thresh = *(zb_uint16_t*)new_value;
            break;
    }

    // now we have all current values in local variables, see if we need to do anything
    if ((mains_alarm_mask & TR_ZCL_MAINS_ALARM_MASK_VOLTAGE_TOO_LOW_MASK) &&
        (mains_min_thresh != ZB_ZCL_POWER_CONFIG_THRESHOLD_ALARM_OMISSION_VALUE) &&
        (mains_voltage != ZB_ZCL_POWER_CONFIG_THRESHOLD_ALARM_OMISSION_VALUE) &&
        (mains_voltage < mains_min_thresh))
    {
        // the mask is set, the min thresh is valid, and the voltage is low, schedule an alarm
        zb_uint16_t ep_and_alarm_code = (endpoint << 8) | ZB_ZCL_POWER_CONFIG_MAINS_VOLTAGE_ALARM_CODE_MIN_THRESHOLD;
        zb_buf_get_out_delayed_ext(get_send_alarm_buf, ep_and_alarm_code, 0);
    }

    if ((mains_alarm_mask & TR_ZCL_MAINS_ALARM_MASK_VOLTAGE_TOO_HIGH_MASK) &&
        (mains_max_thresh != ZB_ZCL_POWER_CONFIG_THRESHOLD_ALARM_OMISSION_VALUE) &&
        (mains_voltage != ZB_ZCL_POWER_CONFIG_THRESHOLD_ALARM_OMISSION_VALUE) &&
        (mains_voltage > mains_max_thresh))
    {
        // the mask is set, the max thresh is valid, and the voltage is high, schedule an alarm
        zb_uint16_t ep_and_alarm_code = (endpoint << 8) | ZB_ZCL_POWER_CONFIG_MAINS_VOLTAGE_ALARM_CODE_MAX_THRESHOLD;
        zb_buf_get_out_delayed_ext(get_send_alarm_buf, ep_and_alarm_code, 0);
    }
}

void check_batt_value_for_alarm(zb_uint8_t  endpoint,
                                zb_uint16_t attr_id,
                                zb_uint8_t  *new_value)
{
    zb_uint8_t  alarm_code = 0;
    zb_uint8_t  batt_value;           // battery value (volt or percent)
    zb_uint8_t  batt_alarm_mask;
    zb_uint8_t  batt_thresh;          // lowest threshold (volt or percent)
    zb_uint8_t  batt_thresh1;         // threshold1 (volt or percent)
    zb_uint8_t  batt_thresh2;         // threshold2 (volt or percent)
    zb_uint8_t  batt_thresh3;         // threshold3 (volt or percent)
    zb_uint8_t  batt_volt_or_percent; // 0 for voltage, 1 for percent
    zb_uint8_t  start = 0;
    zb_uint8_t  end   = 0;
    zb_uint32_t batt_alarm_state;
    zb_uint32_t tmp_alarm_state = 0;
    zb_uint16_t attr_set_offset;          // offset to battery source 1, 2 or 3 attributes

    // start by figuring out which battery attribute set we are using
    attr_set_offset = (attr_id & 0xFFE0) - 0x0020;

    // did the alarm mask change? if so, we have to check both voltage and percent
    if (((attr_id - attr_set_offset) == TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_ALARM_MASK_ID))
    {
        // the alarm mask changed, set it to the new value
        batt_alarm_mask = *(zb_uint8_t*)new_value;

        // and we need to check voltages and percentages
        start = 0;
        end   = 1;
    }
    else
    {
        // the alarm mask didn't change, read its current value
        batt_alarm_mask = get_pwr_cfg_u8_attr_val(TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_ALARM_MASK_ID + attr_set_offset,
                                                  endpoint,
                                                  0x00);

        // now, are we dealing with voltage or percent?
        if (((attr_id - attr_set_offset) == TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_PERCENTAGE_REMAINING_ID) ||
            (((attr_id - attr_set_offset) >= TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_PERCENTAGE_MIN_THRESHOLD_ID) &&
             ((attr_id - attr_set_offset) <= TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_PERCENTAGE_THRESHOLD_3_ID)))
        {
            // this is a battery percent remaining or percent threshold attribute
            start = 1;
            end   = 1;
        }
    }

    // read the battery alarm state attribute. Note, this attribute exists for battery source 1, 2, and 3
    // but all 3 attributes, if they exist, should be mirrors of each other. We use the one for battery
    // source 1 as the master.
    batt_alarm_state = get_pwr_cfg_u32_attr_val(TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_ALARM_STATE_ID,
                                                endpoint,
                                                0x00000000);

    for (batt_volt_or_percent = start ; batt_volt_or_percent <= end ; batt_volt_or_percent++)
    {
        // read battery voltage or percent remaining attribute
        batt_value = get_pwr_cfg_u8_attr_val(TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_VOLTAGE_ID + batt_volt_or_percent + attr_set_offset,
                                             endpoint,
                                             0xFF);

        // read the threshold attributes
        batt_thresh = get_pwr_cfg_u8_attr_val(TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_VOLTAGE_MIN_THRESHOLD_ID + (batt_volt_or_percent * 4) + attr_set_offset,
                                              endpoint,
                                              0);
        batt_thresh1 = get_pwr_cfg_u8_attr_val(TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_VOLTAGE_THRESHOLD_1_ID + (batt_volt_or_percent * 4) + attr_set_offset,
                                               endpoint,
                                               0);
        batt_thresh2 = get_pwr_cfg_u8_attr_val(TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_VOLTAGE_THRESHOLD_2_ID + (batt_volt_or_percent * 4) + attr_set_offset,
                                               endpoint,
                                               0);
        batt_thresh3 = get_pwr_cfg_u8_attr_val(TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_VOLTAGE_THRESHOLD_3_ID + (batt_volt_or_percent * 4) + attr_set_offset,
                                               endpoint,
                                               0);

        // update the value for the attribute being written
        switch (attr_id - attr_set_offset)
        {
            case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_VOLTAGE_ID:
            case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_PERCENTAGE_REMAINING_ID:
                batt_value = *(zb_uint8_t*)new_value;
                break;

            case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_VOLTAGE_MIN_THRESHOLD_ID:
            case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_PERCENTAGE_MIN_THRESHOLD_ID:
                batt_thresh = *(zb_uint8_t*)new_value;
                break;

            case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_VOLTAGE_THRESHOLD_1_ID:
            case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_PERCENTAGE_THRESHOLD_1_ID:
                batt_thresh1 = *(zb_uint8_t*)new_value;
                break;

            case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_VOLTAGE_THRESHOLD_2_ID:
            case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_PERCENTAGE_THRESHOLD_2_ID:
                batt_thresh2 = *(zb_uint8_t*)new_value;
                break;

            case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_VOLTAGE_THRESHOLD_3_ID:
            case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_PERCENTAGE_THRESHOLD_3_ID:
                batt_thresh3 = *(zb_uint8_t*)new_value;
                break;
        }

        // now start checking values and the alarm mask bits
        // the order of these checks means that the lowest threshold crossing with
        // the mask bit enabled will be triggered
        alarm_code = 0;

        if (batt_value < batt_thresh3)
        {
            tmp_alarm_state |= ZB_ZCL_POWER_CONFIG_BATTERY_ALARM_STATE_SOURCE1_VOLTAGE3;

            if (batt_alarm_mask & (0x01 << ZB_ZCL_POWER_CONFIG_BATTERY_ALARM_MASK_ALARM3))
            {
                alarm_code = ZB_ZCL_POWER_CONFIG_BATTERY_ALARM_CODE_SOURCE1_VOLTAGE3;
            }
        }

        if (batt_value < batt_thresh2)
        {
            tmp_alarm_state |= ZB_ZCL_POWER_CONFIG_BATTERY_ALARM_STATE_SOURCE1_VOLTAGE2;

            if (batt_alarm_mask & (0x01 << ZB_ZCL_POWER_CONFIG_BATTERY_ALARM_MASK_ALARM2))
            {
                alarm_code = ZB_ZCL_POWER_CONFIG_BATTERY_ALARM_CODE_SOURCE1_VOLTAGE2;
            }
        }

        if (batt_value < batt_thresh1)
        {
            tmp_alarm_state |= ZB_ZCL_POWER_CONFIG_BATTERY_ALARM_STATE_SOURCE1_VOLTAGE1;

            if (batt_alarm_mask & (0x01 << ZB_ZCL_POWER_CONFIG_BATTERY_ALARM_MASK_ALARM1))
            {
                alarm_code = ZB_ZCL_POWER_CONFIG_BATTERY_ALARM_CODE_SOURCE1_VOLTAGE1;
            }
        }

        if (batt_value < batt_thresh)
        {
            tmp_alarm_state |= ZB_ZCL_POWER_CONFIG_BATTERY_ALARM_STATE_SOURCE1_MIN_THRESHOLD;

            if (batt_alarm_mask & (0x01 << ZB_ZCL_POWER_CONFIG_BATTERY_ALARM_MASK_VOLTAGE_LOW))
            {
                alarm_code = ZB_ZCL_POWER_CONFIG_BATTERY_ALARM_CODE_SOURCE1_MIN_THRESHOLD;
            }
        }

        if (alarm_code != 0)
        {
            // adjust the alarm code to match the battery set that we are working with, 1, 2, or 3
            alarm_code += (attr_set_offset >> 1);

            // send the alarm
            zb_uint16_t ep_and_alarm_code = (endpoint << 8) | alarm_code;
            zb_buf_get_out_delayed_ext(get_send_alarm_buf, ep_and_alarm_code, 0);
        }

        // adjust the temporary alarm state to match the proper battery source
        tmp_alarm_state = tmp_alarm_state << ((attr_set_offset >> 5) * 10);
    }

#ifdef TR_POWER_CONFIGURATION_SERVER_BATTERY_LATCHING

    // do not allow any bits to be cleared, that is only done by the unlatch API call
    if (g_power_configuration_server_unlatch_flag[attr_set_offset >> 5] != ZB_TRUE)
    {
        tmp_alarm_state |= batt_alarm_state & (0x0000000F << ((attr_set_offset >> 5) * 10));
    }
    else
    {
        g_power_configuration_server_unlatch_flag[attr_set_offset >> 5] = ZB_FALSE;
    }
#endif /* ifdef TR_POWER_CONFIGURATION_SERVER_BATTERY_LATCHING */

    // mask out the bits for the alarm state nibble that matches this power source
    batt_alarm_state &= ~(0x0000000F << ((attr_set_offset >> 5) * 10));

    // OR in the bits that appy now
    batt_alarm_state |= tmp_alarm_state;

    // check to see if the alarm state changed. if so, write all 3 attributes and call the callback
    if (batt_alarm_state != get_pwr_cfg_u32_attr_val(TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_ALARM_STATE_ID, endpoint, 0))
    {
        // set all 3 battery alarm state attributes if they exist
        zb_zcl_attr_t *attr_desc;

        attr_desc = zb_zcl_get_attr_desc_a(
            endpoint,
            TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID,
            TR_ZCL_CLUSTER_SERVER_ROLE,
            TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_ALARM_STATE_ID);

        if (attr_desc)
        {
            zb_zcl_set_attr_val(endpoint,
                                TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID,
                                TR_ZCL_CLUSTER_SERVER_ROLE,
                                TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_ALARM_STATE_ID,
                                (zb_uint8_t*)&batt_alarm_state,
                                ZB_FALSE);
        }

        attr_desc = zb_zcl_get_attr_desc_a(
            endpoint,
            TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID,
            TR_ZCL_CLUSTER_SERVER_ROLE,
            TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_ALARM_STATE_ID);

        if (attr_desc)
        {
            zb_zcl_set_attr_val(endpoint,
                                TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID,
                                TR_ZCL_CLUSTER_SERVER_ROLE,
                                TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_ALARM_STATE_ID,
                                (zb_uint8_t*)&batt_alarm_state,
                                ZB_FALSE);
        }

        attr_desc = zb_zcl_get_attr_desc_a(
            endpoint,
            TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID,
            TR_ZCL_CLUSTER_SERVER_ROLE,
            TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_ALARM_STATE_ID);

        if (attr_desc)
        {
            zb_zcl_set_attr_val(endpoint,
                                TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID,
                                TR_ZCL_CLUSTER_SERVER_ROLE,
                                TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_ALARM_STATE_ID,
                                (zb_uint8_t*)&batt_alarm_state,
                                ZB_FALSE);
        }

        // call the callback
        tr_power_configuration_server_battery_alarm_state_changed_cb(endpoint, batt_alarm_state);
    }
}

// process power configuration server attribute write commands
static void power_configuration_server_write_attr_hook(zb_uint8_t  endpoint,
                                                       zb_uint16_t attr_id,
                                                       zb_uint8_t  *new_value,
                                                       zb_uint16_t manuf_code)
{
    ZVUNUSED(manuf_code);

    switch (attr_id)
    {
        case TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_VOLTAGE_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_ALARM_MASK_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_VOLTAGE_MIN_THRESHOLD_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_VOLTAGE_MAX_THRESHOLD_ID:
            // check mains voltage against min and max threshold and start dwell timer if needed
            check_mains_voltage_for_alarm(endpoint, attr_id, new_value);
            break;

        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_VOLTAGE_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_PERCENTAGE_REMAINING_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_ALARM_MASK_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_VOLTAGE_MIN_THRESHOLD_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_VOLTAGE_THRESHOLD_1_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_VOLTAGE_THRESHOLD_2_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_VOLTAGE_THRESHOLD_3_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_PERCENTAGE_MIN_THRESHOLD_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_PERCENTAGE_THRESHOLD_1_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_PERCENTAGE_THRESHOLD_2_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_PERCENTAGE_THRESHOLD_3_ID:

        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_VOLTAGE_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_PERCENTAGE_REMAINING_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_ALARM_MASK_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_VOLTAGE_MIN_THRESHOLD_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_VOLTAGE_THRESHOLD_1_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_VOLTAGE_THRESHOLD_2_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_VOLTAGE_THRESHOLD_3_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_PERCENTAGE_MIN_THRESHOLD_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_PERCENTAGE_THRESHOLD_1_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_PERCENTAGE_THRESHOLD_2_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_PERCENTAGE_THRESHOLD_3_ID:

        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_VOLTAGE_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_PERCENTAGE_REMAINING_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_ALARM_MASK_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_VOLTAGE_MIN_THRESHOLD_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_VOLTAGE_THRESHOLD_1_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_VOLTAGE_THRESHOLD_2_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_VOLTAGE_THRESHOLD_3_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_PERCENTAGE_MIN_THRESHOLD_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_PERCENTAGE_THRESHOLD_1_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_PERCENTAGE_THRESHOLD_2_ID:
        case TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_PERCENTAGE_THRESHOLD_3_ID:
            // check to see if an alarm needs to be sent
            check_batt_value_for_alarm(endpoint, attr_id, new_value);
            break;
    }
    tr_power_configuration_server_write_attr_cb(endpoint, attr_id, new_value, manuf_code);
}

static zb_bool_t power_configuration_server_cluster_handler(zb_uint8_t param)
{
    zb_bool_t           processed = ZB_TRUE;
    zb_zcl_parsed_hdr_t cmd_info;
    zb_ret_t            status = RET_OK;

    if (ZB_ZCL_GENERAL_GET_CMD_LISTS_PARAM == param)
    {
        ZCL_CTX().zb_zcl_cluster_cmd_list = &gs_power_configuration_server_cmd_list;
        return ZB_TRUE;
    }

    ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);

    switch (cmd_info.cmd_id)
    {
        default:
            tr_power_configuration_server_printf("RX:(%s) Unknown Cmd ID: %02X, EP: %02X\n",
                                                 PLUGIN_NAME,
                                                 cmd_info.cmd_id,
                                                 ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint);
            processed = ZB_FALSE;
            break;
    }

    if (processed)
    {
        if (cmd_info.disable_default_response && status == RET_OK)
        {
            zb_buf_free(param);
        }
        else if (status != RET_BUSY)
        {
            ZB_ZCL_COPY_PARSED_HEADER(param, &cmd_info);
            ZB_ZCL_SEND_DEFAULT_RESP(param,
                                     ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).source.u.short_addr,
                                     ZB_APS_ADDR_MODE_16_ENDP_PRESENT,
                                     ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).src_endpoint,
                                     ZB_ZCL_PARSED_HDR_SHORT_DATA(&cmd_info).dst_endpoint,
                                     cmd_info.profile_id,
                                     TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID,
                                     cmd_info.seq_number,
                                     cmd_info.cmd_id,
                                     status == RET_OK ? TR_ZCL_STATUS_SUCCESS : TR_ZCL_STATUS_INVALID_FIELD);
        }
    }
    return processed;
}

// Power configuration server cluster plugin init
void tr_power_configuration_server_init(void)
{
    zb_zcl_add_cluster_handlers(TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID,
                                TR_ZCL_CLUSTER_SERVER_ROLE,
                                (zb_zcl_cluster_check_value_t)power_configuration_server_check_value,
                                (zb_zcl_cluster_write_attr_hook_t)power_configuration_server_write_attr_hook,
                                (zb_zcl_cluster_handler_t)power_configuration_server_cluster_handler);

    tr_power_configuration_server_init_cb();
}

// Power configuration server cluster plugin set mains voltage
zb_zcl_status_t tr_power_configuration_server_set_mains_voltage(zb_uint8_t  endpoint,
                                                                zb_uint16_t voltage_100mv)
{
    return (zb_zcl_set_attr_val(endpoint,
                                TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID,
                                TR_ZCL_CLUSTER_SERVER_ROLE,
                                TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_VOLTAGE_ID,
                                (zb_uint8_t*)&voltage_100mv,
                                ZB_FALSE));
}

// Power configuration server cluster plugin set mains frequency
zb_zcl_status_t tr_power_configuration_server_set_mains_frequency(zb_uint8_t endpoint,
                                                                  zb_uint8_t frequency_hz)
{
    return (zb_zcl_set_attr_val(endpoint,
                                TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID,
                                TR_ZCL_CLUSTER_SERVER_ROLE,
                                TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_FREQUENCY_ID,
                                (zb_uint8_t*)&frequency_hz,
                                ZB_FALSE));
}

// Power configuration server cluster plugin set battery voltage
zb_zcl_status_t tr_power_configuration_server_set_battery_voltage(zb_uint8_t                                     endpoint,
                                                                  tr_power_configuration_server_battery_source_t battery_source,
                                                                  zb_uint8_t                                     voltage_100mv)
{
    return (zb_zcl_set_attr_val(endpoint,
                                TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID,
                                TR_ZCL_CLUSTER_SERVER_ROLE,
                                TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_VOLTAGE_ID + (battery_source * 0x20),
                                (zb_uint8_t*)&voltage_100mv,
                                ZB_FALSE));
}

// Power configuration server cluster plugin set battery percent remaining
zb_zcl_status_t tr_power_configuration_server_set_battery_percentage_remaining(zb_uint8_t                                     endpoint,
                                                                               tr_power_configuration_server_battery_source_t battery_source,
                                                                               zb_uint8_t                                     percentage_remaining)
{
    return (zb_zcl_set_attr_val(endpoint,
                                TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID,
                                TR_ZCL_CLUSTER_SERVER_ROLE,
                                TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_PERCENTAGE_REMAINING_ID + (battery_source * 0x20),
                                (zb_uint8_t*)&percentage_remaining,
                                ZB_FALSE));
}

// Power configuration server cluster plugin set/clear mains power lost
void tr_power_configuration_server_set_clear_mains_power_lost(zb_uint8_t endpoint,
                                                              zb_bool_t  mains_power_lost)
{
    zb_uint32_t batt_alarm_state;
    zb_uint8_t  mains_alarm_mask;

    batt_alarm_state = get_pwr_cfg_u32_attr_val(TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_ALARM_STATE_ID,
                                                endpoint,
                                                0x00000000);
    mains_alarm_mask = get_pwr_cfg_u8_attr_val(TR_ZCL_ATTR_POWER_CONFIGURATION_MAINS_ALARM_MASK_ID,
                                               endpoint,
                                               0x00);

    if (mains_power_lost)
    {
        // check and set the battery alarm state bit
        if ((batt_alarm_state & ZB_ZCL_POWER_CONFIG_BATTERY_ALARM_STATE_MAINS_POWER_SUPPLY_LOST_UNAVAILABLE) == 0)
        {
            // the mains power lost bit is not already set, set it and check the alarm mask
            batt_alarm_state |= ZB_ZCL_POWER_CONFIG_BATTERY_ALARM_STATE_MAINS_POWER_SUPPLY_LOST_UNAVAILABLE;

            // check the alarm mask bit and send an alarm if needed
            if ((mains_alarm_mask & TR_ZCL_MAINS_ALARM_MASK_MAINS_POWER_SUPPLY_LOST_MASK) != 0)
            {
                // the mask bit is set, send the alarm
                zb_uint16_t ep_and_alarm_code = (endpoint << 8) | ZB_ZCL_POWER_CONFIG_BATTERY_ALARM_CODE_MAINS_POWER_SUPPLY_LOST_UNAVAILABLE;
                zb_buf_get_out_delayed_ext(get_send_alarm_buf, ep_and_alarm_code, 0);
            }

            // write the new value for the battery alarm state attribute(s)
            zb_zcl_set_attr_val(endpoint,
                                TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID,
                                TR_ZCL_CLUSTER_SERVER_ROLE,
                                TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_ALARM_STATE_ID,
                                (zb_uint8_t*)&batt_alarm_state,
                                ZB_FALSE);
            zb_zcl_set_attr_val(endpoint,
                                TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID,
                                TR_ZCL_CLUSTER_SERVER_ROLE,
                                TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_ALARM_STATE_ID,
                                (zb_uint8_t*)&batt_alarm_state,
                                ZB_FALSE);
            zb_zcl_set_attr_val(endpoint,
                                TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID,
                                TR_ZCL_CLUSTER_SERVER_ROLE,
                                TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_ALARM_STATE_ID,
                                (zb_uint8_t*)&batt_alarm_state,
                                ZB_FALSE);
        }
    }
    else
    {
        // mains power has been restored
        // check and clear the battery alarm state bit
        if ((batt_alarm_state & ZB_ZCL_POWER_CONFIG_BATTERY_ALARM_STATE_MAINS_POWER_SUPPLY_LOST_UNAVAILABLE) != 0)
        {
            // the mains power lost bit is set set, clear it and check the alarm mask
            batt_alarm_state &= ~ZB_ZCL_POWER_CONFIG_BATTERY_ALARM_STATE_MAINS_POWER_SUPPLY_LOST_UNAVAILABLE;

            // write the new value for the battery alarm state attribute(s)
            zb_zcl_set_attr_val(endpoint,
                                TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID,
                                TR_ZCL_CLUSTER_SERVER_ROLE,
                                TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_ALARM_STATE_ID,
                                (zb_uint8_t*)&batt_alarm_state,
                                ZB_FALSE);
            zb_zcl_set_attr_val(endpoint,
                                TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID,
                                TR_ZCL_CLUSTER_SERVER_ROLE,
                                TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_2_ALARM_STATE_ID,
                                (zb_uint8_t*)&batt_alarm_state,
                                ZB_FALSE);
            zb_zcl_set_attr_val(endpoint,
                                TR_ZCL_CLUSTER_POWER_CONFIGURATION_ID,
                                TR_ZCL_CLUSTER_SERVER_ROLE,
                                TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_3_ALARM_STATE_ID,
                                (zb_uint8_t*)&batt_alarm_state,
                                ZB_FALSE);
        }
    }
}

#ifdef TR_POWER_CONFIGURATION_SERVER_BATTERY_LATCHING

// Power configuration server cluster plugin unlatch the battery alarm state bits
void tr_power_configuration_server_unlatch_battery(zb_uint8_t                                     endpoint,
                                                   tr_power_configuration_server_battery_source_t battery_source)
{
    g_power_configuration_server_unlatch_flag[battery_source] = ZB_TRUE;

    // do a "fake" check of voltage against the thresholds for the specified battery source
    zb_uint8_t batt_voltage = get_pwr_cfg_u8_attr_val(TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_VOLTAGE_ID + (battery_source * 0x20),
                                                      endpoint,
                                                      0x00000000);

    check_batt_value_for_alarm(endpoint,
                               TR_ZCL_ATTR_POWER_CONFIGURATION_BATTERY_VOLTAGE_ID + (battery_source * 0x20),
                               (zb_uint8_t*)&batt_voltage);
}

#endif /* ifdef TR_POWER_CONFIGURATION_SERVER_BATTERY_LATCHING */
