#include "stz2/types.h"

#include "stz2/test.h"

int main(int argc, char* argv[])
{
    TEST_SUITE("buf");

    TEST_CASE("Size") { EXPECT_EQ_LONG(sizeof(Buf), 24L); }

    TEST_CASE("Alloc")
    {
        isize cap = 4;
        isize len = 2;

        Buf b = buf_new(cap);
        EXPECT_NEQ_NULL(b.buf);
        EXPECT_EQ_LONG(b.len, 0L);
        EXPECT_EQ_LONG(b.cap, cap);

        buf_reset(&b);
        memset(b.buf, 'a', b.cap);

        char* c1 = make(&b, char, len, ALLOC_ZERO);
        RANGE(i, len) { EXPECT_EQ_INT(c1[i], 0); }
        RANGE(i, len, cap) { EXPECT_EQ_INT(c1[i], 'a'); }

        buf_reset(&b);
        memset(b.buf, 'a', b.cap);

        char* c2 = make(&b, char, len, ALLOC_NOZERO);
        RANGE(i, cap) { EXPECT_EQ_INT(c2[i], 'a'); }
    }

    return TEST_RESULTS();
}
