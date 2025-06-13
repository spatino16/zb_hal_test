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
#include "tr_hal_timers.h"
#include "tr_hal_spi.h"
#include "tr_hal_rtc.h"
#include "tr_hal_wdog.h"

#define UART_DBG_PORT_ID 0
#define GPIO_UART_DBG_RX (tr_hal_gpio_pin_t){ 16 }
#define GPIO_UART_DBG_TX (tr_hal_gpio_pin_t){ 17 }

#define GPIO_BUTTON1     (tr_hal_gpio_pin_t){ 4 }
#define GPIO_BUTTON2     (tr_hal_gpio_pin_t){ 5 }

#define GPIO_LED_RED     (tr_hal_gpio_pin_t){ 1 }
#define GPIO_LED_GREEN   (tr_hal_gpio_pin_t){ 30 }
#define GPIO_LED_BLUE    (tr_hal_gpio_pin_t){ 31 }

#define LED_ON           0
#define LED_OFF          1

#define TR_WEAK          __attribute__((weak))


void tr_hal_init(void);
void tr_hal_button_interrupt_cb(tr_hal_gpio_pin_t   pin,
                                tr_hal_gpio_event_t event);


#endif /* TR_HAL_CONFIG_H */
