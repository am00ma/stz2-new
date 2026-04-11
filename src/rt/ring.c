#include "stz2/rt/ring.h"

u64 ring_avail_read(Ring* r)
{
    i64 write, read;
    write = r->write;
    __atomic_thread_fence(__ATOMIC_ACQUIRE);
    read = r->read;

    Assert((write - read <= (i64)r->cap), "OOM");
    return write - read;
}

u64 ring_avail_write(Ring* r)
{
    i64 write, read;
    write = r->write;
    read  = r->read;
    __atomic_thread_fence(__ATOMIC_ACQUIRE);

    Assert(((i64)r->cap - (write - read) >= 0), "OOM");
    return r->cap - (write - read);
}

// TODO: Actual code
RingData ring_begin_read(Ring* r, u64 size)
{
    RingData data = {};

    i64 rptr  = ring_mask(r, r->read);
    u64 avail = ring_avail_read(r);
    size      = size > avail ? avail : size;

    if ((rptr + size) <= r->cap)
    {
        data = (RingData){
            .buf = {&r->buf[rptr], NULL},
            .len = {size, 0},
        };
    }
    else
    {
        data = (RingData){
            .buf = {&r->buf[rptr], &r->buf[0]},
            .len = {r->cap - rptr, size - (r->cap - rptr)},
        };
    }

    return data;
}

RingData ring_begin_write(Ring* r, u64 size)
{
    RingData data = {};

    i64 wptr  = ring_mask(r, r->write);
    u64 avail = ring_avail_write(r);
    size      = size > avail ? avail : size;

    if ((wptr + size) <= r->cap)
    {
        data = (RingData){
            .buf = {&r->buf[wptr], NULL},
            .len = {size, 0},
        };
    }
    else
    {
        data = (RingData){
            .buf = {&r->buf[wptr], &r->buf[0]},
            .len = {r->cap - wptr, size - (r->cap - wptr)},
        };
    }

    return data;
}

void ring_commit_read(Ring* r, u64 size)
{
    u64 dst = r->read + size;
    __atomic_thread_fence(__ATOMIC_RELEASE);
    r->read = dst;
}

void ring_commit_write(Ring* r, u64 size)
{
    u64 dst = r->write + size;
    __atomic_thread_fence(__ATOMIC_RELEASE);
    r->write = dst;
}

// Assumes buffer is big enough
// Can check d.len[0] + d.len[1] to ensure
u64 ring_data_read(RingData* d, u8* buf)
{
    if (!d->len[0]) return 0;

    memcpy(buf, d->buf[0], d->len[0]);
    if (!d->len[1]) return d->len[0];

    memcpy(&buf[d->len[0]], d->buf[1], d->len[1]);
    return d->len[0] + d->len[1];
}

// Assumes buffer is big enough
// Can check d.len[0] + d.len[1] to ensure
u64 ring_data_write(RingData* d, u8* buf)
{
    if (!d->len[0]) return 0;

    memcpy(d->buf[0], buf, d->len[0]);
    if (!d->len[1]) return d->len[0];

    memcpy(d->buf[1], &buf[d->len[0]], d->len[1]);
    return d->len[0] + d->len[1];
}

u64 ring_read(Ring* r, u8* buf, u64 size)
{
    RingData d     = ring_begin_read(r, size);
    u64      nread = ring_data_read(&d, buf);
    ring_commit_read(r, nread);
    return nread;
}

u64 ring_write(Ring* r, u8* buf, u64 size)
{
    RingData d      = ring_begin_write(r, size);
    u64      nwrite = ring_data_write(&d, buf);
    ring_commit_write(r, nwrite);
    return nwrite;
}
