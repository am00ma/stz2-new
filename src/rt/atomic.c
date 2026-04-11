#include "stz2/rt/atomic.h"

int atomic_state_is_pending(volatile AtomicCounter* counter) { return CUR_INDEX(*counter) != NEXT_INDEX(*counter); }
u16 atomic_state_get_current_index(volatile AtomicCounter* counter) { return CUR_INDEX(*counter); }

/*
 * Steps:
 *     1. Copy provided counter to oldv, init newv to oldv
 *     2. Store necessary info to copy if needed
 *     3. Ensure that next generation is set to current to hide switch while writing
 *     4. Copy if necessary
 */
u32 write_next_state_start_aux(volatile AtomicCounter* counter, void* state, usize elem_size)
{
    AtomicCounter oldv, newv;
    u32           cur_i, next_i;
    int           need_copy;

    do
    {
        oldv = *counter;
        newv = oldv;

        cur_i     = CUR_ARRAY_INDEX(newv);
        next_i    = NEXT_ARRAY_INDEX(newv);
        need_copy = (CUR_INDEX(newv) == NEXT_INDEX(newv));

        NEXT_INDEX(newv) = CUR_INDEX(newv);

    } while (!cas_u32((u32*)counter, oldv.v, newv.v));

    __atomic_thread_fence(__ATOMIC_ACQUIRE);

    if (need_copy)
    {
        memcpy(                                //
            (char*)state + next_i * elem_size, //
            (char*)state + cur_i * elem_size,  //
            elem_size);
    }

    return next_i;
}

/*
 * Steps:
 *     1. Copy provided counter to oldv, init newv to oldv
 *     2. Update the 'generation'
 *     3. atomic_state_is_pending returns true for readers
 */
void write_next_state_stop_aux(volatile AtomicCounter* counter)
{
    AtomicCounter oldv, newv;

    __atomic_thread_fence(__ATOMIC_RELEASE);

    do
    {
        oldv = *counter;
        newv = oldv;
        NEXT_INDEX(newv)++;
    } while (!cas_u32((u32*)counter, oldv.v, newv.v));
}

/*
 * Steps:
 *     1. Keep track of depth while starting
 */
void* atomic_state_write_start(void* state, volatile AtomicCounter* counter, i32* depth, usize elem_size)
{
    u32 idx;

    if ((*depth)++ == 0) idx = write_next_state_start_aux(counter, state, elem_size);
    else idx = NEXT_ARRAY_INDEX(*counter);

    return (char*)state + idx * elem_size;
}

/*
 * Steps:
 *     1. Keep track of depth before stopping
 */
void atomic_state_write_stop(volatile AtomicCounter* counter, i32* depth)
{
    if (--(*depth) == 0) write_next_state_stop_aux(counter);
}

/*
 * Steps:
 *     1. Simply read current
 */
void* atomic_state_read_current(void* state, volatile AtomicCounter* counter, usize elem_size)
{
    __atomic_thread_fence(__ATOMIC_ACQUIRE);
    return (char*)state + CUR_ARRAY_INDEX(*counter) * elem_size;
}

/*
 * Steps:
 *     1. Switch if new generation is ready (signalling to other readers)
 *     2. Read new generation
 */
void* atomic_state_try_switch(void* state, volatile AtomicCounter* counter, usize elem_size, int* switched)
{
    AtomicCounter oldv, newv;

    do
    {
        oldv = *counter;
        newv = oldv;

        if (switched) *switched = (CUR_INDEX(newv) != NEXT_INDEX(newv));

        CUR_INDEX(newv) = NEXT_INDEX(newv);

    } while (!cas_u32((u32*)counter, oldv.v, newv.v));

    __atomic_thread_fence(__ATOMIC_ACQUIRE);

    return (char*)state + CUR_ARRAY_INDEX(*counter) * elem_size;
}
