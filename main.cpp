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
#include "camera.h"
#include "capture.h"

std::shared_ptr<SDL_Texture> create_capture_texture(std::shared_ptr<SDL_Renderer> renderer, Capture& capture);

std::string toDecimalString(const RGB& color);
std::string toDecimalSepString(const RGB& color);
std::string toHexString(const RGB& color);
std::string toHexSepString(const RGB& color);
std::string toBinaryString(const RGB& color);
std::string toBinarySepString(const RGB& color);

SDL_FPoint SDL_PointMid(float x1, float y1, float x2, float y2);
SDL_FPoint SDL_PointMid(const SDL_FPoint& p1, const SDL_FPoint& p2);

void drawTriangle(std::shared_ptr<SDL_Renderer> renderer, float x1, float y1, float x2, float y2, float x3, float y3, Uint8 r, Uint8 g, Uint8 b, Uint8 a);
void drawTriangle(std::shared_ptr<SDL_Renderer> renderer, float x1, float y1, float x2, float y2, float x3, float y3, Uint8 r1, Uint8 g1, Uint8 b1, Uint8 a1, Uint8 r2, Uint8 g2, Uint8 b2, Uint8 a2, Uint8 r3, Uint8 g3, Uint8 b3, Uint8 a3);
void draw_flashlight(std::shared_ptr<SDL_Renderer> renderer, float x, float y, float radius, int edges, uint8_t cr, uint8_t cg, uint8_t cb, uint8_t ca, uint8_t cor, uint8_t cog, uint8_t cob, uint8_t coa, uint8_t otr, uint8_t otg, uint8_t otb, uint8_t ota);

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

  TTF_Font* font = TTF_OpenFont("/home/dalton/projects/cappy/fonts/advanced_pixel-7.ttf", 36);
  if (!font) {
    std::cerr << "Failed to load font: " << TTF_GetError() << '\n';
    return 1;
  }

  // Create a surface for rendering text
  std::shared_ptr<SDL_Surface> text_surface;
  std::shared_ptr<SDL_Texture> text_texture;

  auto recompute_text = [&text_surface, &text_texture, &font, &renderer](const char* text) -> bool {
    text_surface = std::shared_ptr<SDL_Surface>(TTF_RenderText_Solid(font, text, {255, 255, 255, 255}), SDL_DestroySurface);
    if (!text_surface) return false;

    text_texture = std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(renderer.get(), text_surface.get()), SDL_DestroyTexture);
    if (!text_texture) return false;

    return true;
  };

  if (!recompute_text("hello")) {
    std::cerr << "Failed to recompute text\n"
              << '\n';
    return 1;
  }

  Camera camera;
  bool quit             = false;
  bool show_color       = false;
  bool show_flashlight  = false;
  float flashlight_size = 100.0f;
  while (!quit) {
    std::shared_ptr<SDL_Cursor> move_cursor = std::shared_ptr<SDL_Cursor>(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL), SDL_DestroyCursor);

    SDL_Event event;

    float mx, my;
    SDL_GetMouseState(&mx, &my);
    SDL_FPoint mouse = camera.screen_to_world(mx, my);
    while (SDL_PollEvent(&event)) {
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
          } else if (code == SDLK_r) {
            camera.reset();
          } else if (code == SDLK_c) {
            show_color      = !show_color;
            show_flashlight = false;
            if (show_color) {
              RGB rgb;
              if (!capture.at(mouse.x, mouse.y, rgb)) break;
              recompute_text(toDecimalSepString(rgb).c_str());
            }
            SDL_ShowCursor();
          } else if (code == SDLK_f) {
            show_flashlight = !show_flashlight;
            show_color      = false;
            if (show_flashlight)
              SDL_HideCursor();
            else
              SDL_ShowCursor();
          } else {
            RGB rgb;
            if (!capture.at(mouse.x, mouse.y, rgb)) break;
            if (code == SDLK_d && mod & SDL_KMOD_CTRL) {
              if (mod & SDL_KMOD_SHIFT) {
                SDL_SetClipboardText(toDecimalSepString(rgb).c_str());
              } else {
                SDL_SetClipboardText(toDecimalString(rgb).c_str());
              }
            } else if (code == SDLK_h && mod & SDL_KMOD_CTRL) {
              if (mod & SDL_KMOD_SHIFT) {
                SDL_SetClipboardText(toHexSepString(rgb).c_str());
              } else {
                SDL_SetClipboardText(toHexString(rgb).c_str());
              }
            } else if (code == SDLK_b && mod & SDL_KMOD_CTRL) {
              if (mod & SDL_KMOD_SHIFT) {
                SDL_SetClipboardText(toBinarySepString(rgb).c_str());
              } else {
                SDL_SetClipboardText(toBinaryString(rgb).c_str());
              }
            }
          }

          break;
        }
        case SDL_EVENT_KEY_UP: {
          SDL_Keycode code = event.key.keysym.sym;
          if (code == SDLK_c) {
          }

          break;
        }
        case SDL_EVENT_MOUSE_MOTION: {
          if ((event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT))) {
            camera.pan(event.motion.xrel, event.motion.yrel);
          }

          RGB rgb;
          if (!capture.at(mouse.x, mouse.y, rgb)) break;
          recompute_text(toDecimalSepString(rgb).c_str());
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
            SDL_SetCursor(SDL_GetDefaultCursor());
          }
          break;
        }
        case SDL_EVENT_MOUSE_WHEEL: {
          float scale = camera.get_scale();

          if (show_flashlight && (SDL_GetModState() & SDL_KMOD_LSHIFT)) {
            flashlight_size += event.wheel.y > 0 ? -10.0f : 10.f;
            if (flashlight_size <= 0) flashlight_size = 0.0f;
          } else {
            if (event.wheel.y > 0) {
              if (scale <= 100.0f) {
                camera.zoom(0.1, mx, my);
              }
            } else if (event.wheel.y < 0) {
              if (scale >= 0.25) {
                camera.zoom(-0.1, mx, my);
              }
            }
          }
          break;
        }
      }
    }

    SDL_SetRenderDrawColor(renderer.get(), 125, 125, 125, 255);
    SDL_RenderClear(renderer.get());

    SDL_FPoint pos = camera.world_to_screen(0, 0);
    SDL_FRect r    = {pos.x, pos.y, (float)capture.width * camera.get_scale(), (float)capture.height * camera.get_scale()};
    SDL_RenderTexture(renderer.get(), texture.get(), NULL, &r);

    if (show_color) {
      RGB rgb;
      if (capture.at(mouse.x, mouse.y, rgb)) {
        float size     = 200.0f;
        float offset   = 50.0f;
        SDL_FRect rect = {mx + offset, my - size - offset, size, size};

        SDL_SetRenderDrawColor(renderer.get(), rgb.r, rgb.g, rgb.b, 255);
        SDL_RenderFillRect(renderer.get(), &rect);

        SDL_SetRenderDrawColor(renderer.get(), 200, 200, 200, 255);
        SDL_RenderRect(renderer.get(), &rect);

        SDL_FRect text_rect = {mx + offset + (0.5f * (size - text_surface->w)), my - size - offset - text_surface->h, (float)text_surface->w, (float)text_surface->h};

        SDL_FRect text_rect_back = {mx + offset, my - size - offset - text_surface->h, size, (float)text_surface->h};
        SDL_SetRenderDrawColor(renderer.get(), 0, 0, 0, 255);
        SDL_RenderFillRect(renderer.get(), &text_rect_back);

        SDL_SetRenderDrawColor(renderer.get(), 200, 200, 200, 255);
        SDL_RenderRect(renderer.get(), &text_rect_back);

        SDL_RenderTexture(renderer.get(), text_texture.get(), NULL, &text_rect);
      }
    } else if (show_flashlight) {
      float x, y;
      SDL_GetMouseState(&x, &y);
      draw_flashlight(renderer, x, y, flashlight_size, 100,
                      255, 255, 255, 0,
                      255, 255, 255, 0,
                      0, 0, 0, 200);
    }

    SDL_RenderPresent(renderer.get());
  }

  TTF_CloseFont(font);
  TTF_Quit();
  SDL_Quit();

  return 0;
}

std::shared_ptr<SDL_Texture> create_capture_texture(std::shared_ptr<SDL_Renderer> renderer, Capture& capture) {
  std::shared_ptr<SDL_Surface> surface = std::shared_ptr<SDL_Surface>(SDL_CreateSurfaceFrom(capture.pixels, capture.width, capture.height, capture.stride * 3, SDL_PIXELFORMAT_RGB24), SDL_DestroySurface);
  return std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(renderer.get(), surface.get()), SDL_DestroyTexture);
}

std::string toDecimalString(const RGB& color) {
  int x = (color.r << 16) | (color.g << 8) | color.b;
  std::stringstream stream;
  stream << x;
  return stream.str();
}

std::string toDecimalSepString(const RGB& color) {
  std::stringstream stream;
  stream << static_cast<int>(color.r) << ", " << static_cast<int>(color.g) << ", " << static_cast<int>(color.b);
  return stream.str();
}

std::string toHexString(const RGB& color) {
  std::stringstream stream;
  stream << "0x" << std::setfill('0') << std::setw(2) << std::hex;
  stream << static_cast<int>(color.r) << std::setw(2) << static_cast<int>(color.g) << std::setw(2) << static_cast<int>(color.b);
  return stream.str();
}

std::string toHexSepString(const RGB& color) {
  std::stringstream stream;
  stream << "0x" << std::setfill('0') << std::setw(2) << std::hex;
  stream << static_cast<int>(color.r) << std::setw(2) << ", ";
  stream << "0x" << std::setfill('0') << std::setw(2) << std::hex;
  stream << static_cast<int>(color.g) << std::setw(2) << ", ";
  stream << "0x" << std::setfill('0') << std::setw(2) << std::hex;
  stream << static_cast<int>(color.b) << std::setw(2);
  return stream.str();
}

std::string toBinaryString(const RGB& color) {
  return "0b" + std::bitset<8>(color.r).to_string() + std::bitset<8>(color.g).to_string() + std::bitset<8>(color.b).to_string();
}

std::string toBinarySepString(const RGB& color) {
  return "0b" + std::bitset<8>(color.r).to_string() + ", 0b" + std::bitset<8>(color.g).to_string() + ", 0b" + std::bitset<8>(color.b).to_string();
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

void draw_flashlight(std::shared_ptr<SDL_Renderer> renderer, float x, float y, float radius, int edges,
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
