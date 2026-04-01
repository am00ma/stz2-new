#pragma once

#include "stz2/types.h"

#include <getopt.h>

// Mirroring cJSON Types:
typedef enum
{
    ARG_Invalid       = 0,
    ARG_False         = 1 << 0,
    ARG_True          = 1 << 1,
    ARG_NULL          = 1 << 2,
    ARG_Float         = 1 << 3, // Number in cJSON
    ARG_String        = 1 << 4,
    ARG_Array         = 1 << 5,
    ARG_Object        = 1 << 6,
    ARG_Raw_JSON      = 1 << 7,
    ARG_IsReference   = 1 << 8,
    ARG_StringIsConst = 1 << 9,
    ARG_Bool          = 1 << 10, // Extra detailed types
    ARG_Integer       = 1 << 11,

} ArgType;

typedef struct
{
    Str  name;
    int  required; // FIXME: This has nothing to do with being a necessary arg
    char shortopt;
    Str  help;

    ArgType type;  // Mirroring cJSON
    void*   val;   // Should directly point to struct to avoid copy
    bool    found; // Getopt does not enforce required, se we have to

} Arg;

typedef struct
{
    Arg*  buf;
    isize len;

    Str0 usage;

} Args;

void args_dump_help(Args* c);
void args_dump_values(Args* c);
int  args_parse(Buf* b, Args* args, int argc, char* argv[], bool strict);
