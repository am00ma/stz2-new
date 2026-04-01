#include "stz2/types.h"

#include "stz2/test.h"

// Custom struct
typedef struct
{
    i32 idx;
    u64 len;
    Str a;
} Struct_A;

// Table and count for reflection
#define X_COUNT_Struct_A 3
#define X_TABLE_Struct_A(idx, type, ref, field, key)                                                                   \
    X(0, i32, &, idx, "idx")                                                                                           \
    X(1, u64, &, len, "len")                                                                                           \
    X(2, Str, &, a, "a")

// Nested with reference
typedef struct
{
    Struct_A* buf;
    Struct_A  data;
    isize     len;
} Struct_B;

#define X_COUNT_Struct_B 3
#define X_TABLE_Struct_B(idx, type, ref, field, key)                                                                   \
    X(0, Struct_A, , buf, "buf")                                                                                       \
    X(1, Struct_A, &, data, "data")                                                                                    \
    X(2, i64, &, len, "len")

// Serialize even nested stuff, with and without reference
#define X(...) X_PRINT_FIELD(Struct_A, __VA_ARGS__)

X_DECLARE_PRINT(Struct_A, idx, type, ref, field, key);
X_DECLARE_PRINT(Struct_B, idx, type, ref, field, key);

#undef X

int main(int argc, char* argv[])
{
    TEST_SUITE("Print");

    TEST_CASE("Print")
    {
        int a = 12;
        PrintLn(a);

        Struct_A x = {-10, 20, _("hello")};
        printvar__Struct_A(&x);
        p_newline();

        Struct_B y = {.buf = &x, .data = x, .len = 1};
        printvar__Struct_B(&y);
        p_newline();
    }

    return TEST_RESULTS();
}
