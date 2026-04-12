#include "stz2/types.h"

#include "stz2/test.h"

int main(int argc, char* argv[])
{
    TEST_SUITE("array");

    TEST_CASE("Alloc")
    {
        buf_stack(temp, 1024);

        isize num    = 10;
        isize newlen = 5;

        f32s af32 = arr_new(f32s, &temp, f32, num, ALLOC_ZERO);
        EXPECT_EQ_LONG(temp.len, (isize)sizeof(f32) * num);
        EXPECT_TRUE(buf_ontop(&temp, af32.buf, af32.len * sizeof(*af32.buf)));

        RANGE(i, af32.len) { EXPECT_EQ_FLOAT(af32.buf[i], 0.0); }

        arr_shrink(af32, (&temp), newlen);
        EXPECT_EQ_LONG(temp.len, (isize)sizeof(f32) * newlen);
        EXPECT_TRUE(buf_ontop(&temp, af32.buf, af32.len * sizeof(*af32.buf)));

        buf_reset(&temp);
        i64s ai64 = arr_new(i64s, &temp, i64, num, ALLOC_ZERO);
        EXPECT_EQ_LONG(temp.len, (isize)sizeof(i64) * num);
        EXPECT_TRUE(buf_ontop(&temp, ai64.buf, ai64.len * sizeof(*ai64.buf)));

        RANGE(i, ai64.len) { EXPECT_EQ_LONG(ai64.buf[i], 0L); }

        arr_shrink(ai64, (&temp), newlen);
        EXPECT_EQ_LONG(temp.len, (isize)sizeof(i64) * newlen);
        EXPECT_TRUE(buf_ontop(&temp, ai64.buf, ai64.len * sizeof(*ai64.buf)));
    }

    return TEST_RESULTS();
}
