#include <bitset>
#include <iomanip>
#include <iostream>
#include <memory>
#include <sstream>

#include "SDL3/SDL.h"
#include "camera.h"
#include "capture.h"

std::shared_ptr<SDL_Texture> create_capture_texture(std::shared_ptr<SDL_Renderer> renderer, Capture& capture);

std::string toDecimalString(const RGB& color);
std::string toDecimalSepString(const RGB& color);
std::string toHexString(const RGB& color);
std::string toHexSepString(const RGB& color);
std::string toBinaryString(const RGB& color);
std::string toBinarySepString(const RGB& color);

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

  std::shared_ptr<SDL_Texture> texture = create_capture_texture(renderer, capture);

  if (!texture) {
    std::cerr << "Failed to create capture texture!\n";
    return 1;
  }

  Camera camera;

  SDL_Event event;
  bool quit       = false;
  bool show_color = false;

  while (!quit) {
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
            show_color = !show_color;
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
          break;
        }
        case SDL_EVENT_MOUSE_WHEEL: {
          float scale = camera.get_scale();

          if (event.wheel.y > 0) {
            if (scale <= 25.0f) {
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

    SDL_SetRenderDrawColor(renderer.get(), 211, 211, 211, 255);
    SDL_RenderClear(renderer.get());

    SDL_FPoint pos = camera.world_to_screen(0, 0);
    SDL_FRect r    = {pos.x, pos.y, (float)capture.width * camera.get_scale(), (float)capture.height * camera.get_scale()};
    SDL_RenderTexture(renderer.get(), texture.get(), NULL, &r);

    if (show_color) {
      RGB rgb;
      if (capture.at(mouse.x, mouse.y, rgb)) {
        float size     = 100.0f;
        float offset   = 50.0f;
        SDL_FRect rect = {mx + offset, my - size - offset, size, size};

        SDL_SetRenderDrawColor(renderer.get(), rgb.r, rgb.g, rgb.b, 255);
        SDL_RenderFillRect(renderer.get(), &rect);

        SDL_SetRenderDrawColor(renderer.get(), 255, 0, 0, 255);
        SDL_RenderRect(renderer.get(), &rect);
      }
    }

    SDL_RenderPresent(renderer.get());
  }

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