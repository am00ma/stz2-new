#include "stz2/rt/ring.h"

#include "stz2/test.h"

#include <pthread.h>

typedef struct
{
    pthread_t tid;
    Ring*     ring;

    u8* buf;
    u64 chunk;
    u64 ndone;
    u64 total;

} ThreadData;

void* thread_producer(void* data)
{
    ThreadData* prod = data;

    while (prod->ndone < prod->total)
    {
        u64 avail = prod->ndone + prod->chunk > prod->total ? prod->total - prod->ndone : prod->chunk;
        // p_info("prod : avail = %ld (%ld, a:%ld, w:%ld, r:%ld)", //
        //        avail, prod->ndone, ring_avail_read(prod->ring), prod->ring->write, prod->ring->read);

        // p_info("     : spinlock start");
        // while (ring_avail_write(prod->ring) < avail) {}
        // p_info("     : spinlock end");

        u64 done = ring_write(prod->ring, &prod->buf[prod->ndone], avail);
        // p_info("     : done  = %ld", done);
        // if (!done) p_info("prod: done == 0");

        prod->ndone += done;
        // p_info("     : ndone = %ld", prod->ndone);
    }

    // p_title("Finished Producer");

    return NULL;
}

void* thread_consumer(void* data)
{
    ThreadData* cons = data;

    while (cons->ndone < cons->total)
    {
        u64 avail = cons->ndone + cons->chunk > cons->total ? cons->total - cons->ndone : cons->chunk;
        // p_info("cons : avail = %ld (%ld, a:%ld, w:%ld, r:%ld)", //
        //        avail, cons->ndone, ring_avail_read(cons->ring), cons->ring->write, cons->ring->read);

        // p_info("     : spinlock start");
        // while (ring_avail_read(cons->ring) < avail) {}
        // p_info("     : spinlock end");

        u64 done = ring_read(cons->ring, &cons->buf[cons->ndone], avail);
        // p_info("     : done  = %ld", done);
        // if (!done) p_info("cons: done == 0");

        cons->ndone += done;
        // p_info("     : ndone = %ld", cons->ndone);
    }

    // p_title("Finished Consumer");

    return NULL;
}

int main(int argc, char* argv[])
{

    TEST_SUITE("stz-rt/ring.h");

    Buf perm = buf_new(1024);

    TEST_CASE("Struct size")
    {
        EXPECT_EQ_LONG(sizeof(Ring), 32L);
        EXPECT_EQ_LONG(sizeof(RingData), 32L);
    }

    TEST_CASE("RingData: Single part")
    {
        Buf temp = perm;

        u64  cap  = 16;
        Ring ring = {
            .cap = cap,
            .buf = make(&temp, u8, cap),
        };
        RingData d = {};

        u64 nwritten, nread;
        u8  buf_write[cap];
        u8  buf_read[cap];
        RANGE(i, (isize)cap) { buf_write[i] = i; }
        RANGE(i, (isize)cap) { buf_read[i] = 0; }

        EXPECT_EQ_LONG(ring_avail_read(&ring), 0L);
        EXPECT_EQ_LONG(ring_avail_write(&ring), cap);

        // --- 1st write ---
        u64 w1 = 4;

        d = ring_begin_write(&ring, w1);
        EXPECT_NEQ_NULL(d.buf[0]);
        EXPECT_EQ_LONG(d.len[0], w1);
        EXPECT_EQ_NULL(d.buf[1]);
        EXPECT_EQ_LONG(d.len[1], 0L);

        nwritten = ring_data_write(&d, buf_write);
        EXPECT_EQ_LONG(nwritten, w1);

        ring_commit_write(&ring, nwritten);

        // --- 1st read ---
        u64 r1 = 3;

        d = ring_begin_read(&ring, r1);
        EXPECT_NEQ_NULL(d.buf[0]);
        EXPECT_EQ_LONG(d.len[0], r1);
        EXPECT_EQ_NULL(d.buf[1]);
        EXPECT_EQ_LONG(d.len[1], 0L);

        nread = ring_data_read(&d, buf_read);
        EXPECT_EQ_LONG(nread, r1);
        RANGE(i, (isize)nread) { EXPECT_EQ_INT(buf_read[i], (int)i); }

        ring_commit_read(&ring, nread);

        EXPECT_EQ_LONG(ring_avail_read(&ring), 1L);
        EXPECT_EQ_LONG(ring_avail_write(&ring), cap - (w1 - r1));

        // --- 2nd read (> avail) ---
        u64 r2 = 3;

        d = ring_begin_read(&ring, r2);
        EXPECT_NEQ_NULL(d.buf[0]);
        EXPECT_EQ_LONG(d.len[0], 1L);
        EXPECT_EQ_NULL(d.buf[1]);
        EXPECT_EQ_LONG(d.len[1], 0L);

        nread = ring_data_read(&d, &buf_read[nread]); // Note offset for buffer
        EXPECT_EQ_LONG(nread, 1L);
        EXPECT_EQ_INT(buf_read[3], 3);

        ring_commit_read(&ring, nread);

        EXPECT_EQ_LONG(ring_avail_read(&ring), 0L);
        EXPECT_EQ_LONG(ring_avail_write(&ring), cap);
    }

    TEST_CASE("RingData: Two parts")
    {
        Buf temp = perm;

        u64 cap  = 16;
        u64 step = 3;

        Ring ring = {
            .cap = cap,
            .buf = make(&temp, u8, cap),
        };

        u64 bufsize = cap * 4;
        u8  buf_write[bufsize];
        u8  buf_read[bufsize];
        RANGE(i, (isize)bufsize) { buf_write[i] = i; }
        RANGE(i, (isize)bufsize) { buf_read[i] = 0; }

        EXPECT_EQ_LONG(ring_avail_read(&ring), 0L);
        EXPECT_EQ_LONG(ring_avail_write(&ring), cap);

        u64 nwrite = 0;
        u64 nread  = 0;

        RANGE(i, 0, (isize)bufsize, step)
        {
            u64 avail = step + i > bufsize ? bufsize - i : step;
            u64 read  = 0;
            u64 write = 0;

            write = ring_write(&ring, &buf_write[i], avail);
            EXPECT_EQ_LONG(write, avail);
            nwrite += write;

            read = ring_read(&ring, &buf_read[i], avail);
            EXPECT_EQ_LONG(read, avail);
            nread += read;
        }

        EXPECT_EQ_LONG(nread, bufsize);
        EXPECT_EQ_LONG(nwrite, bufsize);

        RANGE(i, (isize)bufsize) { EXPECT_EQ_INT(buf_write[i], (int)i); }
        RANGE(i, (isize)bufsize) { EXPECT_EQ_INT(buf_read[i], (int)i); }
    }

    TEST_CASE("Ring: Single Producer, Single Consumer")
    {
        Buf temp = perm;

        u64  cap  = 16;
        Ring ring = {
            .cap = cap,
            .buf = make(&temp, u8, cap),
        };

        u64 bufsize   = cap * 4;
        u8* buf_write = make(&temp, u8, bufsize);
        u8* buf_read  = make(&temp, u8, bufsize);
        RANGE(i, (isize)bufsize) { buf_write[i] = i; }
        RANGE(i, (isize)bufsize) { buf_read[i] = 0; }

        EXPECT_EQ_LONG(ring_avail_read(&ring), 0L);
        EXPECT_EQ_LONG(ring_avail_write(&ring), cap);

        int err;

        ThreadData prod = {
            .ring  = &ring,
            .buf   = buf_write,
            .chunk = 3,
            .total = bufsize,
            .ndone = 0,
        };

        ThreadData cons = {
            .ring  = &ring,
            .buf   = buf_read,
            .chunk = 3,
            .total = bufsize,
            .ndone = 0,
        };

        err = pthread_create(&prod.tid, 0, thread_producer, &prod);
        EXPECT_EQ_INT(err, 0);

        err = pthread_create(&cons.tid, 0, thread_consumer, &cons);
        EXPECT_EQ_INT(err, 0);

        err = pthread_join(prod.tid, NULL);
        EXPECT_EQ_INT(err, 0);

        err = pthread_join(cons.tid, NULL);
        EXPECT_EQ_INT(err, 0);

        RANGE(i, (isize)bufsize) { EXPECT_EQ_INT(buf_read[i], buf_write[i]); }
    }

    buf_free(&perm);

    return TEST_RESULTS();
}
