/// ****************************************************************************
/// @file bootloader.h
///
/// @brief
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef BOOTLOADER_H
#define BOOTLOADER_H

#include "tr_hal_config.h"
#include "tr_printf.h"

#define FLASH_PAGE_PROGRAMMING_SIZE 256
#define SIZE_OF_FLASH_SECTOR_ERASE  4096
#define FLASH_PAGE_MASK             (~(256 - 1))

#ifndef PRINT_ENABLE
#define PRINT_ENABLE 0
#endif

// this define controls how the bootloader looks for the start of xmodem
// if it is a 1, it looks for an 'x', otherwise it simply starts by sending
// the NAK and hopes for a response.
// rignt now leave this set to 1
#define LOOK_FOR_START_CHAR 1

#if PRINT_ENABLE
#define PRINT(...) tr_printf(__VA_ARGS__)
#else
#define PRINT(...)
#endif

void set_flash_erase(uint32_t flash_addr,
                     uint32_t image_size);
int serial_read(int length,
                int timeout_ms);
uint32_t crc32(uint32_t flash_addr,
               uint32_t data_len);

#endif // BOOTLAODER_H
