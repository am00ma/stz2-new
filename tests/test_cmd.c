#include "stz2/cmd.h"

#include "stz2/test.h"

int main(int argc, char* argv[])
{
    TEST_SUITE("stz-util/cmd.h");

    Buf perm = buf_new(MB_);

    isize out_maxlen = 1024;
    isize err_maxlen = 1024;

    TEST_CASE("Valid command")
    {
        Buf temp = perm;
        int err;

        CmdResult result   = {};
        Str0      command  = _0("ls CMake* | sort");
        Str       expected = _("CMakeLists.txt\nCMakePresets.json\n");

        err = cmd_exec(   //
            &temp,        //
            command,      //
            CmdShellBash, //
            out_maxlen,   //
            err_maxlen,   //
            &result);
        EXPECT_EQ_INT(err, 0);

        EXPECT_EQ_INT(result.status, 0);
        EXPECT_EQ_LONG(result.err.len, 0L);

        EXPECT_EQ_STR(result.out, expected);
        EXPECT_EQ_LONG(temp.len, expected.len);
    }

    TEST_CASE("Invalid command")
    {
        Buf temp = perm;
        int err;

        CmdResult result   = {};
        Str0      command  = _0("abcdef");
        Str       expected = _("/bin/bash: line 1: abcdef: command not found\n");

        err = cmd_exec(   //
            &temp,        //
            command,      //
            CmdShellBash, //
            out_maxlen,   //
            err_maxlen,   //
            &result);
        EXPECT_EQ_INT(err, 0);

        EXPECT_EQ_INT(result.status, 127);
        EXPECT_EQ_STR(result.err, expected);
        EXPECT_EQ_LONG(temp.len, expected.len);
    }

    buf_free(&perm);

    return TEST_RESULTS();
}
