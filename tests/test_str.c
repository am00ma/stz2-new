#include "stz2/types.h"

#include "stz2/test.h"

int main(int argc, char* argv[])
{
    TEST_SUITE("Str");

    TEST_CASE("Size") { EXPECT_EQ_LONG(sizeof(Str), 16L); }

    TEST_CASE("Basics")
    {
        Str str1 = _("hello");
        EXPECT_EQ_LONG(str1.len, 5L);

        Str0 str2 = _0("hello");
        EXPECT_EQ_LONG(str2.len, 5L);
    }

    TEST_CASE("Copy")
    {
        buf_stack(temp, 128);
        memset(temp.buf, 'a', 128);

        buf_reset(&temp);
        Str str1 = str_copy(&temp, _("hello"), false);
        EXPECT_EQ_LONG(str1.len, 5L);
        EXPECT_EQ_LONG(temp.len, 5L);
        EXPECT_FALSE(IsNullTerm(str1));

        buf_reset(&temp);
        Str str2 = str_copy(&temp, _("hello"), true);
        EXPECT_EQ_LONG(str2.len, 5L);
        EXPECT_EQ_LONG(temp.len, 6L);
        EXPECT_TRUE(IsNullTerm(str2));
    }

    TEST_CASE("Format")
    {
        buf_stack(temp, 128);
        memset(temp.buf, 'a', 128);

        str_fmtn(&temp, 10, "hellohello");
        EXPECT_EQ_LONG(temp.len, 10L);

        buf_reset(&temp);
        Str str3 = str_fmtn(&temp, 5, "hello");
        EXPECT_FALSE(IsNullTerm(str3));
        EXPECT_EQ_LONG(temp.len, 5L);

        // NOTE: Unexpected behaviour of snprintf
        char buf[128];
        memset(buf, 'a', 128);
        EXPECT_EQ_INT(snprintf(buf, 128, "hello"), 5);
        EXPECT_EQ_INT(buf[5], 0); // And not 'a'

        // Leads to the following
        buf_reset(&temp);
        Str str4 = str_fmt(&temp, "hello");
        EXPECT_TRUE(IsNullTerm(str4));
        EXPECT_EQ_LONG(temp.len, 5L); // But buf is proper

        buf_reset(&temp);
        Str str5 = str_fmt(&temp, "hello: %03ld", 1L);
        EXPECT_EQ_STR(str5, _("hello: 001"));
    }

    TEST_CASE("Sub")
    {
        Str s1 = _("hello hi how are you");
        EXPECT_EQ_STR(str_sub(s1, 0, 5), _("hello"));
        EXPECT_EQ_STR(str_sub(s1, 0, -1), s1);
        EXPECT_TRUE(str_startswith(s1, _("hello")));
        EXPECT_TRUE(str_endswith(s1, _("you")));
    }

    TEST_CASE("Trim")
    {
        EXPECT_EQ_STR(str_trim(_("\t x  \t"), STRS_TRIM_DEFAULT), _("x"));
        EXPECT_EQ_STR(str_trim(_("\n x  \n"), STRS_TRIM_LEFTRIGHT | STRS_TRIM_NEWLINES), _(" x  "));
        EXPECT_EQ_STR(str_trim(_("\n x  \n"), STRS_TRIM_RIGHT | STRS_TRIM_NEWLINES), _("\n x  "));
        EXPECT_EQ_STR(str_trim(_("\n x  \n"), STRS_TRIM_LEFT | STRS_TRIM_NEWLINES), _(" x  \n"));
        EXPECT_EQ_STR(str_trim(_("\n x  \n"), STRS_TRIM_RIGHT | STRS_TRIM_SPACES | STRS_TRIM_NEWLINES), _("\n x"));
        EXPECT_EQ_STR(str_trim(_("\n x  \n"), STRS_TRIM_LEFT | STRS_TRIM_SPACES | STRS_TRIM_NEWLINES), _("x  \n"));
        EXPECT_EQ_STR(str_trim(_("\n x  \n"), STRS_TRIM_DEFAULT | STRS_TRIM_NEWLINES), _("x"));
    }

    TEST_CASE("Split")
    {
        buf_stack(b, 128);

        // TODO: STRS_SPLIT_SUBSTITUTE_NULL
        struct
        {
            Str           src;
            isize         maxlen;
            StrSplitFlags flags;
            isize         expected;
        } cases[] = {
            {_(""), 0, 0, 0},
            {_("a b c"), 16, STRS_SPLIT_DEFAULT, 3},
            {_("a b c"), 2, STRS_SPLIT_DEFAULT, 2},
            {_("a    b c"), 16, STRS_SPLIT_DEFAULT, 6},
            {_("a    b c"), 16, STRS_SPLIT_IGNORE_EMPTY, 3},
        };

        RANGE(i, countof(cases))
        {
            buf_reset(&b);
            Strs got = str_splitc(&b, cases[i].src, ' ', cases[i].maxlen, cases[i].flags);
            EXPECT_EQ_LONG(got.len, cases[i].expected);
            EXPECT_EQ_LONG(b.len, (cases[i].expected * (isize)sizeof(Str)));
        }
    }

    TEST_CASE("Split lines")
    {
        buf_stack(b, 128);

        // TODO: STRS_SPLIT_SUBSTITUTE_NULL
        struct
        {
            Str   src;
            bool  ignore;
            isize expected;
        } cases[] = {
            {_(""), true, 0},
            {_(""), false, 1},
            {_("a\nb\nc"), false, 3},
            {_("a\n\n\n\nb\nc"), false, 6},
            {_("a\n\n\n\nb\nc"), true, 3},
        };

        RANGE(i, countof(cases))
        {
            buf_reset(&b);
            Strs got = str_split_lines(&b, cases[i].src, -1, cases[i].ignore);
            EXPECT_EQ_LONG(got.len, cases[i].expected);
            EXPECT_EQ_LONG(b.len, (cases[i].expected * (isize)sizeof(Str)));
        }
    }

    TEST_CASE("KeyVal")
    {
        buf_stack(b, 128);

        // clang-format off
        struct
        {
            Str           src;
            char           sep;
            StrTrimFlags flags;
            Str           exp_key;
            Str           exp_val;
        } cases[] = {
            {_(""),           ':', STRS_TRIM_DEFAULT, _(""),    _("")},
            {_("abc"),        ':', STRS_TRIM_DEFAULT, _("abc"), _("")},
            {_("a:b c"),      ':', STRS_TRIM_DEFAULT, _("a"),   _("b c")},
            {_("a:   b c"),   ':', STRS_TRIM_DEFAULT, _("a"),   _("b c")},
            {_("a:   b c  "), ':', STRS_TRIM_DEFAULT, _("a"),   _("b c")},
            {_("a:   b c  "), ':', STRS_TRIM_NONE,    _("a"),   _("   b c  ")},
        };
        // clang-format on

        RANGE(i, countof(cases))
        {
            buf_reset(&b);
            KeyVal got = str_split_keyval(cases[i].src, cases[i].sep, cases[i].flags);
            EXPECT_EQ_STR(got.key, cases[i].exp_key);
            EXPECT_EQ_STR(got.val, cases[i].exp_val);
        }
    }

    TEST_CASE("StrMap")
    {
        EXPECT_EQ_LONG(sizeof(StrMap), 24L);

        buf_stack(b, 1024);
        StrMap m = strmap_new(&b, 4);

        RANGE(i, (1 << m.exp))
        {
            Str key = str_fmt(&b, "key-%ld", i);
            Str val = str_fmt(&b, "val-%ld", i);
            EXPECT_TRUE(strmap_insert(&m, key, val) >= 0);
        }
        EXPECT_FALSE(strmap_insert(&m, _("a"), _("b")) >= 0);

        RANGE(i, (1 << m.exp))
        {
            Str key    = str_fmt(&b, "key-%ld", i);
            Str val    = str_fmt(&b, "val-%ld", i);
            Str newval = str_fmt(&b, "newval-%ld", i);
            EXPECT_NEQ_NULL(strmap_lookup(&m, key));
            EXPECT_EQ_STR(val, *strmap_lookup(&m, key));
            *strmap_lookup(&m, key) = newval;
            EXPECT_EQ_STR(newval, *strmap_lookup(&m, key));
        }
        EXPECT_EQ_NULL(strmap_lookup(&m, _("a")));
    }

    TEST_CASE("StrSet")
    {
        EXPECT_EQ_LONG(sizeof(StrSet), 24L);

        buf_stack(b, 1024);
        StrSet m = strset_new(&b, 4);

        RANGE(i, (1 << m.exp))
        {
            Str key = str_fmt(&b, "key-%ld", i);
            EXPECT_TRUE(strset_insert(&m, key) >= 0);
        }
        EXPECT_FALSE(strset_insert(&m, _("a")) >= 0);

        RANGE(i, (1 << m.exp))
        {
            Str key = str_fmt(&b, "key-%ld", i);
            EXPECT_TRUE(strset_lookup(&m, key) >= 0);
        }
        EXPECT_FALSE(strset_lookup(&m, _("a")) >= 0);
    }

    return TEST_RESULTS();
}
