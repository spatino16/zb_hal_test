/// ****************************************************************************
/// @file tr_circular_buffer.h
///
/// @brief TODO - documentation
///
///
/// SPDX-License-Identifier: LicenseRef-TridentMSLA
/// SPDX-FileCopyrightText: 2025 Trident IoT, LLC <https://www.tridentiot.com>
/// ***************************************************************************
#ifndef TR_CIRCULAR_BUFFER_H
#define TR_CIRCULAR_BUFFER_H

typedef struct
{
    char     *buffer;
    uint32_t size;
    uint32_t in_ptr;
    uint32_t out_ptr;
    uint32_t count;
}tr_circular_buffer_t;

// macro for creating the circular buffer
#define TR_CIRCULAR_BUFFER(name, size) \
        char name##_buf[size];         \
        tr_circular_buffer_t name =    \
        { name##_buf, size, 0, 0, 0 };

#define TR_CIRCULAR_BUFFER_INIT(name) \
        init_buffer((tr_circular_buffer_t*)&name)

#define TR_CIRCULAR_BUFFER_ENQUEUE(name, data, len) \
        enqueue((tr_circular_buffer_t*)&name, (const char*)data, len)

#define TR_CIRCULAR_BUFFER_DEQUEUE(name, data, len) \
        dequeue((tr_circular_buffer_t*)&name, (char*)data, len)

#define TR_CIRCULAR_BUFFER_LENGTH(name) \
        (tr_circular_buffer_t*)&name->count

#define TR_CIRCULAR_BUFFER_IS_EMPTY(name) \
        is_empty((tr_circular_buffer_t*)&name)

#define TR_CIRCULAR_BUFFER_DUMP_BUFFER(name) \
        dump_buffer((tr_circular_buffer_t*)&name)

// function prototypes
void init_buffer(tr_circular_buffer_t *cb);
bool is_full(tr_circular_buffer_t *cb);
bool is_empty(tr_circular_buffer_t *cb);

bool enqueue(tr_circular_buffer_t *cb,
             const char           *data,
             int                  length);
bool dequeue(tr_circular_buffer_t *cb,
             char                 *data,
             int                  length);
void dump_buffer(tr_circular_buffer_t *cb);

#endif // ifndef TR_CIRCULAR_BUFFER_H
