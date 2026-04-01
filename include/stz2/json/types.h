#pragma once

#include "stz2/types.h"

// clang-format off
// --------------- Buffer ---------------

typedef struct
{
    char* buf;
    isize len;
    isize pos;
    int   err;

} Buffer;

#define BufAt(p, len_) (p->buf[p->pos + (len_)])

#define BufFromStr(s)      (Buffer){.buf = s.buf, .len = s.len}
#define BufFromBuffer(b)   (Buffer){.buf = &b->buf[b->pos], .len = b->len - b->pos}
#define BufFromStack(b, n) char buf__##b[(n)] = {}; Buffer b = {.buf = buf__##b, .len = (n)};

#define BufToStr(p, len_) (Str){.buf = &p->buf[p->pos - (len_)], .len = (len_)}

#define BufCheckIf(p, cond, val) if ((cond)) { p->err = -1; return val; }
#define BufCheckCap(p, cap, val) BufCheckIf(p, (p->pos + (cap) > p->len), val)

// NOTE: !! expr should modify bb !! e.g. `BufSafeConcat(b, (&bb), concat__string((&bb), s))`
#define BufSafeConcat( b, bb, expr) ({(expr); if (bb->err) {b->err = bb->err; return BufToStr(bb, bb->pos);}})
#define BufSafeConsume(b, bb, expr) ({(expr); if (bb->err) {b->err = bb->err; return (Str){};}})

// clang-format on
