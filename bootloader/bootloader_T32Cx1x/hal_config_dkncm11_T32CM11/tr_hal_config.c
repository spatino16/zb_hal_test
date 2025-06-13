/// ****************************************************************************
/// @file tr_hal_config.c
///
/// @brief hardware configuration file for Trident Neptune board - DKNCM11C10
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include <stdint.h>
#include "tr_hal_config.h"
#include "tr_printf.h"

extern void xmodem_byte_rx(uint8_t data);

#define TX_BUF_LEN 32
static uint8_t app_uart0_transmit_buffer[TX_BUF_LEN];

void tr_hal_put_char(char ch)
{
    tr_hal_uart_raw_tx_one_byte(UART_DBG_PORT_ID, ch);
}

void tr_hal_put_string(const char *str,
                       int        len)
{
    tr_hal_uart_raw_tx_buffer(UART_DBG_PORT_ID,
                              str,
                              (uint16_t)len);
}

void tr_hal_init(void)
{
    tr_hal_uart_settings_t g_uart0_settings = DEFAULT_UART0_CONFIG;
    g_uart0_settings.rx_handler_function    = xmodem_byte_rx;
    g_uart0_settings.raw_tx_buffer          = app_uart0_transmit_buffer;
    g_uart0_settings.raw_tx_buff_length     = TX_BUF_LEN;
    g_uart0_settings.interrupt_priority     = TR_HAL_INTERRUPT_PRIORITY_6;
    tr_hal_uart_init(UART_0_ID, &g_uart0_settings);

    tr_printf_init(tr_hal_put_char, tr_hal_put_string);
}
