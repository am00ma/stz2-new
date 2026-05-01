#include "stz2/web/registry.h"

WebRegistry web_registry_new(isize mem_cap, isize exp)
{
    Buf mem = buf_new(mem_cap);
    return (WebRegistry){
        .paths  = strset_new(&mem, exp),
        .files  = make(&mem, WebFile*, (1 << exp), ALLOC_ZERO),
        .oldest = NULL,
        .newest = NULL,
        .mem    = mem,
    };
}
void web_registry_free(WebRegistry* w) { buf_free(&w->mem); }

int web_registry_path_lookup(WebRegistry* w, Str0 path, WebFile** data)
{
    // Return if already available
    isize idx = strset_lookup(&w->paths, Str_(path));
    if (idx >= 0)
    {
        *data = w->files[idx];
        return 0;
    }

    // Allocate new data
    WebFile* d;
    d = make(&w->mem, WebFile);

    // --  'Critical section'
    idx = strset_insert(&w->paths, Str_(path)); // insert key to get index
    *d  = (WebFile){
        .path = path,
        .idx  = idx,
        .next = NULL,
        .prev = w->newest,
        .size = 0,
    };
    w->files[idx] = d;                                        // set file
    if (!w->oldest) w->oldest = w->files[idx];                // if first file
    if (w->newest) { w->newest->prev->next = w->files[idx]; } // link newest if exists
    w->newest = w->files[idx];                                // set/replace newest
    // -- done

    *data = d;

    return 0;
}

int web_registry_path_delete(WebRegistry* w)
{
    if (!w->oldest) return -1;

    // --  'Critical section'
    WebFile* f = w->oldest;                   // get data
    Assert((!f->prev), "");                   // (should be null, as we are oldest)
    if (f->next) f->next->prev = NULL;        // remove oldest from next
    if (w->newest == f) { w->newest = NULL; } // update newest (should be null if only file)
    w->oldest = f->next;                      // update oldest (note comparison of pointers)
    // NOTE: f->idx was indexed at `lookup` otherwise would need strset_delete(Str key)
    w->files[f->idx] = NULL; // delete it
    // TODO: strset_delete_idx(isize idx) is needed to manage len
    w->paths.buf[f->idx] = StrNull; // delete key
    w->paths.len--;                 // decrement length
    // -- done

    return 0;
}
