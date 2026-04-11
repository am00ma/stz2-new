#pragma once

#include "stz2/types.h"

typedef union {
    struct
    {
        u16 cur;
        u16 next;
    } s;
    u32 v;
} AtomicCounter;

#define COUNTER(e)          ((e).v)
#define CUR_INDEX(e)        ((e).s.cur)
#define NEXT_INDEX(e)       ((e).s.next)
#define CUR_ARRAY_INDEX(e)  (CUR_INDEX(e) & 1u)
#define NEXT_ARRAY_INDEX(e) ((CUR_INDEX(e) + 1u) & 1u)

#define DECLARE_ATOMIC_STATE(type, name)                                                                               \
    typedef struct                                                                                                     \
    {                                                                                                                  \
        type state[2];                                                                                                 \
        alignas(4) volatile AtomicCounter counter;                                                                     \
        i32 write_depth;                                                                                               \
    } name;

int      atomic_state_is_pending(volatile AtomicCounter* counter);
uint16_t atomic_state_get_current_index(volatile AtomicCounter* counter);

void* atomic_state_write_start(void* state, volatile AtomicCounter* counter, int32_t* depth, size_t elem_size);
void  atomic_state_write_stop(volatile AtomicCounter* counter, int32_t* depth);

void* atomic_state_read_current(void* state, volatile AtomicCounter* counter, size_t elem_size);
void* atomic_state_try_switch(void* state, volatile AtomicCounter* counter, size_t elem_size, int* switched);

static inline int cas_u32(u32* addr, u32 expected, u32 desired)
{
    return __atomic_compare_exchange_n( //
        addr,                           //
        &expected,                      //
        desired,                        //
        0,                              //
        __ATOMIC_ACQ_REL,               //
        __ATOMIC_RELAXED                //
    );
}

// ----------------------------------------
// ----- User facing API -----
// ----------------------------------------

// clang-format off
#define ATOMIC_STATE_WRITE_START(s)          (typeof((s).state[0])*)atomic_state_write_start( (s).state,   &(s).counter, &(s).write_depth, sizeof((s).state[0]))
#define ATOMIC_STATE_WRITE_STOP(s)                                  atomic_state_write_stop( &(s).counter, &(s).write_depth)
#define ATOMIC_STATE_READ_CURRENT(s)         (typeof((s).state[0])*)atomic_state_read_current((s).state,   &(s).counter, sizeof((s).state[0]))
#define ATOMIC_STATE_TRY_SWITCH(s, switched) (typeof((s).state[0])*)atomic_state_try_switch(  (s).state,   &(s).counter, sizeof((s).state[0]), switched)
// clang-format on
