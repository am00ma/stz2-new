#pragma once

#include "stz2/types.h"

bool path_exists(Str0 path);
Str  path_basename(Buf* b, Str0 path);
Str  path_dirname(Buf* b, Str0 path);
Str  path_extension(Buf* b, Str0 path);
Strs path_split(Buf* b, Str0 path, isize maxlen);
Str0 path_join(Buf* b, Str0 parent, Str0 path);

int path_delete(Str0 path);
int path_read_text(Buf* b, Str0 path, Str* dst);
int path_write_text(Str0 path, Str src, const char* mode);
