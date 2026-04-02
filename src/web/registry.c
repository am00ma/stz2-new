#include "stz2/web/registry.h"

WebRegistry web_registry_new(isize mem_cap, isize exp)
{
    Buf mem = buf_new(mem_cap);
    return (WebRegistry){
        .paths  = strset_new(&mem, exp),
        .files  = make(&mem, WebFile*, (1 << exp), ALLOC_ZERO),
        .oldest = -1,
        .newest = -1,
        .mem    = mem,
    };
}
void web_registry_free(WebRegistry* w) { buf_free(&w->mem); }

int web_registry_path_cache(WebRegistry* w, Str0 path)
{
    // Check if it already exists
    isize idx = strset_lookup(&w->paths, Str_(path));
    if (idx >= 0) { return -1; } // Can overwrite if needed

    // Allocate new data
    WebFile* data;
    data  = make(&w->mem, WebFile);
    *data = (WebFile){.path = path, .size = 0};

    // --  'Critical section'
    idx           = strset_insert(&w->paths, Str_(path)); // insert key to get index
    w->files[idx] = data;                                 // set file
    // -- done

    return 0;
}

int web_registry_path_lookup(WebRegistry* w, Str0 path, WebFile** data)
{
    isize idx = strset_lookup(&w->paths, Str_(path));
    if (idx < 0)
    {
        *data = NULL;
        return -1;
    }
    *data = w->files[idx];
    return 0;
}

int web_registry_path_delete(WebRegistry* w, Str0 path)
{
    isize idx = strset_lookup(&w->paths, Str_(path));
    if (idx < 0) { return -1; }
    w->paths.buf[idx] = StrNull;
    return 0;
}
