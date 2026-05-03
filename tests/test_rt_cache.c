#include "stz2/rt/cache.h"

#include "stz2/test.h"

int main(int argc, char* argv[])
{
    TEST_SUITE("Cache");

    TEST_CASE("size")
    {
        EXPECT_EQ_LONG(sizeof(Cache), 72L);
        EXPECT_EQ_LONG(sizeof(CacheVal), 80L);
    }

    TEST_CASE("insert, lookup, delete")
    {
        int err = 0;

        Buf b = buf_new(MB_);

        Cache c = {};

        isize capacity_set_exp = 4;
        isize capacity_mem     = 1024;

        err = cache_new(&b, &c, capacity_mem, capacity_set_exp);
        EXPECT_EQ_INT(err, 0);

        cache_free(&c);

        buf_free(&b);
    }

    return TEST_RESULTS();
}
