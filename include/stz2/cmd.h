#pragma once

#include "stz2/types.h"

#define CmdShellBash _0("/bin/bash")
#define CmdShellFish _0("/bin/fish")

typedef struct
{

    Str out;
    Str err;
    i32 status;

} CmdResult;

int cmd_exec( //
    Buf*       a,
    Str0       command,
    Str0       shell,
    isize      out_maxlen,
    isize      err_maxlen,
    CmdResult* result);
