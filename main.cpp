#include <iostream>
#include <memory>

#include "SDL3/SDL.h"
#include "camera.h"
#include "capture.h"

std::shared_ptr<SDL_Texture> create_capture_texture(std::shared_ptr<SDL_Renderer> renderer, Capture& capture) {
  std::shared_ptr<SDL_Surface> surface = std::shared_ptr<SDL_Surface>(SDL_CreateSurfaceFrom(capture.pixels, capture.width, capture.height, capture.stride * 3, SDL_PIXELFORMAT_RGB24), SDL_DestroySurface);
  return std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(renderer.get(), surface.get()), SDL_DestroyTexture);
}

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
  bool quit = false;

  while (!quit) {
    while (SDL_PollEvent(&event)) {
      switch (event.type) {
        case SDL_EVENT_QUIT: {
          quit = true;
          break;
        }
        case SDL_EVENT_KEY_DOWN: {
          SDL_Keycode code = event.key.keysym.sym;
          if (code == SDLK_q) {
            quit = true;
            break;
          } else if (code == SDLK_r) {
            camera.reset();
            break;
          }
        }
        case SDL_EVENT_MOUSE_MOTION: {
          if ((event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT))) {
            camera.pan(event.motion.xrel, event.motion.yrel);
          }
          break;
        }
        case SDL_EVENT_MOUSE_WHEEL: {
          float scale = camera.get_scale();
          float mx, my;
          SDL_GetMouseState(&mx, &my);

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

    float mx, my;
    SDL_GetMouseState(&mx, &my);
    SDL_FPoint mouse = camera.screen_to_world(mx, my);

    SDL_SetRenderDrawColor(renderer.get(), 211, 211, 211, 255);
    SDL_RenderClear(renderer.get());

    SDL_FPoint pos = camera.world_to_screen(0, 0);
    SDL_FRect r    = {pos.x, pos.y, (float)capture.width * camera.get_scale(), (float)capture.height * camera.get_scale()};
    SDL_RenderTexture(renderer.get(), texture.get(), NULL, &r);

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

    SDL_RenderPresent(renderer.get());
  }

  SDL_Quit();

  return 0;
}
