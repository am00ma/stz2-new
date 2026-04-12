#include "stz2/md/toc.h"

#include "stz2/test.h"

int main(int argc, char* argv[])
{
    TEST_SUITE("MdTOC");

    TEST_CASE("Size") { EXPECT_EQ_LONG(sizeof(MdTOC), 64L); }

    return TEST_RESULTS();
}
