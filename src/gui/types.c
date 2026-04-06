#include "stz2/gui/types.h"
#include "stz2/gui/draw.h"

int window_init(Window* w, u32 width, u32 height, isize fontsize, void* data)
{
    int err = 0;

    w->quit     = false;
    w->bounds   = (SDL_Rect){0, 0, width, height};
    w->fontsize = fontsize;
    w->data     = data;

    err = SDL_Init(SDL_INIT_VIDEO);
    OnError_Return(err, err, "Failed: SDL_Init");

    SDL_ShowCursor(0);

    w->win = SDL_CreateWindow("SDLX", 100, 100, w->bounds.w, w->bounds.h, SDL_WINDOW_SHOWN);
    err    = -(!w->win);
    OnError_Return(err, err, "Failed: SDL_CreateWindow");

    w->rnd = SDL_CreateRenderer(w->win, -1, SDL_RENDERER_ACCELERATED);
    err    = -(!w->rnd);
    OnError_Return(err, err, "Failed: SDL_CreateRenderer");

    SDL_RendererInfo info;
    SDL_GetRendererInfo(w->rnd, &info);
    p_info("Renderer: %s", info.name);

    SDL_Color black = {0, 0, 0, 255};
    RANGE(i, 4)
    {
        w->fonts[i] = FC_CreateFont();

        err = !FC_LoadFont(w->fonts[i], w->rnd, FontPaths[i].buf, w->fontsize, black, TTF_STYLE_NORMAL);
        OnError_Return(err, err, "Failed: TTF_OpenFont: %.*s", _s(FontPaths[i]));
    }

    return 0;
}

void window_destroy(Window* w)
{
    if (!w) return;

    RANGE(i, 4) { FC_FreeFont(w->fonts[i]); }
    if (w->rnd) SDL_DestroyRenderer(w->rnd);
    if (w->win) SDL_DestroyWindow(w->win);

    SDL_Quit();
}

void window_dump(Window* w) { p_info("  w, h: %d, %d", w->bounds.w, w->bounds.h); }

void window_update(Window* w, SDL_Event event, void*)
{
    switch (event.type)
    {
    case SDL_QUIT: w->quit = true; break;
    case SDL_KEYDOWN:
        switch (event.key.keysym.sym)
        {
        case SDLK_ESCAPE: w->quit = true; break;
        }
    }
}

void window_render(Window* w, SDL_Renderer* r, void*) { draw_filled(r, w->bounds, (SDL_Color){80, 20, 20, 20}); }
