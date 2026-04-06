#pragma once

#include "stz2/types.h"

typedef struct WebFile WebFile;

typedef struct WebFile
{
    Str0 path;

    isize    idx;
    WebFile* prev;
    WebFile* next;

    Time loaded;
    Time used;

    isize size;
    Str   data;

} WebFile;

SI bool  web_file_valid(WebFile* w) { return w->size == w->data.len; }
SI isize web_file_progress(WebFile* w) { return w->size ? (w->data.len * 100) / (w->size) : 0; }

typedef struct
{
    StrSet paths; // Indexes into files

    WebFile** files;
    WebFile*  oldest;
    WebFile*  newest;

    Buf mem; // We want file buffers to be in scope when using WebRegistry
} WebRegistry;

WebRegistry web_registry_new(isize mem_cap, isize exp);
void        web_registry_free(WebRegistry* w);

// TODO: Clean up API as neither path should return any errors
int web_registry_path_lookup(WebRegistry* w, Str0 path, WebFile** data);
int web_registry_path_delete(WebRegistry* w);
