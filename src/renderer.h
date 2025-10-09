#ifndef _RENDERER_H_
#define _RENDERER_H_

#include "SDL3/SDL.h"

void draw_circle_flashlight(SDL_Renderer* renderer, float x, float y, float radius, int edges, float cr, float cg, float cb, float ca, float cor, float cog, float cob, float coa, float otr, float otg, float otb, float ota);
void draw_rect_flashlight(SDL_Renderer* renderer, float x, float y, float w, float h, float inr, float ing, float inb, float ina, float outr, float outg, float outb, float outa);

SDL_FPoint SDL_PointMid(float x1, float y1, float x2, float y2);

#endif