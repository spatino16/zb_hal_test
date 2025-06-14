/// ****************************************************************************
/// @file on_off_bulb_app.c
///
/// @brief non-sleepy on/off bulb ZED sample app
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "stdint.h"
#include "tr_af.h"
#include "tr_hal_config.h"
#include "project_app_tokens.h"
#include "tr_hal_uart.h"



typedef struct
{
    tr_hal_gpio_pin_t led_pin;
    zb_uint8_t        blink_count;
    zb_time_t         on_ms;
    zb_time_t         off_ms;
}led_blink_params_t;

static uint8_t            g_search_attempts = 0;
static led_blink_params_t g_blink_params;

static void start_nwk_search(zb_uint8_t search_attempts);
static void led_blink(tr_hal_gpio_pin_t led_pin,
                      zb_uint16_t       on_ms,
                      zb_uint16_t       off_ms,
                      zb_uint8_t        blink_count);
static void led_blink_handler(zb_uint8_t param);

void uart1_echo_task(void)
{
    char rx_byte;

    tr_hal_status_t status = tr_hal_uart_raw_rx_one_byte(UART_1_ID, &rx_byte);

    if (status == TR_HAL_STATUS_MORE_BYTES || status == TR_HAL_STATUS_DONE)
    {
        tr_app_printf("UART RX: %c\n", rx_byte);  // Debug en consola USB
        tr_hal_uart_raw_tx_one_byte(UART_1_ID, rx_byte);  // eco
    }
}


static void uart_poll_handler(zb_uint8_t param)
{
    ZVUNUSED(param);

    uart1_echo_task();  // lee y responde si hay algo

    ZB_SCHEDULE_APP_ALARM(uart_poll_handler, 0, ZB_MILLISECONDS_TO_BEACON_INTERVAL(50)); // repite cada 50 ms
}



void tr_hal_button_interrupt_cb(tr_hal_gpio_pin_t   pin,
                                tr_hal_gpio_event_t event)
{
    if (tr_hal_gpio_are_pins_equal(pin, GPIO_BUTTON1))
    {
        tr_app_printf("BTN1 pressed!\n");
    }
    else if (tr_hal_gpio_are_pins_equal(pin, GPIO_BUTTON2))
    {
        tr_app_printf("BTN2 pressed!\n");
    }

    // restart backoff delay on button press
    if (tr_network_rejoin_backoff_active())
    {
        tr_network_rejoin_reset_backoff_delay();
    }
}

void tr_app_init_cb(void)
{
    tr_app_printf("on off bulb init!\n");

    memset(&g_blink_params, 0, sizeof(led_blink_params_t));

    if (tr_get_connection_state() != TR_CONN_STATE_NO_NETWORK)
    {
        led_blink(GPIO_LED_GREEN, 2000, 1, 1);
    }

        // UART1 Setup (TX: GPIO4, RX: GPIO5)
    static tr_hal_uart_settings_t g_uart1_settings = {
        .tx_pin = UART1_TX_PIN_OPTION1,    // GPIO4
        .rx_pin = UART1_RX_PIN_OPTION1,    // GPIO5
        .baud_rate = TR_HAL_UART_BAUD_RATE_115200,
        .data_bits = LCR_DATA_BITS_8_VALUE,
        .stop_bits = LCR_STOP_BITS_ONE_VALUE,
        .parity = LCR_PARITY_NONE_VALUE,
        .tx_dma_enabled = false,
        .rx_dma_enabled = false,
        .rx_handler_function = NULL,
        .rx_dma_buffer = NULL,
        .rx_dma_buff_length = 0,
        .rx_bytes_before_trigger = FCR_TRIGGER_1_BYTE,
        .hardware_flow_control_enabled = false,
        .enable_chip_interrupts = false,
        .interrupt_priority = 3,
        .wake_on_interrupt = false,
    };

    tr_hal_status_t uart_status = tr_hal_uart_init(UART_1_ID, &g_uart1_settings);
    tr_app_printf("UART1 init: %s\n", uart_status == TR_HAL_SUCCESS ? "OK" : "FAIL");

    const char* uart_msg = " UART1 test OK from T32CM11!\r\n";
    tr_hal_uart_raw_tx_buffer(UART_1_ID, uart_msg, strlen(uart_msg));

    ZB_SCHEDULE_APP_ALARM(uart_poll_handler, 0, ZB_MILLISECONDS_TO_BEACON_INTERVAL(50));




}


void tr_connection_state_cb(tr_conn_state_e conn_state)
{
    switch (conn_state)
    {
        case TR_CONN_STATE_NO_NETWORK:
            tr_app_printf("TR_CONN_STATE_NO_NETWORK\n");
            led_blink(GPIO_LED_BLUE, 250, 250, 3);
            start_nwk_search(5);
            break;

        case TR_CONN_STATE_NWK_STEERING_ATTEMPT_FAILURE:
            // NWK steering attempt failed
            if (--g_search_attempts != 0)
            {
                tr_app_printf("NWK Search Fail! Attempts Remaining: %d\n", g_search_attempts);
                led_blink(GPIO_LED_BLUE, 250, 250, 3);
                bdb_start_top_level_commissioning(ZB_BDB_NETWORK_STEERING);
            }
            else
            {
                led_blink(GPIO_LED_RED, 2000, 1, 1);
            }
            break;

        case TR_CONN_STATE_UNKNOWN:
        default:
            break;
    }
}

void tr_identify_server_identify_start_cb(zb_uint8_t  endpoint,
                                          zb_uint16_t timeout_sec)
{
    tr_hal_gpio_set_output(GPIO_LED_BLUE, LED_ON);
}

void tr_identify_server_identify_stop_cb(zb_uint8_t endpoint)
{
    tr_hal_gpio_set_output(GPIO_LED_BLUE, LED_OFF);
}

zb_uint8_t tr_zcl_command_cb(zb_uint8_t param)
{
    zb_zcl_parsed_hdr_t *cmd_info = ZB_BUF_GET_PARAM(param, zb_zcl_parsed_hdr_t);

    if (cmd_info->cmd_direction == ZB_ZCL_FRAME_DIRECTION_TO_SRV &&
        cmd_info->cluster_id == TR_ZCL_CLUSTER_ON_OFF_ID)
    {
        switch (cmd_info->cmd_id)
        {
            case TR_ZCL_CMD_OFF_ID:
                tr_hal_gpio_set_output(GPIO_LED_RED, LED_OFF);
                tr_hal_gpio_set_output(GPIO_LED_GREEN, LED_OFF);
                tr_hal_gpio_set_output(GPIO_LED_BLUE, LED_OFF);
                break;

            case TR_ZCL_CMD_ON_ID:
                tr_hal_gpio_set_output(GPIO_LED_RED, LED_ON);
                tr_hal_gpio_set_output(GPIO_LED_GREEN, LED_ON);
                tr_hal_gpio_set_output(GPIO_LED_BLUE, LED_ON);
                break;

            case TR_ZCL_CMD_TOGGLE_ID:
                tr_hal_gpio_toggle_output(GPIO_LED_RED);
                tr_hal_gpio_toggle_output(GPIO_LED_GREEN);
                tr_hal_gpio_toggle_output(GPIO_LED_BLUE);
                break;
        }
    }

    return ZB_FALSE;
}

/************************************************************************************/
/**                               Private Functions                                **/
/************************************************************************************/

static void start_nwk_search(zb_uint8_t search_attempts)
{
    if (tr_get_connection_state() == TR_CONN_STATE_NO_NETWORK)
    {
        tr_app_printf("Starting NWK Search! Attempts: %d\n", search_attempts);
        g_search_attempts = search_attempts;
        zb_zdo_set_nwk_scan_attempts(1);            // scan primary and secondary channel sets once
        bdb_start_top_level_commissioning(ZB_BDB_NETWORK_STEERING);
    }
}

static void led_blink(tr_hal_gpio_pin_t led_pin,
                      zb_uint16_t       on_ms,
                      zb_uint16_t       off_ms,
                      zb_uint8_t        blink_count)
{
    zb_time_t timeout = 0;

    ZB_SCHEDULE_GET_ALARM_TIME(led_blink_handler, ZB_ALARM_ANY_PARAM, &timeout);

    if (timeout > 0)
    {
        ZB_SCHEDULE_APP_ALARM_CANCEL(led_blink_handler, ZB_ALARM_ANY_PARAM);
    }

    g_blink_params.led_pin     = led_pin;
    g_blink_params.on_ms       = ZB_MILLISECONDS_TO_BEACON_INTERVAL(on_ms);
    g_blink_params.off_ms      = ZB_MILLISECONDS_TO_BEACON_INTERVAL(off_ms);
    g_blink_params.blink_count = blink_count;

    tr_hal_gpio_set_output(GPIO_LED_RED, LED_OFF);
    tr_hal_gpio_set_output(GPIO_LED_GREEN, LED_OFF);
    tr_hal_gpio_set_output(GPIO_LED_BLUE, LED_OFF);

    if (g_blink_params.on_ms != 0)
    {
        led_blink_handler(0);
    }
}

static void led_blink_handler(zb_uint8_t param)
{
    ZVUNUSED(param);
    tr_hal_level_t led_level = 0;

    tr_hal_gpio_get_output(g_blink_params.led_pin,
                           &led_level);

    if (led_level == LED_ON)
    {
        tr_hal_gpio_set_output(g_blink_params.led_pin, LED_OFF);
        g_blink_params.blink_count--;

        if (g_blink_params.blink_count > 0)
        {
            ZB_SCHEDULE_APP_ALARM(led_blink_handler, 0, g_blink_params.off_ms);
        }
    }
    else
    {
        tr_hal_gpio_set_output(g_blink_params.led_pin, LED_ON);

        if (g_blink_params.blink_count > 0)
        {
            ZB_SCHEDULE_APP_ALARM(led_blink_handler, 0, g_blink_params.on_ms);
        }
    }
    ///uart1_echo_task();
}

/************************************************************************************/
/**                          Network Rejoin Plugin Test                            **/
/************************************************************************************/

zb_bool_t tr_network_rejoin_attempt_cb(zb_uint32_t channel_mask,
                                       zb_bool_t   secure)
{
    led_blink(GPIO_LED_GREEN, 100, 1, 1);
    return ZB_TRUE;
}

/************************************************************************************/
/**                        External Attributes and Tokens                          **/
/************************************************************************************/

// helper function to find the first erased (0xff) byte in an array
uint8_t get_token_str_len(uint8_t *data)
{
    uint8_t i = 0;

    while (data[i] != 0xFF)
    {
        i++;
    }
    return i;
}

// test attribute values
// these must remain available outside of the external attribute read function
static uint8_t model[] = { 0x07, 'M', 'y', ' ', 'B', 'u', 'l', 'b' };
static uint8_t mfg_name[tr_get_mfg_token_len(TR_MFG_TOKEN_MFG_NAME) + 1];

// this is a zigbee basic cluster product code. for this example, it is a UPC-A code
// per ZCL8, add 1 byte for octstr len and 1 byte for prod code type (0x03 for UPC)
// this token was written with the following command:
//    elcap tokens write --type APP --def build/T32CM11C.Debug/T32CM11C_app_token_def.json --name APP_TOK_UPC --value 0x313233343536373839303132 --usb <target usb id>
//       this sets the token to '123456789012'. elcap --value string handling will be fixed in a future release
static uint8_t prod_code[tr_get_app_token_len(APP_TOK_UPC) + 2];

// callback for external attribute reads
// return NULL if the attribute is not to be handled by the application
// return a pointer to the data if it is handled by the application
zb_uint8_t *tr_zcl_external_attribute_read_cb(uint8_t  endpoint,
                                              uint16_t cluster_id,
                                              uint8_t  cluster_role,
                                              uint16_t attr_id,
                                              uint16_t manuf_code)
{
    if ((endpoint == TR_DEVICE_EP1) && (manuf_code == ZB_ZCL_NON_MANUFACTURER_SPECIFIC))
    {
        switch (cluster_id)
        {
            case TR_ZCL_CLUSTER_BASIC_ID:
            {
                if (cluster_role == TR_ZCL_CLUSTER_SERVER_ROLE)
                {
                    switch (attr_id)
                    {
                        case TR_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID:
                        {
                            tr_get_mfg_token(&mfg_name[1], TR_MFG_TOKEN_MFG_NAME);
                            mfg_name[0] = get_token_str_len(&mfg_name[1]);
                            return mfg_name;
                            break;
                        }

                        case TR_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID:
                        {
                            return model;
                            break;
                        }

                        case TR_ZCL_ATTR_BASIC_PRODUCT_CODE_ID:
                        {
                            tr_get_app_token(&prod_code[2], APP_TOK_UPC);
                            prod_code[0] = 13; // octstr len = 12 for upc + 1 for type
                            prod_code[1] = TR_ZCL_PRODUCT_CODE_UNIVERSAL_PRODUCT_CODE;
                            return prod_code;
                            break;
                        }
                    }
                }
                break;
            }
        }
    }
    return NULL;
}
