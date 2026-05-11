#include "stz2/types.h"

typedef struct
{
    Str*  buf;
    isize len;
    isize exp; ///< Exponent to power of 2

} RtStrSet;

SI RtStrSet rt_strset_new(Buf* b, isize exp);
SI int      rt_strset_lookup(RtStrSet* m, Str key);
SI int      rt_strset_insert(RtStrSet* m, Str key);
SI int      rt_strset_delete(RtStrSet* m, Str key);

#define rt_strset_grave (Str){.buf = 0, .len = -1}

// Mask, step, index -> can use for your own hashmap
SI int rt_strset_next(u64 hash, int exp, int i)
{
    unsigned mask = (1 << exp) - 1;
    unsigned step = hash >> (64 - exp) | 1;
    return (i + step) & mask;
}

SI RtStrSet rt_strset_new(Buf* b, isize exp)
{
    return (RtStrSet){
        // ALLOC_ZERO needed to set .buf = 0 which marks empty slot
        .buf = make(b, Str, (1 << exp), ALLOC_ZERO),
        .len = 0,
        .exp = exp,
    };
}

SI int rt_strset_lookup(RtStrSet* m, Str key)
{
    if (!m->len) { return -1; } // empty set

    u64 hash  = str_hash64(key);
    i32 count = 0;
    for (i32 i = hash;;)
    {
        i = rt_strset_next(hash, m->exp, i);
        // TODO: Atomic load for key -> Str
        if (!m->buf[i].buf && !m->buf[i].len) { return -1; }            // found empty slot
        else if (!m->buf[i].buf && (m->buf[i].len == -1)) { continue; } // found gravestone
        else if (str_equal(key, m->buf[i])) { return i; }               // found filled slot
        if ((count++) >= m->len) { return -1; } // BUG: necessary? no slot found after full iteration
    }
}

SI int rt_strset_insert(RtStrSet* m, Str key)
{
    if ((m->len + 1) > (1 << m->exp)) { return -1; } // overflows capacity

    u64 hash = str_hash64(key);
    for (i32 i = hash;;)
    {
        i = rt_strset_next(hash, m->exp, i);
        // TODO: Atomic load for key -> Str
        if (!m->buf[i].buf) // found empty slot or gravestone, insert
        {
            m->buf[i] = key;
            m->len++;
            return i;
        }
        else if (str_equal(key, m->buf[i])) // found filled slot, overwrite
        {
            return i;
        }
    }
}

SI int rt_strset_delete(RtStrSet* m, Str key)
{
    // NOTE: Should not both lookup and store be atomic?
    int idx = rt_strset_lookup(m, key);
    if (idx < 0) { return -1; } // not found
    // TODO: Atomic store for key -> Str
    m->buf[idx] = (Str){.buf = 0, .len = -1}; // insert gravestone
    m->len--;
    return idx;
}
