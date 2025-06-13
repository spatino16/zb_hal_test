/// ****************************************************************************
/// @file tr_cli_buffer.h
///
/// @brief code for buffering bytes to be sent to the CLI utility
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_CLI_BUFFER_H
#define TR_CLI_BUFFER_H

#include <stdint.h>

/// ***************************************************************************
/// FUNCTIONS
/// ***************************************************************************

// init the CLI byte buffer
void tr_cli_buffer_init(void);

// store a byte from the UART in the buffer
void tr_cli_buffer_byte(uint8_t new_input_char);

// empty the bytes from the buffer and hand to the CLI
void tr_cli_buffer_process_bytes(void);

// clear the stats of the driver
void tr_cli_buffer_clear_stats(void);

// driver stats, one of these is returned from tr_cli_buffer_get_stats()
typedef struct
{
    // max number of buffered bytes at any time since stats were cleared
    uint8_t buffer_high_watermark;

    // character drops due to buffer being full
    uint16_t byte_drops;

    // size of the character buffer
    uint16_t buffer_size;

} tr_cli_buffer_stats_t;

// read the driver stats
tr_cli_buffer_stats_t tr_cli_buffer_get_stats(void);


#endif // TR_CLI_BUFFER_H
