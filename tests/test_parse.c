#include "stz2/parse.h" // IWYU pragma: keep

#include "stz2/test.h"

// --------------- Stuct A ---------------
typedef struct
{

    i32 idx;
    u64 len;
    f32 f;
    Str a;

} Struct_A;

// Table and count for reflection
#define X_TABLE_Struct_A(idx, type, ref, field, key, print_fn, parse_fn)                                               \
    X(0, i32, &, idx, "idx", , parse__i32)                                                                             \
    X(1, u64, &, len, "len", , parse__u64)                                                                             \
    X(2, f32, &, f, "f", , parse__f32)                                                                                 \
    X(3, Str, &, a, "a", , parse__Str)
#define X_COUNT_Struct_A 4

// Deserialize -> parse__Struct_A
#define X(...) X_PARSE(Struct_A, __VA_ARGS__)
X_DECLARE_PARSE(Struct_A, idx, type, ref, field, key, print_fn, parse_fn);
#undef X

// --------------- Stuct B ---------------

typedef struct
{
    i32      idx;
    u64      len;
    Struct_A a;

} Struct_B;

// Table and count for reflection
#define X_TABLE_Struct_B(idx, type, ref, field, key, print_fn, parse_fn)                                               \
    X(0, i32, &, idx, "idx", , parse__i32)                                                                             \
    X(1, u64, &, len, "len", , parse__u64)                                                                             \
    X(2, Struct_A, &, a, "a", , parse__Struct_A)
#define X_COUNT_Struct_B 3

// Deserialize -> parse__Struct_B
#define X(...) X_PARSE(Struct_B, __VA_ARGS__)
X_DECLARE_PARSE(Struct_B, idx, type, ref, field, key, print_fn, parse_fn);
#undef X

// --------------- Stuct C ---------------

typedef struct
{
    i32       idx;
    u64       len;
    Struct_A* a;

} Struct_C;

// Table and count for reflection
#define X_TABLE_Struct_C(idx, type, ref, field, key, print_fn, parse_fn)                                               \
    X(0, i32, &, idx, "idx", , parse__i32)                                                                             \
    X(1, u64, &, len, "len", , parse__u64)                                                                             \
    X(2, Struct_A, , a, "a", , parse__Struct_A)
#define X_COUNT_Struct_C 3

// Deserialize -> parse__Struct_B
#define X(...) X_PARSE(Struct_C, __VA_ARGS__)
X_DECLARE_PARSE(Struct_C, idx, type, ref, field, key, print_fn, parse_fn);
#undef X

// --------------- Main ---------------

int main(int argc, char* argv[])
{
    TEST_SUITE("parse");

    TEST_CASE("Struct_A")
    {
        Str src = _("{\"idx\":10, \"len\":345, \"f\":3.140000, \"a\":\"hello\"}");

        Parser b = BufFromStr(src);

        Struct_A expected = {
            .idx = 10,
            .len = 345,
            .f   = 3.14,
            .a   = _("hello"),
        };
        Struct_A x = {};

        Str dst = parse__Struct_A(&b, &x);

        EXPECT_EQ_STR(src, dst);
        EXPECT_EQ_INT(x.idx, expected.idx);
        EXPECT_EQ_LONG(x.len, expected.len);
        EXPECT_EQ_FLOAT(x.f, expected.f);
        EXPECT_EQ_STR(x.a, expected.a);
    }

    TEST_CASE("Struct_B")
    {
        Str src = _("{\"idx\":10, \"len\":345, \"a\":{\"idx\":-20, \"len\":567, \"f\":3.140000, \"a\":\"hello\"}}");

        Parser b = BufFromStr(src);

        Struct_B expected = {
            .idx = 10,
            .len = 345,
            .a   = {.idx = -20, .len = 567, .f = 3.14, .a = _("hello")},
        };
        Struct_B x = {};

        Str dst = parse__Struct_B(&b, &x);

        EXPECT_EQ_STR(src, dst);
        EXPECT_EQ_INT(x.idx, expected.idx);
        EXPECT_EQ_LONG(x.len, expected.len);
        EXPECT_EQ_INT(x.a.idx, expected.a.idx);
        EXPECT_EQ_LONG(x.a.len, expected.a.len);
        EXPECT_EQ_FLOAT(x.a.f, expected.a.f);
        EXPECT_EQ_STR(x.a.a, expected.a.a);
    }

    TEST_CASE("Struct_C")
    {
        Str src = _("{\"idx\":10, \"len\":345, \"a\":{\"idx\":-20, \"len\":567, \"f\":3.140000, \"a\":\"hello \\\" 😾 "
                    "\\n \"}}");

        Parser b = BufFromStr(src);

        Struct_C expected = {
            .idx = 10,
            .len = 345,
            .a   = &(Struct_A){.idx = -20, .len = 567, .f = 3.14, .a = _("hello \\\" 😾 \\n ")},
        };

        Struct_C x = {.a = &(Struct_A){}};

        Str dst = parse__Struct_C(&b, &x);

        EXPECT_EQ_STR(src, dst);
        EXPECT_EQ_INT(x.idx, expected.idx);
        EXPECT_EQ_LONG(x.len, expected.len);
        EXPECT_EQ_INT(x.a->idx, expected.a->idx);
        EXPECT_EQ_LONG(x.a->len, expected.a->len);
        EXPECT_EQ_FLOAT(x.a->f, expected.a->f);
        EXPECT_EQ_STR(x.a->a, expected.a->a);
    }

    TEST_RESULTS();
}
