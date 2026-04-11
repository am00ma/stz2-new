#include "stz2/types.h"

#include "stz2/test.h"

int main(int argc, char* argv[])
{
    TEST_SUITE("Range, foreach");

    TEST_CASE("RANGE")
    {
        i64 arr[10];
        RANGE(i, 10)
        {
            arr[i] = i;
            EXPECT_EQ_LONG(i, arr[i]);
        }
    }

    TEST_CASE("FOREACH")
    {
        buf_stack(b, 128);

        Str arr = str_copy(&b, _("hello"), false);
        FOREACH(i, x, el, arr) { EXPECT_EQ_INT(arr.buf[el.i], *el.x); }
        FOREACH(i, el, arr) { EXPECT_EQ_INT(arr.buf[el.i], *el.x); }
        FOREACH(i, arr) { EXPECT_EQ_INT(arr.buf[el.i], *el.x); }
        FOREACH(arr) { EXPECT_EQ_INT(arr.buf[el.i], *el.x); }

        FOREACH(arr) { *el.x = el.i; }
        FOREACH(arr) { EXPECT_EQ_INT((char)el.i, *el.x); }
    }

    TEST_RESULTS();
}
