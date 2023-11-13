#ifndef _RENDERER_H_
#define _RENDERER_H_

#include <memory>

#include "SDL3/SDL.h"

void draw_circle_flashlight(std::shared_ptr<SDL_Renderer> renderer, float x, float y, float radius, int edges, uint8_t cr, uint8_t cg, uint8_t cb, uint8_t ca, uint8_t cor, uint8_t cog, uint8_t cob, uint8_t coa, uint8_t otr, uint8_t otg, uint8_t otb, uint8_t ota);
void draw_rect_flashlight(std::shared_ptr<SDL_Renderer> renderer, float x, float y, float w, float h, uint8_t inr, uint8_t ing, uint8_t inb, uint8_t ina, uint8_t outr, uint8_t outg, uint8_t outb, uint8_t outa);

SDL_FPoint SDL_PointMid(float x1, float y1, float x2, float y2);
SDL_FPoint SDL_PointMid(const SDL_FPoint& p1, const SDL_FPoint& p2);

#endif