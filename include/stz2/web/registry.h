#pragma once

#include "stz2/types.h"

typedef struct
{
    Str0 path;

    Time loaded;
    Time used;

    isize size;
    Str   data;

} WebFile;

SI bool  web_file_valid(WebFile* w) { return w->size == w->data.len; }
SI isize web_file_progress(WebFile* w) { return w->size ? (w->data.len * 100) / (w->size) : 0; }

typedef struct
{
    WebFile* buf;
    isize    len;
    isize    exp;
} WebRegistry;

WebRegistry web_registry_new(isize exp);
void        web_registry_free(WebRegistry* w);

int web_registry_path_lookup(WebRegistry* w, Str0 path, Str* data);
int web_registry_path_cache(WebRegistry* w, Str0 path);
int web_registry_path_delete(WebRegistry* w, Str0 path);
