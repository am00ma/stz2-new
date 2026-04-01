#include "stz2/path.h"

#include "stz2/test.h"

int main(int argc, char* argv[])
{
    TEST_SUITE("Path");

    TEST_CASE("Size") { EXPECT_EQ_LONG(sizeof(Str0), 16L); }

    TEST_CASE("Extension")
    {
        buf_stack(b, 1024);
        memset(b.buf, 0, 1024);

        EXPECT_EQ_STR(path_basename(&b, _0("")), _("."));
        EXPECT_EQ_STR(path_basename(&b, _0("/")), _("/"));
        EXPECT_EQ_STR(path_basename(&b, _0("./")), _("."));
        EXPECT_EQ_STR(path_basename(&b, _0("./.")), _("."));

        buf_reset(&b);
        EXPECT_EQ_STR(path_basename(&b, _0("README.md")), _("README.md"));
        EXPECT_EQ_STR(path_basename(&b, _0("/README.md")), _("README.md"));
        EXPECT_EQ_STR(path_basename(&b, _0("./README.md")), _("README.md"));
        EXPECT_EQ_STR(path_basename(&b, _0("/home/x/README.md")), _("README.md"));

        buf_reset(&b);
        EXPECT_EQ_STR(path_dirname(&b, _0("README.md")), _("."));
        EXPECT_EQ_STR(path_dirname(&b, _0("/README.md")), _("/"));
        EXPECT_EQ_STR(path_dirname(&b, _0("./README.md")), _("."));
        EXPECT_EQ_STR(path_dirname(&b, _0("/home/x/README.md")), _("/home/x"));

        buf_reset(&b);
        EXPECT_EQ_STR(path_extension(&b, _0("")), _(""));
        EXPECT_EQ_STR(path_extension(&b, _0(".")), _("."));
        EXPECT_EQ_STR(path_extension(&b, _0("/")), _(""));
        EXPECT_EQ_STR(path_extension(&b, _0("./")), _("."));
        EXPECT_EQ_STR(path_extension(&b, _0("./.")), _("."));

        buf_reset(&b);
        EXPECT_EQ_STR(path_extension(&b, _0("README.md")), _(".md"));
        EXPECT_EQ_STR(path_extension(&b, _0("/README.md")), _(".md"));
        EXPECT_EQ_STR(path_extension(&b, _0("./README.md")), _(".md"));
        EXPECT_EQ_STR(path_extension(&b, _0("/home/x/README.md")), _(".md"));
        EXPECT_EQ_STR(path_extension(&b, _0("abc.tar.gz")), _(".tar.gz"));
        EXPECT_EQ_STR(path_extension(&b, _0("abc.def/abc.tar.gz")), _(".tar.gz"));
        EXPECT_EQ_STR(path_extension(&b, _0("abc.def/.tar.gz")), _(".tar.gz"));

        buf_reset(&b);
        EXPECT_EQ_STR(Str_(path_join(&b, _0("/"), _0("README.md"))), _("/README.md"));
        EXPECT_EQ_STR(Str_(path_join(&b, _0("/"), _0("/README.md"))), _("//README.md"));
        EXPECT_EQ_STR(Str_(path_join(&b, _0("/a/b"), _0("/README.md"))), _("/a/b//README.md"));
        EXPECT_EQ_STR(Str_(path_join(&b, _0("/a/b/"), _0("README.md"))), _("/a/b/README.md"));
        EXPECT_EQ_STR(Str_(path_join(&b, _0("/a/b/"), _0("/README.md"))), _("/a/b//README.md"));

        buf_reset(&b);
        EXPECT_EQ_LONG(path_split(&b, _0(""), -1).len, 0L);
        EXPECT_EQ_LONG(path_split(&b, _0("/"), -1).len, 0L);
        EXPECT_EQ_LONG(path_split(&b, _0("/a/b/"), -1).len, 2L);
        EXPECT_EQ_LONG(path_split(&b, _0("///a/b/"), -1).len, 2L);
    }

    TEST_CASE("Read, Write, Delete")
    {
        Buf b = buf_new(MB_);

        int err = 0;
        Str dst = {};

        err = path_read_text(&b, _0(__FILE__), &dst);
        EXPECT_EQ_INT(err, 0);

        Str0 temp = _0("/tmp/file_write.txt");

        err = path_write_text(temp, dst, "w");
        EXPECT_EQ_INT(err, 0);

        Str dst2 = {};

        err = path_read_text(&b, temp, &dst2);
        EXPECT_EQ_INT(err, 0);

        EXPECT_EQ_STR(dst, dst2);

        err = path_delete(temp);
        EXPECT_EQ_INT(err, 0);

        err = path_read_text(&b, temp, &dst);
        EXPECT_NEQ_INT(err, 0);

        buf_free(&b);
    }

    return TEST_RESULTS();
}
