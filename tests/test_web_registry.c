#include "stz2/web/registry.h"

#include "stz2/test.h"

int main(int argc, char* argv[])
{
    TEST_SUITE("WebRegistry");

    TEST_CASE("cache, lookup, delete")
    {
        int   err = 0;
        isize exp = 8;

        WebRegistry w = web_registry_new(MB_, exp);

        WebFile* data;

        err = web_registry_path_lookup(&w, _0("a"), &data);
        EXPECT_EQ_INT(err, 0);
        EXPECT_NEQ_NULL(data);
        EXPECT_EQ_STR(Str_(data->path), _("a"));

        err = web_registry_path_delete(&w);
        EXPECT_EQ_INT(err, 0);

        // Just puts it back
        err = web_registry_path_lookup(&w, _0("a"), &data);
        EXPECT_EQ_INT(err, 0);
        EXPECT_NEQ_NULL(data);
        EXPECT_EQ_STR(Str_(data->path), _("a"));

        err = web_registry_path_delete(&w);
        EXPECT_EQ_INT(err, 0);

        err = web_registry_path_delete(&w);
        EXPECT_EQ_INT(err, -1);

        web_registry_free(&w);
    }

    return TEST_RESULTS();
}
