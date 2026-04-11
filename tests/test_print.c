#include "stz2/parse.h" // IWYU pragma: keep

#include "stz2/test.h"

// --------------- Struct_A ---------------

typedef struct
{

    i32 idx;
    u64 len;
    Str a;

} Struct_A;

// Table and count for reflection
#define X_TABLE_Struct_A(idx, type, ref, field, key, print_fn, parse_fn)                                               \
    X(0, i32, &, idx, "idx", print__i32, NULL)                                                                         \
    X(1, u64, &, len, "len", print__u64, NULL)                                                                         \
    X(2, Str, &, a, "a", print__Str, NULL)
#define X_COUNT_Struct_A 3

// Serialize -> print__Struct_A
#define X(...) X_PRINT(Struct_A, __VA_ARGS__)
X_DECLARE_PRINT(Struct_A, idx, type, ref, field, key, print_fn, parse_fn);
#undef X

// --------------- Struct_B ---------------

typedef struct
{

    i32      idx;
    u64      len;
    Struct_A a;

} Struct_B;

// Table and count for reflection
#define X_TABLE_Struct_B(idx, type, ref, field, key, print_fn, parse_fn)                                               \
    X(0, i32, &, idx, "idx", print__i32, NULL)                                                                         \
    X(1, u64, &, len, "len", print__u64, NULL)                                                                         \
    X(2, Struct_A, &, a, "a", print__Struct_A, NULL)
#define X_COUNT_Struct_B 3

// Serialize -> print__Struct_B
#define X(...) X_PRINT(Struct_B, __VA_ARGS__)
X_DECLARE_PRINT(Struct_B, idx, type, ref, field, key, print_fn, parse_fn);
#undef X

// --------------- Struct_C ---------------

typedef struct
{

    i32       idx;
    u64       len;
    Struct_A* a;

} Struct_C;

// Table and count for reflection
#define X_TABLE_Struct_C(idx, type, ref, field, key, print_fn, parse_fn)                                               \
    X(0, i32, &, idx, "idx", print__i32, NULL)                                                                         \
    X(1, u64, &, len, "len", print__u64, NULL)                                                                         \
    X(2, Struct_A, , a, "a", print__Struct_A, NULL)
#define X_COUNT_Struct_C 3

// Serialize -> print__Struct_B
#define X(...) X_PRINT(Struct_C, __VA_ARGS__)
X_DECLARE_PRINT(Struct_C, idx, type, ref, field, key, print_fn, parse_fn);
#undef X

// --------------- Main ---------------

int main(int argc, char* argv[])
{
    TEST_SUITE("print");

    TEST_CASE("Struct_A")
    {
        BufFromStack(b, 128);

        Struct_A x = {
            .idx = 10,
            .len = 345,
            .a   = _("hello"),
        };

        Str dst      = print__Struct_A(&b, &x);
        Str expected = _("{\"idx\":10, \"len\":345, \"a\":\"hello\"}");
        EXPECT_EQ_STR(dst, expected);
    }

    TEST_CASE("Struct_B")
    {
        BufFromStack(b, 128);

        Struct_B x = {
            .idx = 10,
            .len = 345,
            .a   = {-20, 567, _("hello")},
        };

        Str dst      = print__Struct_B(&b, &x);
        Str expected = _("{\"idx\":10, \"len\":345, \"a\":{\"idx\":-20, \"len\":567, \"a\":\"hello\"}}");
        EXPECT_EQ_STR(dst, expected);
    }

    TEST_CASE("Struct_C")
    {
        BufFromStack(b, 128);

        Struct_C x = {
            .idx = 10,
            .len = 345,
            .a   = &(Struct_A){.idx = -20, .len = 567, .a = _("hello")},
        };

        Str dst      = print__Struct_C(&b, &x);
        Str expected = _("{\"idx\":10, \"len\":345, \"a\":{\"idx\":-20, \"len\":567, \"a\":\"hello\"}}");
        EXPECT_EQ_STR(dst, expected);
    }

    TEST_RESULTS();
}
