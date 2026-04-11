#include "stz2/rt/atomic.h"

#include "stz2/test.h"

#include <assert.h>    // assert
#include <pthread.h>   // pthread_create, pthread_join
#include <stdatomic.h> // atomic_int
#include <unistd.h>    // usleep

#define NUM_READERS 4
#define NUM_WRITES  100
#define NUM_FLOATS  16

#define log_fn p_error

typedef struct
{
    float a[NUM_FLOATS];
} Floats;

DECLARE_ATOMIC_STATE(Floats, AtomicFloats);

typedef struct
{
    AtomicFloats state;
    atomic_int   stop_flag;
} ThreadData;

void* reader_thread_stress(void* data)
{
    ThreadData* d = data;
    while (!atomic_load(&d->stop_flag))
    {
        int     switched;
        Floats* val = ATOMIC_STATE_TRY_SWITCH(d->state, &switched);

        RANGE(i, NUM_FLOATS)
        {

            OnError_Return((val->a[i] < 0.0f && val->a[i] > NUM_WRITES), NULL, //
                           "Outside possible: %f", val->a[i]);

            OnError_Warn((val->a[0] != val->a[i]),            //
                         "Inconsistent state: %ld: %f != %f", //
                         i, val->a[0], val->a[i]);
        }

        usleep(1);
    }
    return NULL;
}

#define FILL(state, value) RANGE(_f, NUM_FLOATS)(state)->a[_f] = value;

int main(int argc, char** argv)
{
    TEST_SUITE("stz-rt/atomic.h");

    TEST_CASE("basic write/read")
    {
        AtomicFloats f = {.counter.v = 0, .write_depth = 0};
        FILL(&f.state[0], 42.0f);
        FILL(&f.state[1], 0.0f);

        Floats* w = ATOMIC_STATE_WRITE_START(f);
        FILL(w, 100.0f);
        ATOMIC_STATE_WRITE_STOP(f);

        int     switched;
        Floats* r = ATOMIC_STATE_TRY_SWITCH(f, &switched);
        RANGE(i, NUM_FLOATS) EXPECT_EQ_FLOAT(r->a[i], 100.0f);
        EXPECT_FALSE(atomic_state_is_pending(&f.counter));
    }

    TEST_CASE("single reader single writer")
    {
        AtomicFloats f = {.counter.v = 0, .write_depth = 0};
        FILL(&f.state[0], 10.0f);
        FILL(&f.state[1], 0.0f);

        Floats* w = ATOMIC_STATE_WRITE_START(f);
        FILL(w, 20.0f);

        Floats* r = ATOMIC_STATE_READ_CURRENT(f);
        RANGE(i, NUM_FLOATS) EXPECT_EQ_FLOAT(r->a[i], 10.0f); // read sees old value

        ATOMIC_STATE_WRITE_STOP(f);

        int switched;
        r = ATOMIC_STATE_TRY_SWITCH(f, &switched);
        EXPECT_TRUE(switched);
        RANGE(i, NUM_FLOATS) EXPECT_EQ_FLOAT(r->a[i], 20.0f);
    }

    TEST_CASE("multiple readers single writer")
    {
        ThreadData shared = {.state = {.counter.v = 0, .write_depth = 0}};
        FILL(&shared.state.state[0], 0.0f);
        FILL(&shared.state.state[1], 0.0f);
        atomic_store(&shared.stop_flag, 0);

        pthread_t readers[NUM_READERS];
        RANGE(i, NUM_READERS) pthread_create(&readers[i], NULL, reader_thread_stress, &shared);

        RANGE(i, NUM_WRITES)
        {
            Floats* w = ATOMIC_STATE_WRITE_START(shared.state);
            FILL(w, (float)i);
            ATOMIC_STATE_WRITE_STOP(shared.state);
            usleep(1);
        }

        atomic_store(&shared.stop_flag, 1);
        RANGE(i, NUM_READERS) { pthread_join(readers[i], NULL); }

        // Readers already use TRY_SWITCH
        Floats* final_val = ATOMIC_STATE_READ_CURRENT(shared.state);
        RANGE(i, NUM_FLOATS) { EXPECT_EQ_FLOAT(final_val->a[i], (float)(NUM_WRITES - 1)); }
    }

    TEST_CASE("nested writes")
    {
        AtomicFloats f = {.counter.v = 0, .write_depth = 0};
        FILL(&f.state[0], 0.0f);
        FILL(&f.state[1], 0.0f);

        Floats* w1 = ATOMIC_STATE_WRITE_START(f);
        RANGE(i, NUM_FLOATS) { w1->a[i] = 10.0f; }

        Floats* w2 = ATOMIC_STATE_WRITE_START(f);
        RANGE(i, NUM_FLOATS) { w2->a[i] += 5.0f; }

        ATOMIC_STATE_WRITE_STOP(f); // nested stop
        ATOMIC_STATE_WRITE_STOP(f); // outer stop

        int     switched;
        Floats* r = ATOMIC_STATE_TRY_SWITCH(f, &switched);
        RANGE(i, NUM_FLOATS) { EXPECT_EQ_FLOAT(r->a[i], 15.0f); }
    }

    return TEST_RESULTS();
}
