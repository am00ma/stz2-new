#pragma once

#include "stz2/types.h"

// --------------- Printing tables ---------------
DECLARE_ARRAY(StrsArr, Strs);

SI void table_print_max_width_Str(Buf* b, Str x, isize max_width)
{
    if (x.len <= max_width)
    {
        buf_stack(temp, 32);
        Str0 fmt = str0_fmt(&temp, "%%-%ld.*s", max_width);
        str_fmt(b, fmt.buf, _s(x));
        return;
    }

    buf_stack(temp, 32);
    Str0 fmt = str0_fmt(&temp, "%%-%ld.*s ..", max_width - 3);
    str_fmt(b, fmt.buf, (int)(max_width - 3), x.buf);
}

SI Str table_print(Buf* b, Strs cols, StrsArr rows, isize max_col_width, Str sep)
{
    // Init measurements for headers
    i32s len_header = {
        .buf = make(b, i32, cols.len, ALLOC_ZERO),
        .len = cols.len,
    };
    RANGE(i, cols.len) { len_header.buf[i] = cols.buf[i].len; }

    // Init measurements for rows
    i32s len_rows = {
        .buf = make(b, i32, cols.len, ALLOC_ZERO),
        .len = cols.len,
    };

    // Set minimum as header len
    RANGE(j, cols.len) { len_rows.buf[j] = len_header.buf[j]; }

    // Get max over rows
    RANGE(i, rows.len)
    {
        RANGE(j, cols.len) { len_rows.buf[j] = maximum(len_rows.buf[j], rows.buf[i].buf[j].len); }
    }

    // Clip by column width
    RANGE(j, cols.len) { len_rows.buf[j] = minimum(len_rows.buf[j], max_col_width); }

    // Print the headers
    Buf out = buf_new2(b, buf_avail(b, sizeof(char)), ALLOC_NOZERO);
    RANGE(i, cols.len)
    {
        table_print_max_width_Str(&out, cols.buf[i], len_rows.buf[i]);
        if (i < cols.len - 1) str_fmt(&out, "%.*s", _s(sep));
    }
    str_fmt(&out, "\n");

    // Print the rows
    RANGE(j, rows.len)
    {
        RANGE(i, cols.len)
        {
            table_print_max_width_Str(&out, rows.buf[j].buf[i], len_rows.buf[i]);
            if (i < cols.len - 1) str_fmt(&out, "%.*s", _s(sep));
        }
        str_fmt(&out, "\n");
    }

    // Reclaim
    b->len -= (out.cap - out.len);

    return Str_(out);
}

// --------------- Currently unused ---------------

#define DECLARE_PRINT_MAX_WIDTH_NUM(type, format)                                                                      \
    SI void table_print_max_width_##type(Buf* b, type x, isize max_width)                                              \
    {                                                                                                                  \
        buf_stack(temp, 32);                                                                                           \
        Str0 fmt = str0_fmt(&temp, "%%-%ld" format, max_width);                                                        \
        str_fmt(b, fmt.buf, x);                                                                                        \
    }

DECLARE_PRINT_MAX_WIDTH_NUM(u32, "u");
DECLARE_PRINT_MAX_WIDTH_NUM(u64, "lu");
DECLARE_PRINT_MAX_WIDTH_NUM(i32, "d");
DECLARE_PRINT_MAX_WIDTH_NUM(i64, "ld");
DECLARE_PRINT_MAX_WIDTH_NUM(f32, "f");
DECLARE_PRINT_MAX_WIDTH_NUM(f64, "f");
