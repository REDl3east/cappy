#include "renderer.h"

#include <math.h>

#define RENDERER_PI (3.14159265358979323846)

void draw_triangle_rgb(SDL_Renderer* renderer, float x1, float y1, float x2, float y2, float x3, float y3, float r, float g, float b, float a) {
  SDL_Vertex v[3] = {0};
  v[0].position.x = x1;
  v[0].position.y = y1;
  v[1].position.x = x2;
  v[1].position.y = y2;
  v[2].position.x = x3;
  v[2].position.y = y3;
  v[0].color.r    = r;
  v[0].color.g    = g;
  v[0].color.b    = b;
  v[0].color.a    = a;
  v[1].color.r    = r;
  v[1].color.g    = g;
  v[1].color.b    = b;
  v[1].color.a    = a;
  v[2].color.r    = r;
  v[2].color.g    = g;
  v[2].color.b    = b;
  v[2].color.a    = a;
  SDL_RenderGeometry(renderer, NULL, v, 3, NULL, 0);
}

void draw_triangle(SDL_Renderer* renderer,
                   float x1, float y1, float x2, float y2, float x3, float y3,
                   float r1, float g1, float b1, float a1,
                   float r2, float g2, float b2, float a2,
                   float r3, float g3, float b3, float a3) {
  SDL_Vertex v[3] = {0};
  v[0].position.x = x1;
  v[0].position.y = y1;
  v[1].position.x = x2;
  v[1].position.y = y2;
  v[2].position.x = x3;
  v[2].position.y = y3;
  v[0].color.r    = r1;
  v[0].color.g    = g1;
  v[0].color.b    = b1;
  v[0].color.a    = a1;
  v[1].color.r    = r2;
  v[1].color.g    = g2;
  v[1].color.b    = b2;
  v[1].color.a    = a2;
  v[2].color.r    = r3;
  v[2].color.g    = g3;
  v[2].color.b    = b3;
  v[2].color.a    = a3;
  SDL_RenderGeometry(renderer, NULL, v, 3, NULL, 0);
}

void draw_circle_flashlight(SDL_Renderer* renderer, float x, float y, float radius, int edges,
                            float cr, float cg, float cb, float ca,
                            float cor, float cog, float cob, float coa,
                            float otr, float otg, float otb, float ota) {
  if (edges < 5) { // wont work with edges less than 5
    return;
  }

  int width, height;
  SDL_GetCurrentRenderOutputSize(renderer, &width, &height);

  radius = fabsf(radius);

  SDL_FPoint world_bound[4] = {
      {0, 0},
      {(float)width, 0},
      {(float)width, (float)height},
      {0, (float)height},
  };

  SDL_FPoint world_bound_mid[4] = {
      SDL_PointMid(world_bound[0].x, world_bound[0].y, world_bound[1].x, world_bound[1].y),
      SDL_PointMid(world_bound[1].x, world_bound[1].y, world_bound[2].x, world_bound[2].y),
      SDL_PointMid(world_bound[2].x, world_bound[2].y, world_bound[3].x, world_bound[3].y),
      SDL_PointMid(world_bound[3].x, world_bound[3].y, world_bound[0].x, world_bound[0].y),
  };

  SDL_FPoint bounds[4] = {
      {x - radius, y - radius},
      {x + radius, y - radius},
      {x + radius, y + radius},
      {x - radius, y + radius},
  };

  SDL_FPoint bound_mids[4] = {
      SDL_PointMid(bounds[0].x, bounds[0].y, bounds[1].x, bounds[1].y),
      SDL_PointMid(bounds[1].x, bounds[1].y, bounds[2].x, bounds[2].y),
      SDL_PointMid(bounds[2].x, bounds[2].y, bounds[3].x, bounds[3].y),
      SDL_PointMid(bounds[3].x, bounds[3].y, bounds[0].x, bounds[0].y),
  };

  // LIGHT

  for (int i = 0; i < 4; i++) {
    draw_triangle_rgb(renderer,
                      world_bound[i].x, world_bound[i].y,
                      world_bound[(i + 1) % 4].x, world_bound[(i + 1) % 4].y,
                      bound_mids[i].x, bound_mids[i].y,
                      otr, otg, otb, ota);
  }

  float a0 = 2 * (float)RENDERER_PI / edges;
  float r1 = (float)RENDERER_PI * 0.5f + ((float)RENDERER_PI / edges);
  for (int i = 0; i < edges; ++i) {
    float a1 = fmodf((a0 * i + r1), 2.0f * (float)RENDERER_PI);
    float a2 = fmodf((a0 * (i + 1) + r1), 2.0f * (float)RENDERER_PI);
    if (a1 < 0) a1 += 2.0f * (float)RENDERER_PI;
    if (a2 < 0) a2 += 2.0f * (float)RENDERER_PI;

    float x1 = x + radius * cosf(a1);
    float y1 = y + radius * sinf(a1);
    float x2 = x + radius * cosf(a2);
    float y2 = y + radius * sinf(a2);

    // OUTSIDE LIGHT IN BOUNDS

    int index         = -1;
    bool in_same_quad = false;

    if (a1 < (float)RENDERER_PI / 2) {
      in_same_quad = a2 < (float)RENDERER_PI / 2;
      index        = 0;
    } else if (a1 < (float)RENDERER_PI) {
      in_same_quad = a2 < (float)RENDERER_PI;
      index        = 1;
    } else if (a1 < 3 * (float)RENDERER_PI / 2) {
      in_same_quad = a2 < 3 * (float)RENDERER_PI / 2;
      index        = 2;
    } else {
      in_same_quad = a2 >= (float)RENDERER_PI / 2;
      index        = 3;
    }

    if (in_same_quad) {
      draw_triangle_rgb(renderer,
                        x1, y1,
                        x2, y2,
                        world_bound[(index + 2) % 4].x, world_bound[(index + 2) % 4].y,
                        otr, otg, otb, ota);
    } else {
      draw_triangle_rgb(renderer,
                        x1, y1,
                        bound_mids[(index + 2) % 4].x, bound_mids[(index + 2) % 4].y,
                        world_bound[(index + 2) % 4].x, world_bound[(index + 2) % 4].y,
                        otr, otg, otb, ota);

      draw_triangle_rgb(renderer,
                        x1, y1,
                        x2, y2,
                        bound_mids[(index + 2) % 4].x, bound_mids[(index + 2) % 4].y,
                        otr, otg, otb, ota);

      draw_triangle_rgb(renderer,
                        x2, y2,
                        bound_mids[(index + 2) % 4].x, bound_mids[(index + 2) % 4].y,
                        world_bound[(index + 3) % 4].x, world_bound[(index + 3) % 4].y,
                        otr, otg, otb, ota);
    }

    // LIGHT
    draw_triangle(renderer,
                  x, y,
                  x1, y1,
                  x2, y2,
                  cr, cg, cb, ca,
                  cor, cog, cob, coa,
                  cor, cog, cob, coa);
  }
}

void draw_rect_flashlight(SDL_Renderer* renderer, float x, float y, float w, float h, float inr, float ing, float inb, float ina, float outr, float outg, float outb, float outa) {
  SDL_FPoint rect_bounds[4] = {
      {x, y},
      {x + w, y},
      {x + w, y + h},
      {x, y + h},
  };

  int width, height;
  SDL_GetCurrentRenderOutputSize(renderer, &width, &height);

  draw_triangle_rgb(renderer,
                    rect_bounds[0].x, rect_bounds[0].y,
                    rect_bounds[1].x, rect_bounds[1].y,
                    rect_bounds[2].x, rect_bounds[2].y,
                    inr, ing, inb, ina);
  draw_triangle_rgb(renderer,
                    rect_bounds[0].x, rect_bounds[0].y,
                    rect_bounds[2].x, rect_bounds[2].y,
                    rect_bounds[3].x, rect_bounds[3].y,
                    inr, ing, inb, ina);

  draw_triangle_rgb(renderer,
                    0.0f, 0.0f,
                    (float)width, 0.0f,
                    (float)width, rect_bounds[0].y,
                    outr, outg, outb, outa);
  draw_triangle_rgb(renderer,
                    0.0f, 0.0f,
                    0.0f, rect_bounds[0].y,
                    (float)width, rect_bounds[0].y,
                    outr, outg, outb, outa);

  draw_triangle_rgb(renderer,
                    (float)width, (float)height,
                    (float)width, rect_bounds[2].y,
                    0.0f, rect_bounds[2].y,
                    outr, outg, outb, outa);
  draw_triangle_rgb(renderer,
                    (float)width, (float)height,
                    0.0f, (float)height,
                    0.0f, rect_bounds[2].y,
                    outr, outg, outb, outa);

  draw_triangle_rgb(renderer,
                    0.0f, rect_bounds[0].y,
                    rect_bounds[0].x, rect_bounds[0].y,
                    rect_bounds[3].x, rect_bounds[3].y,
                    outr, outg, outb, outa);
  draw_triangle_rgb(renderer,
                    0.0f, rect_bounds[0].y,
                    0.0f, rect_bounds[3].y,
                    rect_bounds[3].x, rect_bounds[3].y,
                    outr, outg, outb, outa);

  draw_triangle_rgb(renderer,
                    rect_bounds[1].x, rect_bounds[1].y,
                    (float)width, rect_bounds[1].y,
                    (float)width, rect_bounds[2].y,
                    outr, outg, outb, outa);
  draw_triangle_rgb(renderer,
                    rect_bounds[1].x, rect_bounds[1].y,
                    rect_bounds[2].x, rect_bounds[2].y,
                    (float)width, rect_bounds[2].y,
                    outr, outg, outb, outa);
}

SDL_FPoint SDL_PointMid(float x1, float y1, float x2, float y2) {
  SDL_FPoint f;
  f.x = (x1 + x2) * 0.5f;
  f.y = (y1 + y2) * 0.5f;
  return f;
}
