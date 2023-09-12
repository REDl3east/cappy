#include <bitset>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numbers>
#include <sstream>
#include <vector>

#include "SDL3/SDL.h"
#include "SDL3_ttf/SDL_ttf.h"
#include "advanced_pixel_7.h"
#include "camera.h"
#include "capture.h"
#include "machine.h"

std::shared_ptr<SDL_Texture> create_capture_texture(std::shared_ptr<SDL_Renderer> renderer, Capture& capture);

SDL_FPoint SDL_PointMid(float x1, float y1, float x2, float y2);
SDL_FPoint SDL_PointMid(const SDL_FPoint& p1, const SDL_FPoint& p2);

void drawTriangle(std::shared_ptr<SDL_Renderer> renderer, float x1, float y1, float x2, float y2, float x3, float y3, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void drawTriangle(std::shared_ptr<SDL_Renderer> renderer, float x1, float y1, float x2, float y2, float x3, float y3, Uint8 r1, Uint8 g1, Uint8 b1, Uint8 a1, Uint8 r2, Uint8 g2, Uint8 b2, Uint8 a2, Uint8 r3, Uint8 g3, Uint8 b3, Uint8 a3);
void draw_circle_flashlight(std::shared_ptr<SDL_Renderer> renderer, float x, float y, float radius, int edges, uint8_t cr, uint8_t cg, uint8_t cb, uint8_t ca, uint8_t cor, uint8_t cog, uint8_t cob, uint8_t coa, uint8_t otr, uint8_t otg, uint8_t otb, uint8_t ota);
void draw_rect_flashlight(std::shared_ptr<SDL_Renderer> renderer, float x, float y, float w, float h, uint8_t inr, uint8_t ing, uint8_t inb, uint8_t ina, uint8_t outr, uint8_t outg, uint8_t outb, uint8_t outa);

int main() {
  Capture capture;
  if (!capture.capture()) {
    std::cerr << "Failed to capture screen!\n";
    return 1;
  }

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "Failed to init SDL!\n";
    return 1;
  }

  std::shared_ptr<SDL_Window> window = std::shared_ptr<SDL_Window>(SDL_CreateWindow("Cappy", capture.width, capture.height, SDL_WINDOW_BORDERLESS), SDL_DestroyWindow);
  if (!window) {
    std::cerr << "Failed to create window!\n";
    return 1;
  }

  SDL_SetWindowPosition(window.get(), 0, 0);

  std::shared_ptr<SDL_Renderer> renderer = std::shared_ptr<SDL_Renderer>(SDL_CreateRenderer(window.get(), NULL, SDL_RENDERER_PRESENTVSYNC), SDL_DestroyRenderer);
  if (!renderer) {
    std::cerr << "Failed to create renderer!\n";
    return 1;
  }

  SDL_SetRenderDrawBlendMode(renderer.get(), SDL_BLENDMODE_BLEND);

  std::shared_ptr<SDL_Texture> texture = create_capture_texture(renderer, capture);

  if (!texture) {
    std::cerr << "Failed to create capture texture!\n";
    return 1;
  }

  if (TTF_Init() < 0) {
    std::cerr << "Failed to init TTF!\n";
    return 1;
  }

  auto machine = std::make_shared<CappyMachine>(capture, texture);
  machine->set_state<MoveState>();

  std::shared_ptr<SDL_Cursor> move_cursor = std::shared_ptr<SDL_Cursor>(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL), SDL_DestroyCursor);
  SDL_Cursor* default_cursor              = SDL_GetDefaultCursor();

  Camera camera;
  bool quit             = false;
  bool show_color       = false;
  bool show_flashlight  = false;
  float flashlight_size = 300.0f;
  while (!quit) {
    SDL_Event event;

    float mx, my;
    SDL_GetMouseState(&mx, &my);
    SDL_FPoint mouse = camera.screen_to_world(mx, my);
    while (SDL_PollEvent(&event)) {
      bool handled = machine->handle_event(event);
      if (handled) continue;

      switch (event.type) {
        case SDL_EVENT_QUIT: {
          quit = true;
          break;
        }
        case SDL_EVENT_KEY_DOWN: {
          SDL_Keycode code = event.key.keysym.sym;
          SDL_Keymod mod   = SDL_GetModState();
          if (code == SDLK_q) {
            quit = true;
          } else if (code == SDLK_f) {
            machine->set_state<FlashlightState>();
            continue;
          } else if (code == SDLK_c) {
            machine->set_state<ColorState>();
            continue;
          } else if (code == SDLK_r) {
            camera.reset();
          }

          // } else {
          //   RGB rgb;
          //   if (!capture.at(mouse.x, mouse.y, rgb)) break;
          //   if (code == SDLK_d && mod & SDL_KMOD_CTRL) {
          //     if (mod & SDL_KMOD_SHIFT) {
          //       SDL_SetClipboardText(toDecimalSepString(rgb).c_str());
          //     } else {
          //       SDL_SetClipboardText(toDecimalString(rgb).c_str());
          //     }
          //   } else if (code == SDLK_h && mod & SDL_KMOD_CTRL) {
          //     if (mod & SDL_KMOD_SHIFT) {
          //       SDL_SetClipboardText(toHexSepString(rgb).c_str());
          //     } else {
          //       SDL_SetClipboardText(toHexString(rgb).c_str());
          //     }
          //   } else if (code == SDLK_b && mod & SDL_KMOD_CTRL) {
          //     if (mod & SDL_KMOD_SHIFT) {
          //       SDL_SetClipboardText(toBinarySepString(rgb).c_str());
          //     } else {
          //       SDL_SetClipboardText(toBinaryString(rgb).c_str());
          //     }
          //   }
          // }

          break;
        }
        case SDL_EVENT_KEY_UP: {
          // SDL_Keycode code = event.key.keysym.sym;
          // if (code == SDLK_c) {
          // }

          break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
          if (event.button.button == SDL_BUTTON_LEFT) {
            SDL_SetCursor(move_cursor.get());
          }
          break;
        }
        case SDL_EVENT_MOUSE_BUTTON_UP: {
          if (event.button.button == SDL_BUTTON_LEFT) {
            SDL_SetCursor(default_cursor);
          }
          break;
        }
        case SDL_EVENT_MOUSE_MOTION: {
          if ((event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT))) {
            camera.pan(event.motion.xrel, event.motion.yrel);
          }

          break;
        }

        case SDL_EVENT_MOUSE_WHEEL: {
          float scale = camera.get_scale();

          if (event.wheel.y > 0) {
            if (scale <= 100.0f) {
              camera.zoom(0.1, mx, my);
            }
          } else if (event.wheel.y < 0) {
            if (scale >= 0.25) {
              camera.zoom(-0.1, mx, my);
            }
          }

          break;
        }
      }
    }

    machine->draw_frame(machine, renderer, camera);
  }

  TTF_CloseFont(machine->get_font());
  TTF_Quit();
  SDL_Quit();

  return 0;
}

std::shared_ptr<SDL_Texture> create_capture_texture(std::shared_ptr<SDL_Renderer> renderer, Capture& capture) {
  std::shared_ptr<SDL_Surface> surface = std::shared_ptr<SDL_Surface>(SDL_CreateSurfaceFrom(capture.pixels, capture.width, capture.height, capture.stride * 3, SDL_PIXELFORMAT_RGB24), SDL_DestroySurface);
  return std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(renderer.get(), surface.get()), SDL_DestroyTexture);
}

SDL_FPoint SDL_PointMid(const SDL_FPoint& p1, const SDL_FPoint& p2) {
  return SDL_PointMid(p1.x, p1.y, p2.x, p2.y);
}

void drawTriangle(std::shared_ptr<SDL_Renderer> renderer, float x1, float y1, float x2, float y2, float x3, float y3, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
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
                  Uint8 r1, Uint8 g1, Uint8 b1, Uint8 a1,
                  Uint8 r2, Uint8 g2, Uint8 b2, Uint8 a2,
                  Uint8 r3, Uint8 g3, Uint8 b3, Uint8 a3) {
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
                            uint8_t cr, uint8_t cg, uint8_t cb, uint8_t ca,
                            uint8_t cor, uint8_t cog, uint8_t cob, uint8_t coa,
                            uint8_t otr, uint8_t otg, uint8_t otb, uint8_t ota) {
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

  std::vector<SDL_Vertex> v;

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

void draw_rect_flashlight(std::shared_ptr<SDL_Renderer> renderer, float x, float y, float w, float h, uint8_t inr, uint8_t ing, uint8_t inb, uint8_t ina, uint8_t outr, uint8_t outg, uint8_t outb, uint8_t outa) {
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