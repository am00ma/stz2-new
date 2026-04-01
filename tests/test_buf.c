#include "stz2/types.h"

#include "stz2/test.h"

int main(int argc, char* argv[])
{
    TEST_SUITE("Buf");

    TEST_CASE("Size") { EXPECT_EQ_LONG(sizeof(Buf), 24L); }

    TEST_CASE("Alloc: ZERO, NOZERO: char")
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

        buf_free(&b);
    }

    TEST_CASE("Alloc: Align")
    {

        Buf b = buf_new(128);

        // clang-format off
        #define EXPECT_ALLOC(type, used) make(&b, typeof(type)); EXPECT_EQ_LONG(b.len, used)
        // clang-format on

        EXPECT_ALLOC(isize, 8L);                   // Some initial alloc
        EXPECT_ALLOC(char[3], 11L);                // Align is 1 anyway
        EXPECT_ALLOC(i16, 11L + 1L + sizeof(i16)); // To demonstrate padding

        // C already takes care of padding in case of struct
        EXPECT_EQ_LONG(sizeof(struct {
                           isize len;
                           char  c[3];
                       }),
                       16L);

        buf_reset(&b);
        EXPECT_ALLOC(
            struct {
                isize len;
                char  c[3];
            },
            16L); // As expected, == sizeof(struct)

        buf_reset(&b);
        EXPECT_ALLOC(char[3], 3L);
        EXPECT_ALLOC(
            struct {
                isize len;
                char  c[3];
            },
            24L); // Works with align as well

        buf_free(&b);
    }

    TEST_CASE("On top")
    {
        buf_stack(b, 128);

        EXPECT_FALSE(buf_ontop(&b, NULL, 0));

        i32* x1 = make(&b, i32);
        EXPECT_TRUE(buf_ontop(&b, (char*)x1, sizeof(*x1)));

        make(&b, char, 3); // To throw off alignment

        i32* x2 = make(&b, i32, 10);
        EXPECT_TRUE(buf_ontop(&b, (char*)x2, 10 * sizeof(*x2)));
    }

    return TEST_RESULTS();
}
