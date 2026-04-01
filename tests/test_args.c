#include "stz2/args.h"

#include "stz2/test.h"

#define PATH_MAX 4096 /* # chars in a path name including nul */
#define E2BIG    7    /* Argument list too long */
#define EINVAL   22   /* Invalid argument */

int main(int argc, char* argv[])
{
    TEST_SUITE("stz/args.h");

    Buf perm = buf_new(16 * MB_);
    Buf temp = perm;

    TEST_CASE("Size") { EXPECT_EQ_LONG(sizeof(struct option), 32L); }

    TEST_CASE("New")
    {
        int err = 0;

        temp = perm;

        struct
        {
            bool  help;
            bool  nohelp;
            Str   path;
            float gain;
            i32   channel;
        } cfg = {.path = _("./data"), .channel = 2};

        // clang-format off
        Arg options[] = {
            {_("help"),    no_argument,       0, _("Show help"),        ARG_Bool,    &cfg.help   , false},
            {_("nohelp"),  no_argument,       0, _("Show nohelp"),      ARG_Bool,    &cfg.nohelp , false},
            {_("path"),    required_argument, 0, _("Input path"),       ARG_String,  &cfg.path   , false},
            {_("gain"),    required_argument, 0, _("Capture gain"),     ARG_Float,   &cfg.gain   , false},
            {_("channel"), required_argument, 0, _("Capture channels"), ARG_Integer, &cfg.channel, false},
        };
        // clang-format on

        Args args = {.len = countof(options), .buf = options, .usage = _0("some_program [OPTIONS]")};

        args_dump_help(&args);

        char* argv[] = {__FILE__, "--path", "./", "--gain", "0.25", "--help", "--channel", "5"};
        isize argc   = countof(argv);

        err = args_parse(&temp, &args, argc, argv, true);
        EXPECT_EQ_INT(err, 0);

        p_line("cfg:");
        p_line("  path   : %.*s", _s(cfg.path));
        p_line("  gain   : %f", cfg.gain);
        p_line("  channel: %d", cfg.channel);
        p_line("  help   : %d", cfg.help);
        p_line("  nohelp : %d", cfg.nohelp);

        // NOTE: Somehow seems like getopt maintains state, so we cannot test twice
        // char* argv2[] = {__FILE__, "--gain", "0.25", "--help"};
    };

    buf_free(&perm);

    TEST_RESULTS();
}
