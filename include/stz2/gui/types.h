#pragma once

/* ---------------------------------------------------------------------------
 * Includes
 * ------------------------------------------------------------------------- */

#include "stz2/types.h"

#include <SDL2/SDL.h>

#include "stz2/gui/SDL_FontCache.h"
#include "stz2/gui/assets.h"

/* ---------------------------------------------------------------------------
 * Widget
 * ------------------------------------------------------------------------- */

typedef struct Widget Widget;
typedef struct Window Window;

typedef void (*UpdateCallback)(Widget* c, SDL_Event* e, void* data);
typedef void (*RenderCallback)(Widget* c, SDL_Renderer* r, void* data);

typedef struct Widget
{
    // Specialization for each widget
    UpdateCallback update;
    RenderCallback render;
    void*          detail;

    // Layout
    SDL_Rect bounds;

    // TODO: Style

    // Composition
    Window* window;
    Widget* parent;
    Widget* children;
    isize   num_children;

    // Focus
    bool  can_focus;
    bool  has_focus;
    isize focus_child; // -1 if no focus_child; only non-zero init member

    // TODO: Animation

} Widget;

/* ---------------------------------------------------------------------------
 * Assets
 * ------------------------------------------------------------------------- */

typedef enum
{

    FONT_NORMAL     = 0,
    FONT_BOLD       = 1,
    FONT_ITALIC     = 2,
    FONT_BOLDITALIC = 3,

} FontStyle;

static const Str FontPaths[] = {
    [FONT_NORMAL]     = _(STZ_FONT_NORMAL),
    [FONT_BOLD]       = _(STZ_FONT_BOLD),
    [FONT_ITALIC]     = _(STZ_FONT_ITALIC),
    [FONT_BOLDITALIC] = _(STZ_FONT_BOLDITALIC),
};

/* ---------------------------------------------------------------------------
 * Window
 * ------------------------------------------------------------------------- */

typedef struct Window
{
    bool quit;

    SDL_Rect      bounds;
    SDL_Window*   win;
    SDL_Renderer* rnd;

    isize    fontsize;
    FC_Font* fonts[4];

    Widget* root;

    void* data;

} Window;

int  window_init(Window* w, u32 width, u32 height, isize fontsize, void* data);
void window_destroy(Window* w);
void window_dump(Window* w);

void window_update(Window* w, SDL_Event* event, void* data); // SDL_Event* allows to change event in chain
void window_render(Window* w, SDL_Renderer* r, void* data);
