#pragma once

#include "stz2/gui/types.h"

typedef struct GuiwListview GuiwListview;

typedef void (*GuiwListviewCb_Select)(GuiwListview* lv);
typedef Str (*GuiwListviewCb_ItemToText)(Buf* a, isize idx, void* item);

typedef struct GuiwListview
{
    void* items;

    isize len;
    isize now;

    // TODO: Layout
    i64s index;

    // TODO: Keymaps
    // TODO: Styles

} GuiwListview;

void guiw_listview_update(Widget* c, SDL_Event* e, void* data);
void guiw_listview_render(Widget* c, SDL_Renderer* r, void* data);

Widget guiw_listview_new(void* items, isize len)
{
    GuiwListview detail = {
        .items = items,
        .len   = len,
        .now   = 0,
    };
    return (Widget){
        .update = guiw_listview_update,
        .render = guiw_listview_render,
        .detail = &detail,
    };
}
