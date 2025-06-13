/// ****************************************************************************
/// @file tr_circular_buffer.c
///
/// @brief TODO - documentation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#include <stdint.h>
#include <stdbool.h>
#include "tr_circular_buffer.h"
#include "tr_debug_print.h"

// Initialize the buffer
void init_buffer(tr_circular_buffer_t *cb)
{
    cb->in_ptr  = 0;
    cb->out_ptr = 0;
    cb->count   = 0;
}

// Check if the buffer is full
bool is_full(tr_circular_buffer_t *cb)
{
    return cb->count == cb->size;
}

// Check if the buffer is empty
bool is_empty(tr_circular_buffer_t *cb)
{
    return cb->count == 0;
}

// Add data to the buffer
bool enqueue(tr_circular_buffer_t *cb,
             const char           *data,
             int                  length)
{
    if (length > (cb->size - cb->count))
    {
        // printf("Not enough space in buffer. Data not added.\n");
        return false;  // Not enough space
    }

    // Write data in a circular manner
    for (int i = 0 ; i < length ; i++)
    {
        cb->buffer[cb->in_ptr] = data[i];
        cb->in_ptr             = (cb->in_ptr + 1) % cb->size;
    }
    cb->count += length;
    return true;
}

// Remove data from the buffer
bool dequeue(tr_circular_buffer_t *cb,
             char                 *data,
             int                  length)
{
    if (length > cb->count)
    {
        // printf("Not enough data in buffer. Cannot dequeue.\n");
        return false;  // Not enough data
    }

    // Read data in a circular manner
    for (int i = 0 ; i < length ; i++)
    {
        data[i]     = cb->buffer[cb->out_ptr];
        cb->out_ptr = (cb->out_ptr + 1) % cb->size;
    }
    cb->count -= length;
    return true;
}

// Display buffer contents for debugging
void dump_buffer(tr_circular_buffer_t *cb)
{
    tr_core_printf("Buffer size:  %d\n", cb->size);
    tr_core_printf("Buffer in:    %d\n", cb->in_ptr);
    tr_core_printf("Buffer out:   %d\n", cb->out_ptr);
    tr_core_printf("Buffer count: %d\n", cb->count);
    tr_core_printf("Buffer contents:\n");

    // for (int i = 0 ; i < cb->size ; i++)
    // {
    //     tr_core_printf("%c", cb->buffer[i]);
    // }
    // tr_core_printf("\n");
}
