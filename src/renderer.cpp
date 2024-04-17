#include "renderer.h"

#include <cmath>
#include <numbers>
#include <vector>

void drawTriangle(std::shared_ptr<SDL_Renderer> renderer, float x1, float y1, float x2, float y2, float x3, float y3, float r, float g, float b, float a) {
  SDL_Vertex v[3] = {0};
  v[0].position   = {x1, y1};
  v[1].position   = {x2, y2};
  v[2].position   = {x3, y3};
  v[0].color      = {r, g, b, a};
  v[1].color      = {r, g, b, a};
  v[2].color      = {r, g, b, a};
  SDL_RenderGeometry(renderer.get(), nullptr, v, 3, nullptr, 0);
}

void drawTriangle(std::shared_ptr<SDL_Renderer> renderer,
                  float x1, float y1, float x2, float y2, float x3, float y3,
                  float r1, float g1, float b1, float a1,
                  float r2, float g2, float b2, float a2,
                  float r3, float g3, float b3, float a3) {
  SDL_Vertex v[3] = {0};
  v[0].position   = {x1, y1};
  v[1].position   = {x2, y2};
  v[2].position   = {x3, y3};
  v[0].color      = {r1, g1, b1, a1};
  v[1].color      = {r2, g2, b2, a2};
  v[2].color      = {r3, g3, b3, a3};
  SDL_RenderGeometry(renderer.get(), nullptr, v, 3, nullptr, 0);
}

void draw_circle_flashlight(std::shared_ptr<SDL_Renderer> renderer, float x, float y, float radius, int edges,
                            float cr, float cg, float cb, float ca,
                            float cor, float cog, float cob, float coa,
                            float otr, float otg, float otb, float ota) {
  if (edges < 5) { // wont work with edges less than 5
    return;
  }

  int width, height;
  SDL_GetCurrentRenderOutputSize(renderer.get(), &width, &height);

  radius = std::abs(radius);

  SDL_FPoint world_bound[4] = {
      {0, 0},
      {(float)width, 0},
      {(float)width, (float)height},
      {0, (float)height},
  };

  SDL_FPoint world_bound_mid[4] = {
      SDL_PointMid(world_bound[0], world_bound[1]),
      SDL_PointMid(world_bound[1], world_bound[2]),
      SDL_PointMid(world_bound[2], world_bound[3]),
      SDL_PointMid(world_bound[3], world_bound[0]),
  };

  SDL_FPoint bounds[4] = {
      {x - radius, y - radius},
      {x + radius, y - radius},
      {x + radius, y + radius},
      {x - radius, y + radius},
  };

  SDL_FPoint bound_mids[4] = {
      SDL_PointMid(bounds[0], bounds[1]),
      SDL_PointMid(bounds[1], bounds[2]),
      SDL_PointMid(bounds[2], bounds[3]),
      SDL_PointMid(bounds[3], bounds[0]),
  };

  // LIGHT

  for (int i = 0; i < 4; i++) {
    drawTriangle(renderer,
                 world_bound[i].x, world_bound[i].y,
                 world_bound[(i + 1) % 4].x, world_bound[(i + 1) % 4].y,
                 bound_mids[i].x, bound_mids[i].y,
                 otr, otg, otb, ota);
  }

  float a0 = 2 * std::numbers::pi_v<float> / edges;
  float r1 = std::numbers::pi_v<float> * 0.5f + (std::numbers::pi_v<float> / edges);
  for (int i = 0; i < edges; ++i) {
    float a1 = std::fmod((a0 * i + r1), 2.0f * std::numbers::pi_v<float>);
    float a2 = std::fmod((a0 * (i + 1) + r1), 2.0f * std::numbers::pi_v<float>);
    if (a1 < 0) a1 += 2.0f * std::numbers::pi_v<float>;
    if (a2 < 0) a2 += 2.0f * std::numbers::pi_v<float>;

    float x1 = x + radius * std::cos(a1);
    float y1 = y + radius * std::sin(a1);
    float x2 = x + radius * std::cos(a2);
    float y2 = y + radius * std::sin(a2);

    // OUTSIDE LIGHT IN BOUNDS

    int index         = -1;
    bool in_same_quad = false;

    if (a1 < std::numbers::pi_v<float> / 2) {
      in_same_quad = a2 < std::numbers::pi_v<float> / 2;
      index        = 0;
    } else if (a1 < std::numbers::pi_v<float>) {
      in_same_quad = a2 < std::numbers::pi_v<float>;
      index        = 1;
    } else if (a1 < 3 * std::numbers::pi_v<float> / 2) {
      in_same_quad = a2 < 3 * std::numbers::pi_v<float> / 2;
      index        = 2;
    } else {
      in_same_quad = a2 >= std::numbers::pi_v<float> / 2;
      index        = 3;
    }

    if (in_same_quad) {
      drawTriangle(renderer,
                   x1, y1,
                   x2, y2,
                   world_bound[(index + 2) % 4].x, world_bound[(index + 2) % 4].y,
                   otr, otg, otb, ota);
    } else {
      drawTriangle(renderer,
                   x1, y1,
                   bound_mids[(index + 2) % 4].x, bound_mids[(index + 2) % 4].y,
                   world_bound[(index + 2) % 4].x, world_bound[(index + 2) % 4].y,
                   otr, otg, otb, ota);

      drawTriangle(renderer,
                   x1, y1,
                   x2, y2,
                   bound_mids[(index + 2) % 4].x, bound_mids[(index + 2) % 4].y,
                   otr, otg, otb, ota);

      drawTriangle(renderer,
                   x2, y2,
                   bound_mids[(index + 2) % 4].x, bound_mids[(index + 2) % 4].y,
                   world_bound[(index + 3) % 4].x, world_bound[(index + 3) % 4].y,
                   otr, otg, otb, ota);
    }

    // LIGHT
    drawTriangle(renderer,
                 x, y,
                 x1, y1,
                 x2, y2,
                 cr, cg, cb, ca,
                 cor, cog, cob, coa,
                 cor, cog, cob, coa);
  }
}

void draw_rect_flashlight(std::shared_ptr<SDL_Renderer> renderer, float x, float y, float w, float h, float inr, float ing, float inb, float ina, float outr, float outg, float outb, float outa) {
  SDL_FPoint rect_bounds[4] = {
      {x, y},
      {x + w, y},
      {x + w, y + h},
      {x, y + h},
  };

  int width, height;
  SDL_GetCurrentRenderOutputSize(renderer.get(), &width, &height);

  drawTriangle(renderer,
               rect_bounds[0].x, rect_bounds[0].y,
               rect_bounds[1].x, rect_bounds[1].y,
               rect_bounds[2].x, rect_bounds[2].y,
               inr, ing, inb, ina);
  drawTriangle(renderer,
               rect_bounds[0].x, rect_bounds[0].y,
               rect_bounds[2].x, rect_bounds[2].y,
               rect_bounds[3].x, rect_bounds[3].y,
               inr, ing, inb, ina);

  drawTriangle(renderer,
               0.0f, 0.0f,
               width, 0.0f,
               width, rect_bounds[0].y,
               outr, outg, outb, outa);
  drawTriangle(renderer,
               0.0f, 0.0f,
               0.0f, rect_bounds[0].y,
               width, rect_bounds[0].y,
               outr, outg, outb, outa);

  drawTriangle(renderer,
               width, height,
               width, rect_bounds[2].y,
               0.0f, rect_bounds[2].y,
               outr, outg, outb, outa);
  drawTriangle(renderer,
               width, height,
               0.0f, height,
               0.0f, rect_bounds[2].y,
               outr, outg, outb, outa);

  drawTriangle(renderer,
               0.0f, rect_bounds[0].y,
               rect_bounds[0].x, rect_bounds[0].y,
               rect_bounds[3].x, rect_bounds[3].y,
               outr, outg, outb, outa);
  drawTriangle(renderer,
               0.0f, rect_bounds[0].y,
               0.0f, rect_bounds[3].y,
               rect_bounds[3].x, rect_bounds[3].y,
               outr, outg, outb, outa);

  drawTriangle(renderer,
               rect_bounds[1].x, rect_bounds[1].y,
               width, rect_bounds[1].y,
               width, rect_bounds[2].y,
               outr, outg, outb, outa);
  drawTriangle(renderer,
               rect_bounds[1].x, rect_bounds[1].y,
               rect_bounds[2].x, rect_bounds[2].y,
               width, rect_bounds[2].y,
               outr, outg, outb, outa);
}

SDL_FPoint SDL_PointMid(float x1, float y1, float x2, float y2) {
  return SDL_FPoint{
      (x1 + x2) * 0.5f,
      (y1 + y2) * 0.5f,
  };
}

SDL_FPoint SDL_PointMid(const SDL_FPoint& p1, const SDL_FPoint& p2) {
  return SDL_PointMid(p1.x, p1.y, p2.x, p2.y);
}
