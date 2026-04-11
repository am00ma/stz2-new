#pragma once

#include "stz2/types.h"

typedef struct
{
    u8* buf[2];
    u64 len[2];

} RingData;

typedef struct
{
    u64 read;  ///< Read pointer
    u64 write; ///< Write pointer
    u64 cap;   ///< Must be power of two
    u8* buf;   ///< Data

} Ring;

/* Avoid modulus by truncating msb bits */
#define ring_mask(r, val) ((val) & (r->cap - 1))

/* Check avail before read/write */
u64 ring_avail_read(Ring* r);
u64 ring_avail_write(Ring* r);

/* Custom read/write (writing to opaque bufs for example) */
RingData ring_begin_read(Ring* r, u64 size);
RingData ring_begin_write(Ring* r, u64 size);
void     ring_commit_read(Ring* r, u64 size);
void     ring_commit_write(Ring* r, u64 size);

/* Convinience functions to manipulate RingData the most common way
 * Returns actual amount of data read/written */
u64 ring_data_read(RingData* d, u8* buf);
u64 ring_data_write(RingData* d, u8* buf);

/* Combining all the above,
 * Returns actual amount of data read/written */
u64 ring_read(Ring* r, u8* buf, u64 size);
u64 ring_write(Ring* r, u8* buf, u64 size);
