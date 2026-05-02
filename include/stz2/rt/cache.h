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
