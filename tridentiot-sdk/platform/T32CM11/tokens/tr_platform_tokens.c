/// ****************************************************************************
/// @file tr_platform_tokens.c
///
/// @brief token interface to CM11 platform
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include <string.h>
#include <stdlib.h>
#include "mp_sector.h"
#include "flashctl.h"

extern void MpCalCrystaltrimInit(mp_cal_xtal_trim_t *mp_cal_xtaltrim);

void tr_platform_token_write(uint8_t  *buffer,
                             uint8_t  buf_size,
                             uint32_t token_addr)
{
    uint8_t  flash_page[LENGTH_PAGE] = { 0 };
    uint32_t start_page              = token_addr / LENGTH_PAGE;
    uint16_t page_index              = token_addr - (start_page * LENGTH_PAGE);
    uint8_t  overflow_bytes          = 0;

    if (buf_size > (LENGTH_PAGE - page_index))
    {
        overflow_bytes = buf_size - (LENGTH_PAGE - page_index);
    }

    // read contents of page
    while (flash_check_busy());
    flash_read_page((uint32_t)&flash_page, (uint32_t)(start_page * LENGTH_PAGE));

    while (flash_check_busy());

    // patch page with token data
    for (uint8_t i = 0 ; i < (buf_size - overflow_bytes) ; i++)
    {
        flash_page[i + page_index] = buffer[i];
    }

    // write page back in
    while (flash_check_busy());
    flash_write_page((uint32_t)&flash_page, (uint32_t)(start_page * LENGTH_PAGE));

    while (flash_check_busy());

    // if we overflowed onto the next page, write the remaining data
    if (overflow_bytes != 0)
    {
        memset(flash_page, 0xFF, sizeof(flash_page));
        uint8_t buffer_offset = buf_size - overflow_bytes;

        // read contents of page
        while (flash_check_busy());
        flash_read_page((uint32_t)&flash_page, (uint32_t)((start_page + 1) * LENGTH_PAGE));

        while (flash_check_busy());

        // patch page with token data
        for (uint8_t i = 0 ; i < overflow_bytes ; i++)
        {
            flash_page[i] = buffer[i + buffer_offset];
        }

        // write page back in
        while (flash_check_busy());
        flash_write_page((uint32_t)&flash_page, (uint32_t)((start_page + 1) * LENGTH_PAGE));

        while (flash_check_busy());
    }
}

void tr_platform_token_read(void     *buffer,
                            uint8_t  buf_size,
                            uint32_t token_addr)
{
    memcpy(buffer, (void*)(token_addr), buf_size);
}

void tr_platform_token_process_xtal_trim(uint16_t xtal_trim)
{
    if (xtal_trim != 0xFFFF)
    {
        mp_cal_xtal_trim_t xtal_data;
        memset(&xtal_data, 0, sizeof(xtal_data));

        xtal_data.flag    = 2;
        xtal_data.xo_trim = xtal_trim;
        MpCalCrystaltrimInit(&xtal_data);
    }
}
