#include "stz2/args.h"

#define E2BIG  7  /* Argument list too long */
#define EINVAL 22 /* Invalid argument */

int args_parse(Buf* a, Args* args, int argc, char* argv[], bool strict)
{
    // Only place where arena is used, but keeps strings which we need
    struct option* long_options = make(a, struct option, args->len + 1, ALLOC_ZERO);
    RANGE(i, args->len)
    {
        long_options[i] = (struct option){
            .name    = args->buf[i].name.buf,
            .has_arg = args->buf[i].required,
            .flag    = 0,
            .val     = 0, // TODO No short options for now
        };
    }

    int current;

    while (1)
    {
        // TODO: Some say that setting this will reset, but what to set it to then??
        // https://stackoverflow.com/questions/60483737/whats-the-portable-way-to-reset-getopt
        int option_index = 0;

        current = getopt_long(argc, argv, "", long_options, &option_index);
        if (current == -1) break;

        switch (current)
        {
        case 0:
            if (optarg)
            {
                args->buf[option_index].found = true;

                void* val = args->buf[option_index].val;
                switch (args->buf[option_index].type)
                {
                case ARG_Invalid:       // Unhandled cases
                case ARG_True:          //
                case ARG_False:         //
                case ARG_Bool:          // Only toggle, so no args
                case ARG_Array:         //
                case ARG_Object:        //
                case ARG_IsReference:   //
                case ARG_Raw_JSON:      //
                case ARG_StringIsConst: //
                case ARG_NULL: break;   //

                case ARG_Float: // TODO: error handling for atof
                    float* f = (float*)val;
                    *f       = atof(optarg);
                    break;

                case ARG_Integer: // TODO: error handling for atoi
                    i32* x = (i32*)val;
                    *x     = atoi(optarg);
                    break;

                case ARG_String:
                    Str* s = (Str*)val;
                    *s     = (Str){.buf = optarg, .len = strlen(optarg)};
                    break;
                }
            }
            else
            {
                void* val = args->buf[option_index].val;
                if (args->buf[option_index].type == ARG_Bool)
                {
                    bool* bt = (bool*)val;
                    *bt      = !(*bt); // Toggle the default value
                }
            }
            break;

        case '?': break;
        default: p_line("?? getopt returned character code 0%o ??\n", current);
        }
    }

    if (strict && (optind < argc)) return -E2BIG; // Arg list too big

    if (optind < argc)
    {
        p_line("non-option ARGV-elements: ");
        while (optind < argc) p_line("%s ", argv[optind++]);
        p_line("\n");
    }

    // option.required has nothing to do with semantically being necessary
    //   RANGE(i, args->len)
    //   {
    //       int err = (args->buf[i].required == required_argument) && !args->buf[i].found;
    //       OnError_Return(err, -EINVAL, "Required arg not found: %.*s", _s(args->buf[i].name));
    //   }

    return 0;
}

void args_dump_help(Args* c)
{
    p_line("USAGE: %.*s", _s(c->usage));
    isize maxname = 0;
    RANGE(i, c->len)
    {
        p_line("  --%-15.*s    %.*s", //
               _s(c->buf[i].name),    //
               _s(c->buf[i].help));
        maxname = maxname < c->buf[i].name.len ? c->buf[i].name.len : maxname;
    }
}

void args_dump_values(Args* c)
{
    RANGE(i, c->len)
    {
        if (c->buf[i].val)
        {
            Str   name  = c->buf[i].name;
            void* val   = c->buf[i].val;
            bool  found = c->buf[i].found;
            switch (c->buf[i].type)
            {
            case ARG_Invalid:
            case ARG_Array:
            case ARG_Object:
            case ARG_IsReference:
            case ARG_Raw_JSON:
            case ARG_StringIsConst:
            case ARG_True:  // These are invalid
            case ARG_False: // as type is bool
            case ARG_NULL: break;

            case ARG_Bool:
                bool b = *(bool*)val;
                p_line("%-10.*s [%d]= %s", _s(name), found, b ? "true" : "false");
                break;

            case ARG_Float:
                float f = *(float*)val;
                p_line("%-10.*s [%d]= %f", _s(name), found, f);
                break;

            case ARG_Integer:
                i32 x = *(i32*)val;
                p_line("%-10.*s [%d]= %d", _s(name), found, x);
                break;

            case ARG_String:
                Str s = *(Str*)val;
                p_line("%-10.*s [%d]= %.*s", _s(name), found, _s(s));
                break;
            }
        }
    }
}
