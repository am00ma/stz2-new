#include "stz2/types.h"

#include "stz2/test.h"

int main(int argc, char* argv[])
{
    TEST_SUITE("buf");

    TEST_CASE("Size") { EXPECT_EQ_LONG(sizeof(Buf), 24L); }

    return TEST_RESULTS();
}
