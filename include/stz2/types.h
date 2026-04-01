#pragma once

/* ---------------------------------------------------------------------------
 *  Includes
 * ------------------------------------------------------------------------- */
// For strptime
#define _XOPEN_SOURCE 700

#include <stdalign.h> // alignof
#include <stdarg.h>   // va_list, va_start, va_end
#include <stdbool.h>  // bool
#include <stddef.h>   // ptrdiff_t
#include <stdint.h>   // uint.., int..
#include <stdio.h>    // fprintf, stderr, ..
#include <stdlib.h>   // EXIT_FAILURE, ..

#include <math.h>   // IWYU pragma: keep
#include <string.h> // memset, memcpy
#include <time.h>   // clock_gettime, CLOCK_MONOTONIC, strptime, strftime

/* ---------------------------------------------------------------------------
 *  Primitive types
 * ------------------------------------------------------------------------- */

typedef uintptr_t uptr;
typedef ptrdiff_t isize;
typedef size_t    usize;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;
typedef float    f32;
typedef double   f64;

/* ---------------------------------------------------------------------------
 *  Time and dates
 * ------------------------------------------------------------------------- */
typedef struct tm       Date;
typedef u64             Time;
typedef struct timeval  TimeUsec;
typedef struct timespec TimeNsec;

/* ---------------------------------------------------------------------------
 *  Essential Macros
 * ------------------------------------------------------------------------- */

#define SI static inline

#define countof(a)  (isize)(sizeof(a) / sizeof(*(a)))
#define lengthof(s) (countof(s) - 1)

// clang-format off
#define maximum(a,b)  ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a > _b ? _a : _b; })
#define minimum(a,b)  ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a < _b ? _a : _b; })
#define previtem(a,b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a - 1 >= 0 ? _a - 1 : _b - 1; })
#define nextitem(a,b) ({ __typeof__ (a) _a = (a); __typeof__ (b) _b = (b); _a + 1 < _b ? _a + 1 : 0     ; })
// clang-format on

// For generics
#define CONCAT_(a, b) a##b
#define CONCAT(a, b)  CONCAT_(a, b)

/* ---------------------------------------------------------------------------
 *  Iteration
 * ------------------------------------------------------------------------- */

#define RANGE(...)                 RANGEx(__VA_ARGS__, RANGE4, RANGE3, RANGE2, RANGE1)(__VA_ARGS__)
#define RANGEx(a, b, c, d, e, ...) e

#define RANGE1(i)          for (isize i = 0; i < 1; i++)
#define RANGE2(i, b)       for (isize i = 0; i < (b); i++)
#define RANGE3(i, a, b)    for (isize i = (a); i < (b); i++)
#define RANGE4(i, a, b, c) for (isize i = (a); i < (b); i += (c))

/* ---------------------------------------------------------------------------
 *  Array basics
 * ------------------------------------------------------------------------- */

#define DECLARE_ARRAY(name, type)                                                                                      \
    typedef struct                                                                                                     \
    {                                                                                                                  \
        type* buf;                                                                                                     \
        isize len;                                                                                                     \
    } name;

#define DECLARE_BUFFER(name, type)                                                                                     \
    typedef struct                                                                                                     \
    {                                                                                                                  \
        type* buf;                                                                                                     \
        isize len;                                                                                                     \
        isize cap;                                                                                                     \
    } name;

#define array_first(arr) (arr).buf[0]
#define array_last(arr)  (arr).buf[(arr).len - 1]

/* ---------------------------------------------------------------------------
 *  Printing
 * ------------------------------------------------------------------------- */

// Terminal ANSI codes
#define COLOR_RESET   "\033[0m"
#define COLOR_BOLD    "\033[1m"
#define COLOR_ITALIC  "\033[3m"
#define COLOR_ULINE   "\033[4m"
#define COLOR_BLACK   "\033[0;30m"
#define COLOR_RED     "\033[0;31m"
#define COLOR_GREEN   "\033[0;32m"
#define COLOR_YELLOW  "\033[0;33m"
#define COLOR_BLUE    "\033[0;34m"
#define COLOR_MAGENTA "\033[0;35m"
#define COLOR_CYAN    "\033[0;36m"
#define COLOR_WHITE   "\033[0;37m"

#define STYLE_URL_START COLOR_BLUE COLOR_ULINE "\033]8;;"
#define STYLE_URL_MID   "\033\\"
#define STYLE_URL_END   "\033]8;;\033\\" COLOR_RESET

// Common print functions
#define p_log(fn, dst, fmt, ...) fn(dst, fmt, ##__VA_ARGS__)

#define p_reset()     p_log(fprintf, stderr, COLOR_RESET)
#define p_newline()   p_log(fprintf, stderr, "\n")
#define p_resetline() p_log(fprintf, stderr, COLOR_RESET "\n")

#define p_line(fmt, ...)             p_log(fprintf, stderr, fmt "\n", ##__VA_ARGS__)
#define p_inline(fmt, ...)           p_log(fprintf, stderr, ##__VA_ARGS__)
#define p_color(color, fmt, ...)     p_log(fprintf, stderr, color fmt COLOR_RESET, ##__VA_ARGS__)
#define p_colorline(color, fmt, ...) p_log(fprintf, stderr, color fmt COLOR_RESET "\n", ##__VA_ARGS__)
#define p_url(link, text)            p_log(fprintf, stderr, STYLE_URL_START link STYLE_URL_MID text STYLE_URL_END)

// Common styles
#define STYLE_TITLE COLOR_YELLOW COLOR_BOLD COLOR_ITALIC COLOR_ULINE
#define STYLE_INFO  COLOR_WHITE
#define STYLE_WARN  COLOR_YELLOW
#define STYLE_ERROR COLOR_RED

#define p_title(fmt, ...) p_colorline(STYLE_TITLE, fmt, ##__VA_ARGS__)
#define p_info(fmt, ...)  p_colorline(STYLE_INFO, fmt, ##__VA_ARGS__)
#define p_warn(fmt, ...)  p_colorline(STYLE_WARN, fmt, ##__VA_ARGS__)
#define p_error(fmt, ...) p_colorline(STYLE_ERROR, fmt, ##__VA_ARGS__)

/* ---------------------------------------------------------------------------
 *  Error handling
 * ------------------------------------------------------------------------- */
// clang-format off

#define OnError(log_fn, cond, behaviour, fmt, ...)                              \
    if (cond)                                                                   \
    {                                                                           \
        log_fn("%s:%-4d : %s()", __FILE_NAME__, __LINE__, __PRETTY_FUNCTION__); \
        log_fn("  " fmt, ##__VA_ARGS__);                                        \
        behaviour;                                                              \
    }

/* Error behaviours:
 *      1. Assert  : Drop to debugger
 *      2. Warn    : Do nothing
 *      3. Continue: In case of loops
 *      4. Break   : In case of loops
 *      5. Goto    : Needs label
 *      6. Return  : Needs return val, in case of function
 *      7. Fatal   : exit(1)
 */

#define Assert(cond, fmt, ...)              OnError(p_error, !(cond), __builtin_trap(), fmt, ##__VA_ARGS__)
#define OnError_Warn(cond, fmt, ...)        OnError(p_warn ,  (cond), (void)0,          fmt, ##__VA_ARGS__)
#define OnError_Continue(cond, fmt, ...)    OnError(p_warn ,  (cond), continue,         fmt, ##__VA_ARGS__)
#define OnError_Break(cond, fmt, ...)       OnError(p_warn ,  (cond), break,            fmt, ##__VA_ARGS__)
#define OnError_Goto(cond, label, fmt, ...) OnError(p_error,  (cond), goto label,       fmt, ##__VA_ARGS__)
#define OnError_Return(cond, val, fmt, ...) OnError(p_error,  (cond), return (val),     fmt, ##__VA_ARGS__)
#define OnError_Fatal(cond, fmt, ...)       OnError(p_error,  (cond), exit(1),          fmt, ##__VA_ARGS__)

// clang-format on
/* ---------------------------------------------------------------------------
 *  Buf
 * ------------------------------------------------------------------------- */

#define KB_ 1024UL
#define MB_ KB_ * 1024
#define GB_ MB_ * 1024

typedef struct
{

    char* buf;
    isize len;
    isize cap;

} Buf;

typedef enum
{
    ALLOC_NOZERO   = 0x0,
    ALLOC_ZERO     = 0x1,
    ALLOC_SOFTFAIL = 0x2,

} AllocFlags;

// OG function: Handles alignment and capacity
SI char* buf_alloc(Buf* b, usize objsize, usize align, isize count, AllocFlags flags);

// Actual way of calling, generic on type
#define make(...)                 makex(__VA_ARGS__, make4, make3, make2)(__VA_ARGS__)
#define makex(a, b, c, d, e, ...) e
#define make2(a, t)               (t*)buf_alloc(a, sizeof(t), alignof(t), 1, ALLOC_NOZERO)
#define make3(a, t, n)            (t*)buf_alloc(a, sizeof(t), alignof(t), n, ALLOC_NOZERO)
#define make4(a, t, n, f)         (t*)buf_alloc(a, sizeof(t), alignof(t), n, f)

// Lifetime
#define buf_stack(name, capacity)                                                                                      \
    char buf__##name[(capacity)] = {};                                                                                 \
    Buf  name                    = {.buf = buf__##name, .cap = (capacity), .len = 0};

SI Buf  buf_new(isize cap);
SI Buf  buf_new2(Buf* b, isize cap, AllocFlags flags);
SI void buf_reset(Buf* b);
SI void buf_free(Buf* b);

// Measurements
SI isize buf_avail(Buf* b);
SI bool  buf_ontop(Buf* b, char* buf, isize len);

/* ---------------------------------------------------------------------------
 *  Strings
 * ------------------------------------------------------------------------- */

typedef struct
{
    char* buf;
    isize len;

} Str;

typedef struct
{
    char* buf;
    isize len;

} Str0;

// clang-format off
#define NullStr  (Str) {}
#define NullStr0 (Str0){}

#define _(s)  (Str)  {.buf = s, .len = sizeof(s) - 1}
#define _0(s) (Str0) {.buf = s, .len = sizeof(s) - 1}

#define Str_(s)  (Str) {.buf = s.buf, .len = s.len}
#define Str0_(s) (Str) {.buf = s.buf, .len = s.len}

#define Str_Chars(s)  (Str){.buf = s, .len = strlen(s)}
#define IsNullTerm(s) ((s).buf[(s).len] == '\0')

#define _s(s) (int)s.len, s.buf

#define FNV_64_OFFSET_BASIS 0xcbf29ce484222325
#define FNV_64_PRIME        1099511628211

// clang-format on

// --------------- Str functions ---------------

SI Str  str_new(Buf* b, isize len);
SI bool str_equal(Str s1, Str s2);

SI Str str_copy(Buf* b, Str s, bool null_terminated);
SI Str str_fmtn(Buf* b, isize len, const char* fmt, ...) __attribute__((format(printf, 3, 4)));
SI Str str_fmt(Buf* b, const char* fmt, ...) __attribute__((format(printf, 2, 3)));

// 'Safe' substrings and comparisons
SI Str   str_sub(Str s1, isize start, isize end); // Inclusive when using negative indices
SI bool  str_startswith(Str s1, Str prefix);
SI bool  str_endswith(Str s1, Str suffix);
SI bool  str_contains(Str s1, Str sub);
SI isize str_find(Str s1, Str sub);

SI u64 str_hash64(Str s);

/* ---------------------------------------------------------------------------
 *  String derivatives
 * ------------------------------------------------------------------------- */

// Array of strings
DECLARE_ARRAY(Strs, Str);

// Key value pairs
typedef struct
{
    Str key;
    Str val;
} KeyVal;

// Array of key value pairs
DECLARE_ARRAY(KeyVals, KeyVal);

// Hashmap of strings
typedef struct
{
    KeyVal* buf;
    isize   len;
    isize   exp; ///< Exponent to power of 2

} StrMap;

/* ---------------------------------------------------------------------------
 * String operations: Trim, Split, Join
 * ------------------------------------------------------------------------- */

// Common variants
typedef enum
{
    STRS_TRIM_NONE       = 0,
    STRS_TRIM_LEFT       = 1U << 0,
    STRS_TRIM_RIGHT      = 1U << 1,
    STRS_TRIM_SPACES     = 1U << 2, // ' '
    STRS_TRIM_TABS       = 1U << 3, // '\t'
    STRS_TRIM_NEWLINES   = 1U << 4, // '\n'
    STRS_TRIM_CRETURNS   = 1U << 5, // '\r'
    STRS_TRIM_LEFTRIGHT  = STRS_TRIM_LEFT | STRS_TRIM_RIGHT,
    STRS_TRIM_WHITESPACE = STRS_TRIM_SPACES | STRS_TRIM_TABS | STRS_TRIM_NEWLINES,
    STRS_TRIM_DEFAULT    = STRS_TRIM_LEFTRIGHT | STRS_TRIM_WHITESPACE,

} StrTrimFlags;

Str str_trim(Str src, StrTrimFlags flags);
Str str_trimc(Str src, char pad);

typedef enum
{
    STRS_SPLIT_DEFAULT         = 0,
    STRS_SPLIT_IGNORE_EMPTY    = 1U << 0,
    STRS_SPLIT_SUBSTITUTE_NULL = 1U << 1,

} StrSplitFlags;

// Support for various modes of splits
Strs str_splitc(Buf* b, Str src, char sep, int maxlen, StrSplitFlags flags);
Str  str_joinc(Buf* b, Strs src, char sep);

// Split by newline and trim if needed
Strs str_lines(Buf* b, Str src, int maxlen, StrSplitFlags split_flags, StrTrimFlags trim_flags);

// Rather specific buf common type of split that requires trimming
KeyVal str_split_keyval(Str src, char sep, StrTrimFlags flags);

/* ---------------------------------------------------------------------------
 *  Implementation
 * ------------------------------------------------------------------------- */

// --------------- Buf ---------------

// Allocation
SI char* buf_alloc(Buf* b, usize objsize, usize align, isize count, AllocFlags flags)
{
    Assert((count >= 0), "Failed: buf_alloc(%ld)", count);

    char* beg = &b->buf[b->len];

    if (count == 0) return beg;

    isize pad = -(uptr)beg & (align - 1); // Works as align is power of 2
    if (count > (b->cap - b->len) / (isize)objsize)
    {
        if (flags & ALLOC_SOFTFAIL) return 0;
        else Assert(true, "Failed: buf_alloc: %ld * %ld < %ld", count, objsize, b->cap - b->len);
    }

    isize total  = count * objsize;
    char* p      = beg + pad;
    b->len      += pad + total;
    if ((flags & ALLOC_ZERO)) p = (char*)memset(p, 0, total);

    return p;
}

// Lifetime
SI Buf buf_new(isize cap)
{
    Buf b = {};
    b.buf = malloc(cap);
    b.len = 0;
    b.cap = b.buf ? cap : 0;
    return b;
}
SI Buf buf_new2(Buf* b, isize cap, AllocFlags flags)
{
    Buf dst = {};
    dst.buf = buf_alloc(b, sizeof(char), alignof(char), cap, flags);
    dst.len = 0;
    dst.cap = dst.buf ? cap : 0;
    return dst;
}
SI void buf_reset(Buf* b) { b->len = 0; }
SI void buf_free(Buf* b)
{
    if (b->buf) free(b->buf);
    *b = (Buf){};
}

// Measurements
SI isize buf_avail(Buf* b) { return b->cap - b->len; }
SI bool  buf_ontop(Buf* b, char* buf, isize len) { return (b->len - len) == (buf - b->buf); }

// --------------- Str ---------------

SI Str str_new(Buf* b, isize len)
{
    return (Str){
        .buf = make(b, char, len),
        .len = len,
    };
}

SI bool str_equal(Str s1, Str s2)
{
    if (s1.len != s2.len) { return false; }
    if (s1.len == 0) { return true; }
    return !memcmp(s1.buf, s2.buf, s1.len);
}

SI Str str_sub(Str src, isize i, isize j)
{
    isize _len = src.len;
    if ((i >= _len) || (j <= -1 * _len)) { return NullStr; }
    if ((i <= -1 * _len) && (j >= _len)) { return src; }
    if (j >= _len) { j = _len; }
    if (i <= -1 * _len) { i = -1 * _len; }
    if (i < 0) { i = _len + i; }
    if (j < 0) { j = _len + j + 1; } // Make it inclusive, as listing to -1 is most common

    if (i >= j) { return (Str){.buf = &src.buf[i], 0}; };
    return (Str){.buf = &src.buf[i], .len = j - i};
}

SI bool str_startswith(Str s1, Str prefix)
{
    if (s1.len < prefix.len) return false;
    return str_equal(str_sub(s1, 0, prefix.len), prefix);
}

SI bool str_endswith(Str s1, Str suffix)
{
    if (s1.len < suffix.len) return false;
    return str_equal(str_sub(s1, s1.len - suffix.len, s1.len), suffix);
}

SI bool str_contains(Str s1, Str sub)
{
    if (!sub.len) return true; // Always contains NullStr
    RANGE(i, s1.len)
    {
        if (!(s1.buf[i] == sub.buf[0])) continue;
        return str_equal((Str){&s1.buf[i], s1.len - i}, sub);
    }
    return false;
}

SI isize str_find(Str s1, Str sub)
{
    if (!sub.len) return 0; // Always finds NullStr
    RANGE(i, s1.len)
    {
        if (!(s1.buf[i] == sub.buf[0])) continue;
        if (str_equal((Str){&s1.buf[i], s1.len - i}, sub)) { return i; }
        else return -1;
    }
    return -1;
}

SI Str str_copy(Buf* a, Str s, bool null_term)
{
    if (!s.len) return NullStr;                                           // Null case
    Str c = {.buf = make(a, char, s.len + (int)null_term), .len = s.len}; // Alloc
    memcpy(c.buf, s.buf, c.len);                                          // Copy string
    if (null_term) c.buf[c.len] = '\0'; // Set null if needed since we init with ARENA_NOZERO
    return c;
}

SI Str str_fmtn(Buf* b, isize len, char const* fmt, ...)
{
    Str s = {.buf = make(b, char, len, ALLOC_NOZERO), .len = 0};

    va_list arg;
    va_start(arg, fmt);
    s.len = vsnprintf(s.buf, len, fmt, arg);
    va_end(arg);

    b->len -= (len - s.len);
    return s;
}

SI Str str_fmt(Buf* b, char const* fmt, ...)
{
    isize len = buf_avail(b);
    Str   s   = {.buf = make(b, char, len, ALLOC_NOZERO), .len = 0};

    va_list arg;
    va_start(arg, fmt);
    s.len = vsnprintf(s.buf, len, fmt, arg);
    va_end(arg);

    b->len -= (len - s.len);
    return s;
}

u64 str_hash64(Str s)
{
    u64 h = FNV_64_OFFSET_BASIS;
    RANGE(i, s.len)
    {
        h ^= s.buf[i] & 255;
        h *= FNV_64_PRIME;
    }
    return h;
}

/* ---------------------------------------------------------------------------
 * String operations
 * ------------------------------------------------------------------------- */

Str str_trim(Str src, StrTrimFlags flags)
{
    if (!src.len) return src;

    isize start = 0;
    if (flags & STRS_TRIM_LEFT)
    {
        RANGE(i, src.len)
        {
            if ((src.buf[i] == ' ') && (flags & STRS_TRIM_SPACES)) continue;
            else if ((src.buf[i] == '\t') && (flags & STRS_TRIM_TABS)) continue;
            else if ((src.buf[i] == '\n') && (flags & STRS_TRIM_NEWLINES)) continue;
            else if ((src.buf[i] == '\r') && (flags & STRS_TRIM_CRETURNS)) continue;
            else
            {
                start = i;
                break;
            }
        }
    }

    isize stop = src.len;
    if (flags & STRS_TRIM_RIGHT)
    {
        for (isize i = src.len - 1; i >= 0; i--)
        {
            if ((src.buf[i] == ' ') && (flags & STRS_TRIM_SPACES)) continue;
            else if ((src.buf[i] == '\t') && (flags & STRS_TRIM_TABS)) continue;
            else if ((src.buf[i] == '\n') && (flags & STRS_TRIM_NEWLINES)) continue;
            else if ((src.buf[i] == '\r') && (flags & STRS_TRIM_CRETURNS)) continue;
            else
            {
                stop = i + 1;
                break;
            }
        }
    }

    return (Str){
        .buf = &src.buf[start],
        .len = stop - start,
    };
}

KeyVal str_split_keyval(Str src, char sep, StrTrimFlags flags)
{
    if (!src.len) return (KeyVal){};

    isize found = -1;
    RANGE(i, src.len)
    {
        if (!(src.buf[i] == sep)) continue;
        found = i;
        break;
    }

    // If not found: Probably means key with no val
    // e.g. with sep ':'
    //      src is `hello ` instead of `hello: hi`
    //      so return `hello`,``
    if (found < 0) return (KeyVal){str_trim(src, flags), {}};

    KeyVal keyval = {
        .key = {.buf = src.buf, .len = found},
        .val = {.buf = &src.buf[found + 1], .len = src.len - found - 1},
    };

    keyval.key = str_trim(keyval.key, flags);
    keyval.val = str_trim(keyval.val, flags);

    return keyval;
}

Strs str_splitc(Buf* b, Str src, char sep, int maxlen, StrSplitFlags flags)
{
    if (!maxlen) return (Strs){};

    char* start = &src.buf[0];
    Strs  parts = {.buf = make(b, Str, maxlen, ALLOC_NOZERO), .len = maxlen};
    isize count = 0;
    RANGE(i, src.len)
    {
        if (sep == src.buf[i])
        {
            isize len = &src.buf[i] - start;
            if (len || !(flags & STRS_SPLIT_IGNORE_EMPTY))
            {
                parts.buf[count] = (Str){.buf = start, .len = len};
                count++;
                if (count >= maxlen) goto __done;
            }
            if (flags & STRS_SPLIT_SUBSTITUTE_NULL) src.buf[i] = '\0';
            start = &src.buf[i] + 1; // Skip delimiter
        }
    }

    if ((isize)(start - src.buf) <= src.len)
    {
        isize len = src.len - (start - src.buf);
        if (len || !(flags & STRS_SPLIT_IGNORE_EMPTY))
        {
            parts.buf[count] = (Str){.buf = start, .len = len};
            count++;
        }
    }

__done:
    parts.len  = count;
    b->len    -= sizeof(Str) * (maxlen - count);

    return parts;
}
