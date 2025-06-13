/// ****************************************************************************
/// @file zb_osif_platform.h
///
/// @brief ZBOSS interface layer
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef ZB_PLATFORM_OSIF_H
#define ZB_PLATFORM_OSIF_H 1

#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "cm3_mcu.h"
#include "zb_types.h"
#include "sys_arch.h"

/* At ARM all types from 1 to 4 bytes are passed to vararg with casting to 4 bytes */
typedef zb_uint32_t zb_minimal_vararg_t;

/* use macros to be able to redefine */
#define ZB_MEMCPY  memcpy
#define ZB_MEMMOVE memmove
#define ZB_MEMSET  memset
#define ZB_MEMCMP  memcmp

/* trident macros that can be redefined */
#define TR_MALLOC pvPortMalloc
#define TR_FREE   vPortFree

#define ZB_BZERO(s, l) ZB_MEMSET((char*)(s), 0, (l))
#define ZB_BZERO2(s)   ZB_BZERO(s, 2)


#ifdef ZB_IAR
#define NULL _NULL
#else
#ifndef NULL
#define NULL ((void*)0)
#endif
#endif

#define ZVUNUSED(v) (void)v

void tr_zb_osif_platform_init(void);

#define ZB_PLATFORM_INIT() tr_zb_osif_platform_init()

zb_uint32_t tr_zb_osif_get_eui64(zb_uint8_t *eui64);

zb_uint32_t tr_zb_osif_timer_status(void);
void tr_zb_osif_start_timer(void);
void tr_zb_osif_stop_timer(void);


#define ZB_CHECK_TIMER_IS_ON() tr_zb_osif_timer_status()
#define ZB_START_HW_TIMER()    tr_zb_osif_start_timer()
#define ZB_STOP_HW_TIMER()     tr_zb_osif_stop_timer()


void tr_zb_osif_enable_int(void);
void tr_zb_osif_disable_int(void);

#define ZB_TRANSPORT_NONBLOCK_ITERATION() 0
#define ZB_ENABLE_ALL_INTER()             tr_zb_osif_enable_int()
#define ZB_DISABLE_ALL_INTER()            tr_zb_osif_disable_int()

#define ZB_OSIF_GLOBAL_LOCK()             ZB_DISABLE_ALL_INTER()
#define ZB_OSIF_GLOBAL_UNLOCK()           ZB_ENABLE_ALL_INTER()


void tr_zb_osif_abort(void);

#define ZB_ABORT tr_zb_osif_abort

// TODO: figure out what should happen when this is called
// #define ZB_GO_IDLE() CPUwfi()
#define ZB_GO_IDLE()


#if !defined ZB_SOFT_SECURITY || defined ZB_HW_ZB_AES128
void zb_osif_hw_aes_init(void);

#endif /* if !defined ZB_SOFT_SECURITY || defined ZB_HW_ZB_AES128 */
#endif /* ZB_PLATFORM_OSIF_H */
