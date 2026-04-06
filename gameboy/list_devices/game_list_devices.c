#include "stz2/gui/types.h"

int main()
{
    int    err = 0;
    Window w   = {};
    err        = window_init(&w, 640, 480, 24, 0);
    OnError_Goto(err, __close_init, "Failed: window_init");

    window_dump(&w);

    u32 frame_period = (1000 / 60);
    u32 frame_start;
    u32 frame_stop;

    u32 max_frames = 60; // 1 secs
    u32 frames     = 0;

    while (!w.quit && (frames < max_frames))
    {
        frame_start = SDL_GetTicks();

        SDL_Event event;
        while (SDL_PollEvent(&event)) { window_update(&w, event, 0); }

        window_render(&w, w.rnd, 0);

        SDL_RenderPresent(w.rnd);

        frame_stop = SDL_GetTicks() - frame_start;
        if (frame_stop < frame_period) SDL_Delay(frame_period - frame_stop);

        frames++;
    }

__close_init:
    window_destroy(&w);

    return 0;
}
