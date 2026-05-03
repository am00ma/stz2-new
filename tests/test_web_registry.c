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
        EXPECT_EQ_LONG(w.paths.len, 0L);

        WebFile* data;

        // Lookup and load if not loaded
        err = web_registry_path_lookup(&w, _0("a"), &data);
        EXPECT_EQ_INT(err, 0);
        EXPECT_EQ_LONG(w.paths.len, 1L);

        EXPECT_NEQ_NULL(data);
        EXPECT_EQ_STR(Str_(data->path), _("a"));

        // Delete oldest
        err = web_registry_path_delete(&w);
        EXPECT_EQ_INT(err, 0);
        EXPECT_EQ_LONG(w.paths.len, 0L);

        // Just puts it back
        err = web_registry_path_lookup(&w, _0("a"), &data);
        EXPECT_EQ_INT(err, 0);
        EXPECT_EQ_LONG(w.paths.len, 1L);

        EXPECT_NEQ_NULL(data);
        EXPECT_EQ_STR(Str_(data->path), _("a"));

        // Request 'cached'
        err = web_registry_path_lookup(&w, _0("a"), &data);
        EXPECT_EQ_INT(err, 0);
        EXPECT_EQ_LONG(w.paths.len, 1L);

        err = web_registry_path_lookup(&w, _0("b"), &data);
        EXPECT_EQ_INT(err, 0);
        EXPECT_EQ_LONG(w.paths.len, 2L);

        // Delete from list of two elements
        err = web_registry_path_delete(&w);
        EXPECT_EQ_INT(err, 0);
        EXPECT_EQ_LONG(w.paths.len, 1L);

        // // BUG: Delete from list of one element
        // err = web_registry_path_delete(&w);
        // EXPECT_EQ_INT(err, 0);
        // EXPECT_EQ_LONG(w.paths.len, 0L);
        //
        // // Delete from empty list
        // err = web_registry_path_delete(&w);
        // EXPECT_EQ_INT(err, -1);
        // EXPECT_EQ_LONG(w.paths.len, 0L);

        web_registry_free(&w);
    }

    return TEST_RESULTS();
}
