#pragma once

#include "stz2/types.h"

typedef struct CacheVal CacheVal;

typedef struct CacheVal
{
    isize idx; // Ref back to StrSet
    Str   key; // Why not

    CacheVal* older; // Doubly linked list
    CacheVal* newer; //

    Time loaded; // Recency
    Time used;   //

    void* data;     // User data
    isize size;     // Expected size (err=-1)
    isize progress; // Async indication (min=0, max=size, err=-1)

} CacheVal;

typedef struct
{
    StrSet     keys; // Indexes into `vals`
    CacheVal** vals;

    CacheVal* oldest; // Linked list
    CacheVal* newest;

    Buf mem; // We want nodes to be in scope when using Cache
} Cache;

// Internals
SI CacheVal* cache_lookup(Cache* c, Str key);
SI int       cache_insert(Cache* c, Str key, CacheVal* val);
SI int       cache_delete(Cache* c, Str key);

// User-facing
SI int  cache_new(Buf* b, Cache* c, isize capacity_mem, isize capacity_set_exp);
SI void cache_free(Cache* c);

SI CacheVal* cache_get(Cache* c, Str key);

// --------------- Implementation ---------------

SI int cache_new(Buf* b, Cache* c, isize capacity_mem, isize capacity_set_exp)
{
    c->keys = strset_new(b, capacity_set_exp);
    c->mem  = buf_new(capacity_mem);
    return 0;
}

SI void cache_free(Cache* c) { buf_free(&c->mem); }

SI CacheVal* cache_lookup(Cache* c, Str key)
{
    int idx = strset_lookup(&c->keys, key);
    if (!idx) return NULL;
    return c->vals[idx];
}

SI int cache_insert(Cache* c, Str key, CacheVal* val)
{
    int idx = strset_insert(&c->keys, key);
    if (!idx) return -1;
    c->vals[idx] = val;
    return 0;
}

SI int cache_delete(Cache* c, Str key)
{
    int idx = strset_delete(&c->keys, key);
    if (idx < 0) return -1;
    c->vals[idx] = NULL;
    return 0;
}

// User-facing
SI CacheVal* cache_get(Cache* c, Str key)
{
    (void)c;
    (void)key;

    // 1. Check if already exists
    //      1. Then pop to head of list
    //      2. Return value
    // 2. Else
    //      1. Load content, return null on fail
    //      2. If already full, remove tail
    //      3. Insert key
    //      4. Insert at head of list
    //      5. Return value

    return NULL;
}
