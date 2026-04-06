#include "stz2/gui/draw.h"

void draw_rect(SDL_Renderer* r, SDL_Rect rect, SDL_Color color)
{
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a);
    SDL_RenderDrawRect(r, &rect);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
}

void draw_filled(SDL_Renderer* r, SDL_Rect rect, SDL_Color color)
{
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a);
    SDL_RenderFillRect(r, &rect);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_NONE);
    SDL_SetRenderDrawColor(r, 255, 255, 255, 255);
}

void draw_text(SDL_Renderer* r, FC_Font* font, char* text, SDL_Rect rect, SDL_Color bg)
{
    draw_filled(r, rect, bg);
    SDL_RenderSetClipRect(r, &rect);
    FC_DrawBox(font, r, rect, text);
    SDL_RenderSetClipRect(r, NULL);
}
