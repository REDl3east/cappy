#include <bitset>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <memory>
#include <numbers>
#include <sstream>
#include <vector>

#include "SDL3/SDL.h"

#include "machine.h"

std::shared_ptr<SDL_Texture> create_capture_texture(std::shared_ptr<SDL_Renderer> renderer, Capture& capture);

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

  Camera camera;
  auto machine = std::make_shared<CappyMachine>(renderer, capture, texture, camera);
  machine->set_state<MoveState>();

  std::shared_ptr<SDL_Cursor> move_cursor = std::shared_ptr<SDL_Cursor>(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL), SDL_DestroyCursor);
  SDL_Cursor* default_cursor              = SDL_GetDefaultCursor();

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
          } else if (event.button.button == SDL_BUTTON_RIGHT) {
            machine->set_state<DrawCropState>(mx, my);
            continue;
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
          machine->zoom(event.wheel.y > 0, mx, my);
          break;
        }
      }
    }

    machine->draw_frame(machine);
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
