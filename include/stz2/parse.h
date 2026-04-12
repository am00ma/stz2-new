#pragma once

#include "stz2/types.h"

#include <inttypes.h> // strtoimax

#define MAX_LEN_INTEGER 32
#define MAX_LEN_DOUBLE  32

// --------------- Character checks ---------------

#define is_sign(c)  ((c == '-') || (c == '+'))
#define is_exp(c)   ((c == 'e') || (c == 'E'))
#define is_digit(c) ((c >= '0') && (c <= '9'))
#define is_hex(c)   (((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F')) || is_digit(c))
#define is_ws(c)    ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'))
#define is_escape(c)                                                                                                   \
    ((c == '"') || (c == '\\') || (c == '/') || (c == 'b') || (c == 'f') || (c == 'n') || (c == 'r') || (c == 't'))

// clang-format off
// --------------- Parser ---------------

typedef struct
{
    char* buf;
    isize len;
    isize pos;
    int   err;

} Parser;

#define BufAt(p, len_) (p->buf[p->pos + (len_)])

#define BufFromStr(s)      (Parser){.buf = s.buf, .len = s.len}
#define BufFromBuffer(b)   (Parser){.buf = &b->buf[b->pos], .len = b->len - b->pos}
#define BufFromStack(b, n) char buf__##b[(n)] = {}; Parser b = {.buf = buf__##b, .len = (n)};

#define BufToStr(p, len_) (Str){.buf = &p->buf[p->pos - (len_)], .len = (len_)}

#define BufCheckIf(p, cond, val) if ((cond)) { p->err = -1; return val; }
#define BufCheckCap(p, cap, val) BufCheckIf(p, (p->pos + (cap) > p->len), val)

// NOTE: !! expr should modify bb !! e.g. `BufSafeConcat(b, (&bb), concat__string((&bb), s))`
#define BufSafeConcat( b, bb, expr) ({(expr); if (bb->err) {b->err = bb->err; return BufToStr(bb, bb->pos);}})
#define BufSafeConsume(b, bb, expr) ({(expr); if (bb->err) {b->err = bb->err; return (Str){};}})

// clang-format on

// --------------- concat ---------------

SI Str concat__char(Parser* b, char c)
{
    BufCheckCap(b, 1, StrNull); // Check capacity
    BufAt(b, 0) = c;            // Set buffer
    b->pos++;                   // Update position
    return BufToStr(b, 1);      // Return single char
}

SI Str concat__string(Parser* b, Str s)
{
    isize len = minimum(b->len - b->pos, s.len);   // Print as much as possible
    memcpy(&BufAt(b, 0), s.buf, len);              //
    b->pos += len;                                 //
    BufCheckIf(b, len != s.len, BufToStr(b, len)); // Set error and return incomplete str
    return BufToStr(b, s.len);                     // Return full string
}

SI Str concat__quoted_string(Parser* b, Str s)
{
    Parser bb = BufFromBuffer(b);
    BufSafeConcat(b, (&bb), concat__char((&bb), '"')); // Starting quote
    BufSafeConcat(b, (&bb), concat__string((&bb), s)); // Print as much as possible, maybe return
    BufSafeConcat(b, (&bb), concat__char((&bb), '"')); // Ending quote
    b->pos += bb.pos;                                  // Successful, so update
    return BufToStr((&bb), bb.pos);                    // Return full string
}

SI Str concat__bool(Parser* b, bool x)
{
    Parser bb = BufFromBuffer(b);
    Str    s  = x ? _("true") : _("false");            // Representation
    BufSafeConcat(b, (&bb), concat__string((&bb), s)); // Print as much as possible, maybe return
    b->pos += bb.pos;                                  // Successful, so update
    return BufToStr((&bb), bb.pos);                    // Return bool literal
}

SI Str concat__unsigned_int(Parser* b, u64 x)
{
    BufCheckCap(b, MAX_LEN_INTEGER, StrNull);                  // Harsh, as snprintf can overflow
    Parser bb  = BufFromBuffer(b);                             //
    bb.pos    += snprintf(&BufAt((&bb), 0), bb.len, "%lu", x); // Print as much as possible
    b->pos    += bb.pos;                                       // Successful, so update
    return BufToStr((&bb), bb.pos);                            // Return printed part, we cannot check if correct
}

SI Str concat__signed_int(Parser* b, i64 x)
{
    BufCheckCap(b, MAX_LEN_INTEGER, StrNull);                  // Harsh, as snprintf can overflow
    Parser bb  = BufFromBuffer(b);                             //
    bb.pos    += snprintf(&BufAt((&bb), 0), bb.len, "%ld", x); // Print as much as possible
    b->pos    += bb.pos;                                       // Successful, so update
    return BufToStr((&bb), bb.pos);                            // Return printed part, we cannot check if correct
}

SI Str concat__double(Parser* b, f64 x)
{
    BufCheckCap(b, MAX_LEN_DOUBLE, StrNull);                  // Harsh, as snprintf can overflow
    Parser bb  = BufFromBuffer(b);                            //
    bb.pos    += snprintf(&BufAt((&bb), 0), bb.len, "%f", x); // Print as much as possible
    b->pos    += bb.pos;                                      // Successful, so update
    return BufToStr((&bb), bb.pos);                           // Return printed part, we cannot check if correct
}

// --------------- Print ---------------

Str print__char(Parser* b, char* x) { return concat__char(b, *x); }
Str print__Str(Parser* b, Str* x) { return concat__quoted_string(b, *x); }

Str print__bool(Parser* b, bool* x) { return concat__bool(b, *x); }
Str print__u32(Parser* b, u32* x) { return concat__unsigned_int(b, *x); }
Str print__u64(Parser* b, u64* x) { return concat__unsigned_int(b, *x); }
Str print__i32(Parser* b, i32* x) { return concat__signed_int(b, *x); }
Str print__i64(Parser* b, i64* x) { return concat__signed_int(b, *x); }
Str print__f32(Parser* b, f32* x) { return concat__double(b, *x); }
Str print__f64(Parser* b, f64* x) { return concat__double(b, *x); }

#define print__array(type, b, arr)                                                                                     \
    {                                                                                                                  \
        concat__char(b, '[');                                                                                          \
        for (isize i = 0; i < (arr).len; i++)                                                                          \
        {                                                                                                              \
            print__##type(b, (arr).buf[i]);                                                                            \
            if (i != (arr).len - 1) { concat__string(b, _(", ")); }                                                    \
        }                                                                                                              \
        concat__char(b, ']');                                                                                          \
    }

// --------------- consume ---------------

SI Str consume__whitespace(Parser* b)
{
    Parser bb = BufFromBuffer(b);                                  // Setup local buffer
    while ((bb.pos <= bb.len) && is_ws(BufAt((&bb), 0))) bb.pos++; // Scan
    b->pos += bb.pos;                                              // Successful, so update original
    return BufToStr((&bb), bb.pos);                                // Return from local buffer
}

SI Str consume__anychar(Parser* b)
{
    BufCheckCap(b, 1, StrNull); // Error if not long enough
    b->pos++;                   // Always succeeds, so no need for local buffer
    return BufToStr(b, 1);      // Return from main buffer
}

SI Str consume__char(Parser* b, char c)
{
    BufCheckCap(b, 1, StrNull);                  // Error if not long enough
    BufCheckIf(b, !(BufAt(b, 0) == c), StrNull); // Check char
    b->pos++;                                    // Update buffer on success
    return BufToStr(b, 1);                       // Return from main buffer
}

SI Str consume__literal(Parser* b, Str s)
{
    BufCheckCap(b, s.len, StrNull);                                    // Error if not long enough
    BufCheckIf(b, !str_equal((Str){&BufAt(b, 0), s.len}, s), StrNull); // Check string
    b->pos += s.len;                                                   // Update buffer on success
    return BufToStr(b, s.len);                                         // Return from main buffer
}

SI Str consume__quoted_literal(Parser* b, Str s)
{
    BufCheckCap(b, s.len + 2, StrNull);                                // Error if not long enough
    BufCheckIf(b, (BufAt(b, 0) != '"'), StrNull);                      // Starting quote
    BufCheckIf(b, !str_equal((Str){&BufAt(b, 1), s.len}, s), StrNull); // Check string
    BufCheckIf(b, (BufAt(b, s.len + 1) != '"'), StrNull);              // Starting quote
    b->pos += s.len + 2;                                               // Update buffer on success
    return BufToStr(b, s.len + 2); // Return from main buffer (what was consumed, not what was parsed)
}

SI Str consume__json_string(Parser* b)
{
    BufCheckCap(b, 2, StrNull);                       // Error if not long enough
    Parser bb = BufFromBuffer(b);                     // Setup local buffer
    BufCheckIf(b, (BufAt((&bb), 0) != '"'), StrNull); // Starting quote
    bb.pos++;                                         //

    while (bb.pos <= bb.len)
    {
        switch (BufAt((&bb), 0))
        {
        case '"':
            bb.pos++;
            goto __done;
            break;
        case '\\': {
            bb.pos++;
            BufCheckCap((&bb), 1, StrNull);
            if (is_escape(BufAt((&bb), 0))) { bb.pos++; }
            else if (BufAt((&bb), 0) == 'u')
            {
                bb.pos++;
                BufCheckCap((&bb), 4, StrNull);
                if (is_hex(BufAt((&bb), 0))    //
                    && is_hex(BufAt((&bb), 1)) //
                    && is_hex(BufAt((&bb), 2)) //
                    && is_hex(BufAt((&bb), 3)))
                {
                    bb.pos += 4;
                }
                else BufCheckIf(b, true, StrNull);
            }
            else
            {
                BufCheckIf(b, true, StrNull);
            }
        }
        break;
        default: bb.pos++; break;
        }
    }

__done:
    BufCheckIf(b, (BufAt(b, bb.pos - 1) != '"'), StrNull); // Last char was quote
    b->pos += bb.pos;                                      // Successful, so update buffer
    return BufToStr((&bb), bb.pos); // Return from main buffer (what was consumed, not what was parsed)
}

SI Str consume__quoted_string(Parser* b)
{
    BufCheckCap(b, 2, StrNull);                                      // Error if not long enough
    Parser bb = BufFromBuffer(b);                                    // Setup local buffer
    BufCheckIf(b, (BufAt((&bb), 0) != '"'), StrNull);                // Starting quote
    bb.pos++;                                                        //
    while ((bb.pos <= bb.len) && (BufAt((&bb), 0) != '"')) bb.pos++; // HACK: ignoring escapes for now
    BufCheckCap((&bb), 1, StrNull);                                  // Unterminated string
    BufCheckIf(b, (BufAt((&bb), 0) != '"'), StrNull);                // Ending quote
    bb.pos++;                                                        //
    b->pos += bb.pos;                                                // Successful, so update buffer
    return BufToStr((&bb), bb.pos); // Return from main buffer (what was consumed, not what was parsed)
}

SI Str consume__digit(Parser* b)
{
    BufCheckCap(b, 1, StrNull);                     // Error if not long enough
    BufCheckIf(b, !is_digit(BufAt(b, 0)), StrNull); // One digit
    b->pos++;                                       // Update buffer on success
    return BufToStr(b, 1);                          // Return from main buffer
}

SI Str consume__digits(Parser* b)
{
    Parser bb = BufFromBuffer(b);                                     // Setup local buffer
    while ((bb.pos <= bb.len) && is_digit(BufAt((&bb), 0))) bb.pos++; // Within bounds increment on digit
    BufCheckIf(b, !(bb.pos), StrNull);                                // Empty string is error
    b->pos += bb.pos;                                                 // Update buffer on success
    return BufToStr((&bb), bb.pos);                                   // Return from local buffer
}

SI Str consume__integer(Parser* b)
{
    Parser bb = BufFromBuffer(b);                   // Setup local buffer
    if (is_sign(BufAt((&bb), 0))) bb.pos++;         // Optional sign
    BufSafeConsume(b, (&bb), consume__digits(&bb)); // Digits or return error
    b->pos += bb.pos;                               // Update buffer on success
    return BufToStr((&bb), bb.pos);                 // Return from local buffer
}

SI Str consume__fraction(Parser* b)
{
    if (BufAt(b, 0) != '.') return StrNull;            // Empty string is ok
    Parser bb = BufFromBuffer(b);                      // Setup local buffer
    BufCheckCap((&bb), 1, StrNull);                    // Error if not long enough
    BufCheckIf(b, !(BufAt((&bb), 0) == '.'), StrNull); // Mandatory dot
    bb.pos++;                                          // Update position
    BufSafeConsume(b, (&bb), consume__digits(&bb));    // Digits or return error
    b->pos += bb.pos;                                  // Update buffer on success
    return BufToStr((&bb), bb.pos);                    // Return from local buffer
}

SI Str consume__exponent(Parser* b)
{
    if (!is_exp(BufAt(b, 0))) return StrNull;           // Empty string is ok
    Parser bb = BufFromBuffer(b);                       // Setup local buffer
    BufCheckCap((&bb), 1, StrNull);                     // Error if not long enough
    BufCheckIf(b, !(is_exp(BufAt((&bb), 0))), StrNull); // Mandatory exponent
    bb.pos++;                                           // Update position
    BufSafeConsume(b, (&bb), consume__integer(&bb));    // Integer or return error
    b->pos += bb.pos;                                   // Update buffer on success
    return BufToStr((&bb), bb.pos);                     // Return from local buffer
}

SI Str consume__number(Parser* b)
{
    Parser bb = BufFromBuffer(b);                     // Setup local buffer
    BufSafeConsume(b, (&bb), consume__integer(&bb));  // Integer or return error
    BufSafeConsume(b, (&bb), consume__fraction(&bb)); // Fraction or return error
    BufSafeConsume(b, (&bb), consume__exponent(&bb)); // Exponent or return error
    b->pos += bb.pos;                                 // Update buffer on success
    return BufToStr((&bb), bb.pos);                   // Return from local buffer
}

// --------------- Parse ---------------

SI Str parse__char(Parser* b, char* x)
{
    Parser bb = BufFromBuffer(b);
    Str    ch = consume__anychar((&bb));
    if (bb.err) return StrNull;
    b->pos += bb.pos;
    if (x) *x = ch.buf[0];
    return BufToStr((&bb), bb.pos);
}

SI Str parse__Str(Parser* b, Str* x)
{
    Parser bb = BufFromBuffer(b);
    Str    s  = consume__json_string((&bb));
    if (bb.err) return StrNull;
    b->pos += bb.pos;
    if (x) *x = (Str){.buf = &s.buf[1], .len = s.len - 2}; // Parsed -> without qoutes
    return BufToStr((&bb), bb.pos);                        // Quoted, was what was consumed
}

SI Str parse__bool(Parser* b, bool* x)
{
    BufCheckCap(b, 4, StrNull);
    bool   val = true;
    Parser bb  = BufFromBuffer(b);
    consume__literal((&bb), _("true"));
    if (bb.err)
    {
        bb = BufFromBuffer(b);
        consume__literal((&bb), _("false"));
        if (!bb.err) val = false;
    }
    if (bb.err) return StrNull;
    b->pos += bb.pos;
    if (x) *x = val;
    return BufToStr((&bb), bb.pos);
}

SI Str parse__i32(Parser* b, i32* x)
{
    Parser bb = BufFromBuffer(b);
    Str    i  = consume__integer((&bb));
    if (bb.err) return StrNull;
    b->pos += bb.pos;
    if (x) *x = strtoimax(i.buf, 0, 10);
    return BufToStr((&bb), bb.pos);
}

SI Str parse__i64(Parser* b, i64* x)
{
    Parser bb = BufFromBuffer(b);
    Str    i  = consume__integer((&bb));
    if (bb.err) return StrNull;
    b->pos += bb.pos;
    if (x) *x = strtol(i.buf, 0, 10);
    return BufToStr((&bb), bb.pos);
}

SI Str parse__u32(Parser* b, u32* x)
{
    Parser bb = BufFromBuffer(b);
    Str    i  = consume__integer((&bb));
    if (bb.err) return StrNull;
    b->pos += bb.pos;
    if (x) *x = strtoumax(i.buf, 0, 10);
    return BufToStr((&bb), bb.pos);
}

SI Str parse__u64(Parser* b, u64* x)
{
    Parser bb = BufFromBuffer(b);
    Str    i  = consume__integer((&bb));
    if (bb.err) return StrNull;
    b->pos += bb.pos;
    if (x) *x = strtoul(i.buf, 0, 10);
    return BufToStr((&bb), bb.pos);
}

SI Str parse__f32(Parser* b, f32* x)
{
    Parser bb = BufFromBuffer(b);
    Str    i  = consume__number((&bb));
    if (bb.err) return StrNull;
    b->pos += bb.pos;
    if (x) *x = strtof(i.buf, 0);
    return BufToStr((&bb), bb.pos);
}

SI Str parse__f64(Parser* b, f64* x)
{
    Parser bb = BufFromBuffer(b);
    Str    i  = consume__number((&bb));
    if (bb.err) return StrNull;
    b->pos += bb.pos;
    if (x) *x = strtod(i.buf, 0);
    return BufToStr((&bb), bb.pos);
}

// --------------- TODO: Array ---------------

#define parse__array(type, b, arr)

/* ---------------------------------------------------------------------------
 * X-Macros
 * ------------------------------------------------------------------------- */

#define X_PARSE(name, idx, type, ref, field, key, print_fn, parse_fn)                                                  \
    len += consume__quoted_literal(b, _(key)).len;                                                                     \
    len += consume__char(b, ':').len;                                                                                  \
    len += parse_fn(b, ref(x->field)).len;                                                                             \
    len += (idx == X_COUNT_##name - 1) ? 0 : consume__literal(b, _(", ")).len;

#define X_DECLARE_PARSE(name, idx, type, ref, field, key, print_fn, parse_fn)                                          \
    SI Str parse__##name(Parser* b, name* x)                                                                           \
    {                                                                                                                  \
        isize len  = 0;                                                                                                \
        len       += consume__char(b, '{').len;                                                                        \
        X_TABLE_##name(idx, type, ref, field, key, print_fn, parse_fn);                                                \
        len += consume__char(b, '}').len;                                                                              \
        return BufToStr(b, len);                                                                                       \
    }

#define X_PRINT(name, idx, type, ref, field, key, print_fn, parse_fn)                                                  \
    len += concat__quoted_string(b, _(key)).len;                                                                       \
    len += concat__char(b, ':').len;                                                                                   \
    len += print_fn(b, ref(x->field)).len;                                                                             \
    len += (idx == X_COUNT_##name - 1) ? 0 : concat__string(b, _(", ")).len;

#define X_DECLARE_PRINT(name, idx, type, ref, field, key, print_fn, parse_fn)                                          \
    SI Str print__##name(Parser* b, name* x)                                                                           \
    {                                                                                                                  \
        isize len  = 0;                                                                                                \
        len       += concat__char(b, '{').len;                                                                         \
        X_TABLE_##name(idx, type, ref, field, key, print_fn, parse_fn);                                                \
        len += concat__char(b, '}').len;                                                                               \
        return BufToStr(b, len);                                                                                       \
    }
