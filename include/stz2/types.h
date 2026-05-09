#pragma once

/* ---------------------------------------------------------------------------
 *  Includes
 * ------------------------------------------------------------------------- */
// For strptime
#define _DEFAULT_SOURCE

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

typedef void*       Any;
typedef char*       Chars;
typedef const char* CChars;

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

// Compared to RANGE, this is much more convoluted, due to not being able to define 2 variables in the initial for statement.
// Defining outside will violate scope assumptions
#define FOREACH(...)                 FOREACHx(__VA_ARGS__, FOREACH4, FOREACH3, FOREACH2, FOREACH1)(__VA_ARGS__)
#define FOREACHx(a, b, c, d, e, ...) e

#define FOREACH4(i, x, el, arr)                                                                                        \
    for (                                                                                                              \
        struct {                                                                                                       \
            isize                 i;                                                                                   \
            __typeof__((arr).buf) x;                                                                                   \
        } el = {};                                                                                                     \
        (el.x = el.i < (arr).len ? &(arr).buf[el.i] : 0), el.i < (arr).len; el.i++)
#define FOREACH3(i, el, arr) FOREACH4(i, x, el, arr)
#define FOREACH2(i, arr)     FOREACH4(i, x, el, arr)
#define FOREACH1(arr)        FOREACH4(i, x, el, arr)

/* ---------------------------------------------------------------------------
 *  Array basics
 * ------------------------------------------------------------------------- */

#define DECLARE_ARRAY(name, type)                                                                                      \
    typedef struct                                                                                                     \
    {                                                                                                                  \
        type* buf;                                                                                                     \
        isize len;                                                                                                     \
    } name;

#define DECLARE_VECTOR(name, type)                                                                                     \
    typedef struct                                                                                                     \
    {                                                                                                                  \
        type* buf;                                                                                                     \
        isize len;                                                                                                     \
        isize cap;                                                                                                     \
    } name;

#define arr_new(name, buffer, type, n, flags)                                                                          \
    (name) { .buf = make(buffer, type, (n), flags), .len = (n) }

#define arr_shrink(arr, buffer, newlen)                                                                                \
    Assert(buf_ontop(buffer, (arr).buf, ((arr).len) * sizeof(__typeof__(*(arr).buf))), "");                            \
    (buffer)->len -= ((arr).len - (newlen)) * sizeof(__typeof__(*(arr).buf));                                          \
    (arr).len      = (newlen);

#define arr_first(arr) (arr).buf[0]
#define arr_last(arr)  (arr).buf[(arr).len - 1]
#define arr_sub(arr, i, j)                                                                                             \
    (__typeof__(arr)) { .buf = &arr.buf[(i)], .len = (j - i) }

#define vec_first(vec) (vec).buf[0]
#define vec_last(vec)  (vec).buf[(vec).len - 1]

// Common arrays
DECLARE_ARRAY(u32s, u32);
DECLARE_ARRAY(u64s, u64);
DECLARE_ARRAY(i32s, i32);
DECLARE_ARRAY(i64s, i64);
DECLARE_ARRAY(f32s, f32);
DECLARE_ARRAY(f64s, f64);

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
#define p_inline(fmt, ...)           p_log(fprintf, stderr, fmt, ##__VA_ARGS__)
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
SI isize buf_avail(Buf* b, usize objsize);
SI bool  buf_ontop(Buf* b, void* buf, isize len);

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
#define StrNull  (Str) {}
#define NullStr0 (Str0){}

#define _(s)  (Str)  {.buf = s, .len = sizeof(s) - 1}
#define _0(s) (Str0) {.buf = s, .len = sizeof(s) - 1}

#define Str_(s)  (Str) {.buf = s.buf, .len = s.len}
#define Str0_(s) (Str0){.buf = s.buf, .len = s.len}

#define Str_Chars(s)  (Str) {.buf = s, .len = strlen(s)}
#define Str0_Chars(s) (Str0){.buf = s, .len = strlen(s)}
#define IsNullTerm(s) ((s).buf[(s).len] == '\0')

// Safe for printing
#define _s(s) (int)(s.buf ? s.len : 0), (s.buf ? s.buf : "")

#define FNV_64_OFFSET_BASIS 0xcbf29ce484222325
#define FNV_64_PRIME        1099511628211

// clang-format on

// --------------- Str functions ---------------

SI Str  str_new(Buf* b, isize len);
SI bool str_equal(Str s1, Str s2);

// 'Safe' substrings and comparisons
SI Str   str_sub(Str s1, isize start, isize end); // Inclusive when using negative indices
SI bool  str_startswith(Str s1, Str prefix);
SI bool  str_endswith(Str s1, Str suffix);
SI bool  str_contains(Str s1, Str sub);
SI isize str_find(Str s1, Str sub);

// Lifetime
SI Str str_copy(Buf* b, Str s, bool null_terminated);

// Format strings
SI Str str_fmtn(Buf* b, isize len, const char* fmt, ...) __attribute__((format(printf, 3, 4)));
SI Str str_fmt(Buf* b, const char* fmt, ...) __attribute__((format(printf, 2, 3)));
SI Str str_repeat(Buf* b, char c, isize len);

// Str0 Format strings
SI Str0 str0_fmtn(Buf* b, isize len, const char* fmt, ...) __attribute__((format(printf, 3, 4)));
SI Str0 str0_fmt(Buf* b, const char* fmt, ...) __attribute__((format(printf, 2, 3)));

// Iterator style
SI Str str_till_next(Str* src, char c);

// Hashmaps
SI u64 str_hash64(Str s);

// TODO: Where does this fit?
SI Str buf_shrink(Buf* src, Buf* sub)
{
    src->len -= sub->cap - sub->len;
    sub->cap  = sub->len;
    return Str_((*sub));
}

/* ---------------------------------------------------------------------------
 *  String derivatives
 * ------------------------------------------------------------------------- */

// Array of strings
DECLARE_ARRAY(Strs, Str);
DECLARE_ARRAY(Str0s, Str0);

// Key value pairs
typedef struct
{
    Str key;
    Str val;
} KeyVal;

DECLARE_ARRAY(KeyVals, KeyVal);

// Key idx pairs
typedef struct
{
    Str key;
    i64 idx;
} KeyIdx;

DECLARE_ARRAY(KeyIdxs, KeyIdx);

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
    STRS_TRIM_WHITESPACE = STRS_TRIM_SPACES | STRS_TRIM_TABS | STRS_TRIM_NEWLINES | STRS_TRIM_CRETURNS,
    STRS_TRIM_DEFAULT    = STRS_TRIM_LEFTRIGHT | STRS_TRIM_WHITESPACE,

} StrTrimFlags;

// Common things to trim
SI Str str_trim(Str src, StrTrimFlags flags);

typedef enum
{
    STRS_SPLIT_DEFAULT         = 0,
    STRS_SPLIT_IGNORE_EMPTY    = 1U << 0,
    STRS_SPLIT_SUBSTITUTE_NULL = 1U << 1,

} StrSplitFlags;

// Rather specific buf common type of split that requires trimming
SI KeyVal str_split_keyval(Str src, char sep, StrTrimFlags flags);

// Support for various modes of splits
SI Strs str_splitc(Buf* b, Str src, char sep, isize maxlen, StrSplitFlags flags);

// Split by newline with option to ignore
SI Strs str_split_lines(Buf* b, Str src, isize maxlen, bool ignore_empty);

/* ---------------------------------------------------------------------------
 * String hashmap
 * ------------------------------------------------------------------------- */

typedef struct
{
    KeyVal* buf;
    isize   len;
    isize   exp; ///< Exponent to power of 2

} StrMap;

SI StrMap strmap_new(Buf* b, isize exp);
SI Str*   strmap_lookup(StrMap* m, Str key);
SI int    strmap_delete(StrMap* m, Str key);
SI int    strmap_insert(StrMap* m, Str key, Str val);

typedef struct
{
    Str*  buf;
    isize len;
    isize exp; ///< Exponent to power of 2

} StrSet;

SI StrSet strset_new(Buf* b, isize exp);
SI int    strset_lookup(StrSet* m, Str key);
SI int    strset_insert(StrSet* m, Str key);
SI int    strset_delete(StrSet* m, Str key);

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
SI isize buf_avail(Buf* b, usize objsize) { return (b->cap - b->len) / objsize; }
SI bool  buf_ontop(Buf* b, void* buf, isize len) { return (b->len - len) == ((char*)buf - b->buf); }

// --------------- Str ---------------

SI Str str_new(Buf* b, isize len)
{
    return (Str){
        .buf = make(b, char, len, ALLOC_NOZERO),
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
    if ((i >= _len) || (j <= -1 * _len)) { return StrNull; }
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

SI bool str_contains(Str s1, Str sub) { return str_find(s1, sub) >= 0; }

SI isize str_find(Str s1, Str sub)
{
    if (!sub.len) return 0; // Always finds NullStr
    if (sub.len > s1.len) return -1;
    RANGE(i, s1.len)
    {
        if (!(s1.buf[i] == sub.buf[0])) continue;
        if (str_equal((Str){&s1.buf[i], sub.len}, sub)) { return i; }
    }
    return -1;
}

SI Str str_copy(Buf* a, Str s, bool null_term)
{
    if (!s.len) return StrNull;                                                         // Null case
    Str c = {.buf = make(a, char, s.len + (int)null_term, ALLOC_NOZERO), .len = s.len}; // Alloc
    memcpy(c.buf, s.buf, c.len);                                                        // Copy string
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
    isize len = buf_avail(b, sizeof(char));
    Str   s   = {.buf = make(b, char, len, ALLOC_NOZERO), .len = 0};

    va_list arg;
    va_start(arg, fmt);
    s.len = vsnprintf(s.buf, len, fmt, arg);
    va_end(arg);

    b->len -= (len - s.len);
    return s;
}

SI Str str_repeat(Buf* b, char c, isize len)
{
    Str dst = str_new(b, len);
    memset(dst.buf, c, len);
    return dst;
}

SI Str str_till_next(Str* src, char c)
{
    char* start = src->buf;
    while (src->buf[0] != c && src->len > 0)
    {
        src->buf++;
        src->len--;
    }
    Str dst   = {start, src->buf - start};
    src->buf += src->len > 0;
    src->len -= src->len > 0;
    return dst;
}

SI u64 str_hash64(Str s)
{
    u64 h = FNV_64_OFFSET_BASIS;
    RANGE(i, s.len)
    {
        h ^= s.buf[i] & 255;
        h *= FNV_64_PRIME;
    }
    return h;
}

// --------------- Str0 ---------------

SI Str0 str0_fmtn(Buf* b, isize len, char const* fmt, ...)
{
    Str0 s = {.buf = make(b, char, len, ALLOC_NOZERO), .len = 0};

    va_list arg;
    va_start(arg, fmt);
    s.len = vsnprintf(s.buf, len, fmt, arg);
    va_end(arg);

    b->len -= (len - s.len);

    b->buf[b->len] = '\0';
    b->len++;
    return s;
}

SI Str0 str0_fmt(Buf* b, char const* fmt, ...)
{
    isize len = buf_avail(b, sizeof(char));
    Str0  s   = {.buf = make(b, char, len, ALLOC_NOZERO), .len = 0};

    va_list arg;
    va_start(arg, fmt);
    s.len = vsnprintf(s.buf, len, fmt, arg);
    va_end(arg);

    b->len -= (len - s.len);

    b->buf[b->len] = '\0';
    b->len++;
    return s;
}

/* ---------------------------------------------------------------------------
 * String operations
 * ------------------------------------------------------------------------- */

SI Str str_trim(Str src, StrTrimFlags flags)
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

SI KeyVal str_split_keyval(Str src, char sep, StrTrimFlags flags)
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

SI Strs str_splitc(Buf* b, Str src, char sep, isize maxlen, StrSplitFlags flags)
{
    if (maxlen == -1) maxlen = buf_avail(b, sizeof(Str));
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

SI Strs str_split_lines(Buf* b, Str src, isize maxlen, bool ignore_empty)
{
    if (maxlen == -1) maxlen = buf_avail(b, sizeof(Str));
    StrSplitFlags flags = ignore_empty ? STRS_SPLIT_IGNORE_EMPTY : STRS_SPLIT_DEFAULT;
    return str_splitc(b, src, '\n', maxlen, flags);
}

// --------------- String Hashmap ---------------
#define strmap_grave (Str){.buf = 0, .len = -1}

// Mask, step, index -> can use for your own hashmap
SI int strmap_next(u64 hash, int exp, int i)
{
    unsigned mask = (1 << exp) - 1;
    unsigned step = hash >> (64 - exp) | 1;
    return (i + step) & mask;
}

SI StrMap strmap_new(Buf* b, isize exp)
{
    return (StrMap){
        // ALLOC_ZERO needed to set .buf = 0 which marks empty slot
        .buf = make(b, KeyVal, (1 << exp), ALLOC_ZERO),
        .len = 0,
        .exp = exp,
    };
}

Str* strmap_lookup(StrMap* m, Str key)
{
    if (!m->len) { return NULL; } // empty set

    u64 hash  = str_hash64(key);
    i32 count = 0;
    for (i32 i = hash;;)
    {
        i = strmap_next(hash, m->exp, i);
        if (!m->buf[i].key.buf && !m->buf[i].key.len) { return NULL; }          // found empty slot
        else if (!m->buf[i].key.buf && (m->buf[i].key.len == -1)) { continue; } // found gravestone
        else if (str_equal(key, m->buf[i].key)) { return &m->buf[i].val; }      // found filled slot
        if ((count++) >= (m->len)) { return NULL; }                             // no slot found after full iteration
    }
}

int strmap_insert(StrMap* m, Str key, Str val)
{
    if ((m->len + 1) > (1 << m->exp)) { return -2; } // overflows capacity
    u64 hash = str_hash64(key);
    for (i32 i = hash;;)
    {
        i = strmap_next(hash, m->exp, i);
        if (!m->buf[i].key.buf) // found empty slot or gravestone, insert
        {
            m->buf[i].key = key;
            m->buf[i].val = val;
            m->len++;
            return i;
        }
        else if (str_equal(key, m->buf[i].key)) // found filled slot, overwrite
        {
            m->buf[i].val = val;
            return i;
        }
    }
}

SI int strmap_delete(StrMap* m, Str key)
{
    u64 hash  = str_hash64(key);
    i32 count = 0;
    for (i32 i = hash;;)
    {
        i = strmap_next(hash, m->exp, i);
        if (!m->buf[i].key.buf && !m->buf[i].key.len) { return -1; }            // found empty slot, nothing to delete
        else if (!m->buf[i].key.buf && (m->buf[i].key.len == -1)) { continue; } // found gravestone
        else if (str_equal(key, m->buf[i].key))                                 // found key
        {
            m->buf[i].key = (Str){.buf = 0, .len = -1};
            m->buf[i].val = StrNull;
            m->len--;
            return 0; // successful deletion
        }
        if ((count++) >= (m->len)) { return -1; } // no slot found after full iteration
    }
}

// --------------- String Set ---------------
#define strset_grave (Str){.buf = 0, .len = -1}

// Mask, step, index -> can use for your own hashmap
SI int strset_next(u64 hash, int exp, int i)
{
    unsigned mask = (1 << exp) - 1;
    unsigned step = hash >> (64 - exp) | 1;
    return (i + step) & mask;
}

SI StrSet strset_new(Buf* b, isize exp)
{
    return (StrSet){
        // ALLOC_ZERO needed to set .buf = 0 which marks empty slot
        .buf = make(b, Str, (1 << exp), ALLOC_ZERO),
        .len = 0,
        .exp = exp,
    };
}

SI int strset_lookup(StrSet* m, Str key)
{
    if (!m->len) { return -1; } // empty set

    u64 hash  = str_hash64(key);
    i32 count = 0;
    for (i32 i = hash;;)
    {
        i = strset_next(hash, m->exp, i);
        if (!m->buf[i].buf && !m->buf[i].len) { return -1; }            // found empty slot
        else if (!m->buf[i].buf && (m->buf[i].len == -1)) { continue; } // found gravestone
        else if (str_equal(key, m->buf[i])) { return i; }               // found filled slot
        if ((count++) >= m->len) { return -1; } // BUG: necessary? no slot found after full iteration
    }
}

SI int strset_insert(StrSet* m, Str key)
{
    if ((m->len + 1) > (1 << m->exp)) { return -1; } // overflows capacity

    u64 hash = str_hash64(key);
    for (i32 i = hash;;)
    {
        i = strset_next(hash, m->exp, i);
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

SI int strset_delete(StrSet* m, Str key)
{
    int idx = strset_lookup(m, key);
    if (idx < 0) { return -1; }               // not found
    m->buf[idx] = (Str){.buf = 0, .len = -1}; // insert gravestone
    m->len--;
    return idx;
}

/* ---------------------------------------------------------------------------
 *  Printing Generics
 * ------------------------------------------------------------------------- */

#define X_TABLE_PRIMITIVES(x, type, fmt, args)                                                                         \
    X(x, Any, "%p", *x)                                                                                                \
    X(x, Chars, "%s", *x)                                                                                              \
    X(x, CChars, "%s", *x)                                                                                             \
    X(x, bool, "%s", *x ? "true" : "false")                                                                            \
    X(x, u16, "%u", *x)                                                                                                \
    X(x, u32, "%u", *x)                                                                                                \
    X(x, u64, "%lu", *x)                                                                                               \
    X(x, i16, "%d", *x)                                                                                                \
    X(x, i32, "%d", *x)                                                                                                \
    X(x, i64, "%ld", *x)                                                                                               \
    X(x, f32, "%f", *x)                                                                                                \
    X(x, f64, "%f", *x)                                                                                                \
    X(x, Str, "%.*s", _s((*x)))                                                                                        \
    X(x, Str0, "%.*s", _s((*x)))

#define X_MACRO_PRINT(x, type, fmt, args)                                                                              \
    SI void printvar__##type(type* x) { p_inline(fmt, args); };

#define X X_MACRO_PRINT
X_TABLE_PRIMITIVES(x, type, fmt, args)
#undef X

#define DECLARE_PRINTVAR_ARRAY(type, fmt)                                                                              \
    SI void printvar__##type(type* x)                                                                                  \
    {                                                                                                                  \
        if (!x->len)                                                                                                   \
        {                                                                                                              \
            p_inline("[]");                                                                                            \
            return;                                                                                                    \
        }                                                                                                              \
        p_inline("[");                                                                                                 \
        RANGE(i, x->len - 1) { p_inline(fmt ", ", x->buf[i]); }                                                        \
        p_inline(fmt "]", x->buf[x->len - 1]);                                                                         \
    }

DECLARE_PRINTVAR_ARRAY(u32s, "%u");
DECLARE_PRINTVAR_ARRAY(u64s, "%lu");
DECLARE_PRINTVAR_ARRAY(i32s, "%d");
DECLARE_PRINTVAR_ARRAY(i64s, "%ld");
DECLARE_PRINTVAR_ARRAY(f32s, "%f");
DECLARE_PRINTVAR_ARRAY(f64s, "%f");

SI void printvar__Strs(Strs* x)
{
    if (!x->len)
    {
        p_inline("[]");
        return;
    }
    p_inline("[");
    RANGE(i, x->len - 1) { p_inline("%.*s, ", _s(x->buf[i])); }
    p_inline("%.*s]", _s(x->buf[x->len - 1]));
}

SI void printvar__Str0s(Str0s* x) { printvar__Strs((Strs*)x); }

SI void printvar__StrMap(StrMap* x)
{
    if (!x->len)
    {
        p_inline("{}");
        return;
    }
    p_inline("{");
    isize count = 0;
    RANGE(i, (1 << x->exp))
    {
        if (!(x->buf[i].key.buf)) continue;

        p_inline("%.*s = %.*s", _s(x->buf[i].key), _s(x->buf[i].val));
        count++;
        if (count < x->len) { p_inline(", "); }
    }
    p_inline("}");
}

#define PrintVar(x)                                                                                                    \
    _Generic((x),                                                                                                      \
        bool: printvar__bool,                                                                                          \
        u16: printvar__u16,                                                                                            \
        u32: printvar__u32,                                                                                            \
        u64: printvar__u64,                                                                                            \
        i16: printvar__i16,                                                                                            \
        i32: printvar__i32,                                                                                            \
        i64: printvar__i64,                                                                                            \
        f32: printvar__f32,                                                                                            \
        f64: printvar__f64,                                                                                            \
        Any: printvar__Any,                                                                                            \
        Chars: printvar__Chars,                                                                                        \
        CChars: printvar__CChars,                                                                                      \
        Str: printvar__Str,                                                                                            \
        Str0: printvar__Str0,                                                                                          \
        u32s: printvar__u32s,                                                                                          \
        u64s: printvar__u64s,                                                                                          \
        i32s: printvar__i32s,                                                                                          \
        i64s: printvar__i64s,                                                                                          \
        f32s: printvar__f32s,                                                                                          \
        f64s: printvar__f64s,                                                                                          \
        Strs: printvar__Strs,                                                                                          \
        Str0s: printvar__Str0s,                                                                                        \
        StrMap: printvar__StrMap)(&x)

#define PrintLn(x)         (p_inline("%s = ", #x), PrintVar(x), p_newline());
#define PrintAligned(x, n) (p_inline("%-" #n "s = ", #x), PrintVar(x), p_newline());
