/// ****************************************************************************
/// @file tr_hal_config.h
///
/// @brief pin definitions for Trident Neptune board - DKNCM11C10
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_HAL_CONFIG_H
#define TR_HAL_CONFIG_H
#include "cm3_mcu.h"
#include "tr_hal_gpio.h"
#include "tr_hal_uart.h"

#define UART_DBG_PORT_ID 0
#define GPIO_UART_DBG_RX (tr_hal_gpio_pin_t){ 16 }
#define GPIO_UART_DBG_TX (tr_hal_gpio_pin_t){ 17 }

void tr_hal_init(void);

#endif /* TR_HAL_CONFIG_H */
