/// ****************************************************************************
/// @file tr_hal_config.c
///
/// @brief hardware configuration file for Trident Neptune board - DKNCM11C10
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include "tr_hal_config.h"
#include "tr_cli.h"
#include "tr_plugin_config.h"

static uint8_t app_uart0_transmit_buffer[TR_CLI_MAX_LINE];

/// ***************************************************************************
/// tr_hal_put_char - needed to pass to printf init and CLI init
/// ***************************************************************************
void tr_hal_put_char(char ch)
{
    tr_hal_uart_raw_tx_one_byte(UART_DBG_PORT_ID, ch);
}

/// ***************************************************************************
/// tr_hal_put_string - needed to pass to printf init
/// ***************************************************************************
void tr_hal_put_string(const char *str,
                       int        len)
{
    tr_hal_uart_raw_tx_buffer(UART_DBG_PORT_ID,
                              (char*)str,
                              (uint16_t)len);
}

/// ***************************************************************************
/// tr_hal_button_interrupt_cb - needed for button callback
/// ***************************************************************************
TR_WEAK void tr_hal_button_interrupt_cb(tr_hal_gpio_pin_t   pin,
                                        tr_hal_gpio_event_t event)
{
    (void)pin;
    (void)event;
}

/// ***************************************************************************
/// app_uart0_receive_handler - this is called when the UART receives bytes
/// just pass these directly to the CLI - the CLI already buffers these and
/// handles them outside of interrupt context
/// ***************************************************************************
void app_uart0_receive_handler(uint8_t received_byte)
{
    tr_cli_buffer_byte(received_byte);
}

/// ***************************************************************************
/// tr_hal_init
/// ***************************************************************************
void tr_hal_init(void)
{
    // init uart to cli buffer
    tr_cli_buffer_init();
    tr_cli_buffer_clear_stats();

    // ****************************
    // setup UART0 for CLI
    // ****************************
    // use default settings as a base
    tr_hal_uart_settings_t g_uart0_settings = DEFAULT_UART0_CONFIG;
    // setup rx function which saves bytes and hands them to the CLI later
    g_uart0_settings.rx_handler_function = app_uart0_receive_handler;
    // we need a raw TX buffer
    g_uart0_settings.raw_tx_buffer      = app_uart0_transmit_buffer;
    g_uart0_settings.raw_tx_buff_length = TR_CLI_MAX_LINE;
    // init the UART0
    tr_hal_uart_init(UART_0_ID, &g_uart0_settings);

    // init printf with the output functions
    tr_printf_init(tr_hal_put_char, tr_hal_put_string);
    // init CLI
    tr_cli_init(TR_CLI_PROMPT, tr_hal_put_char);

    // *******************************************
    // init buttons using Trident HAL APIs
    // *******************************************
    tr_hal_gpio_settings_t button_settings = DEFAULT_GPIO_INPUT_CONFIG;
    button_settings.interrupt_trigger      = TR_HAL_GPIO_TRIGGER_FALLING_EDGE;
    button_settings.event_handler_fx       = tr_hal_button_interrupt_cb;
    button_settings.pull_mode              = TR_HAL_PULLOPT_PULL_UP_100K;

    tr_hal_gpio_init(GPIO_BUTTON1, &button_settings);
    tr_hal_gpio_init(GPIO_BUTTON2, &button_settings);

    // *******************************************
    // init LEDs using Trident HAL APIs
    // *******************************************
    tr_hal_gpio_settings_t led_settings = DEFAULT_GPIO_OUTPUT_CONFIG;
    led_settings.output_level           = LED_OFF;

    tr_hal_gpio_init(GPIO_LED_RED, &led_settings);
    tr_hal_gpio_init(GPIO_LED_GREEN, &led_settings);
    tr_hal_gpio_init(GPIO_LED_BLUE, &led_settings);
}
