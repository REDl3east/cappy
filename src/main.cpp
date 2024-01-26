#include <cmath>
#include <future>
#include <iostream>
#include <thread>

#include "SDL3/SDL.h"
#include "portable-file-dialogs.h"
#include "stb_image_write.h"

#include "cappyMachine.h"
#include "colorState.h"
#include "config.h"
#include "drawCropState.h"
#include "flashlightState.h"
#include "moveState.h"

std::shared_ptr<SDL_Texture> create_capture_texture(std::shared_ptr<SDL_Renderer> renderer, Capture& capture);

int main(int argc, char** argv) {
  Uint32 flags = 0;
  int x        = 0;
  int y        = 0;
  Capture capture;

  if (!capture.capture()) {
    SDL_Log("Failed to capture screen!");
    return 1;
  }

  cappyConfig config;
  config_init(config);

  flags |= SDL_WINDOW_BORDERLESS;
  if (config.window_fullscreen) flags |= SDL_WINDOW_FULLSCREEN;

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_Log("Failed to init SDL!");
    return 1;
  }

  std::shared_ptr<SDL_Window> window = std::shared_ptr<SDL_Window>(SDL_CreateWindowWithPosition("Cappy", x, y, capture.width, capture.height, flags), SDL_DestroyWindow);
  if (!window) {
    SDL_Log("Failed to create window!");
    return 1;
  }

  std::shared_ptr<SDL_Renderer> renderer = std::shared_ptr<SDL_Renderer>(SDL_CreateRenderer(window.get(), NULL, SDL_RENDERER_PRESENTVSYNC), SDL_DestroyRenderer);
  if (!renderer) {
    SDL_Log("Failed to create renderer!");
    return 1;
  }

  SDL_SetRenderDrawBlendMode(renderer.get(), SDL_BLENDMODE_BLEND);

  std::shared_ptr<SDL_Texture> texture = create_capture_texture(renderer, capture);

  if (!texture) {
    SDL_Log("Failed to create capture texture!");
    return 1;
  }

  if (TTF_Init() < 0) {
    SDL_Log("Failed to init TTF!");
    return 1;
  }

  TTF_Font* font = TTF_OpenFontRW(SDL_RWFromConstMem(advanced_pixel_7, sizeof(advanced_pixel_7)), SDL_TRUE, 36);
  if (!font) {
    SDL_Log("Failed to load font: %s", TTF_GetError());
    return 1;
  }

  CameraSmooth camera;
  auto machine = CappyMachine::make(config, renderer, capture, texture, camera, font);
  machine->set_state<MoveState>();

  // setting bounds in capture, if width or height is <= 0
  // then it sets the crop to the capture width or height.
  if (config.window_pre_crop[0] < 0) config.window_pre_crop[0] = 0;
  if (config.window_pre_crop[1] < 0) config.window_pre_crop[1] = 0;
  if (config.window_pre_crop[2] <= 0) config.window_pre_crop[2] = capture.width;
  if (config.window_pre_crop[3] <= 0) config.window_pre_crop[3] = capture.height;

  if (config.window_pre_crop[0] > capture.width) config.window_pre_crop[0] = capture.width;
  if (config.window_pre_crop[1] > capture.height) config.window_pre_crop[1] = capture.height;
  if (config.window_pre_crop[2] > capture.width) config.window_pre_crop[2] = capture.width;
  if (config.window_pre_crop[3] > capture.height) config.window_pre_crop[3] = capture.height;

  // set x and y to top left most point
  // and calculate width and height
  machine->current_x = std::min(config.window_pre_crop[0], config.window_pre_crop[2]);
  machine->current_y = std::min(config.window_pre_crop[1], config.window_pre_crop[3]);
  machine->current_w = std::abs(config.window_pre_crop[2] - config.window_pre_crop[0]);
  machine->current_h = std::abs(config.window_pre_crop[3] - config.window_pre_crop[1]);

  std::shared_ptr<SDL_Cursor> move_cursor = std::shared_ptr<SDL_Cursor>(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEALL), SDL_DestroyCursor);
  SDL_Cursor* default_cursor              = SDL_GetDefaultCursor();

  auto save_capture_from_dialog = [&machine](std::string path) {
    if (path.empty()) return 0;

    std::thread([machine, path]() mutable {
      constexpr int comp = 3;
      int stride         = machine->get_capture().stride;
      int index          = machine->current_y * stride + machine->current_x;
      RGB* pixels        = &machine->get_capture().pixels[index];

      if (!path.ends_with(".png")) {
        path += ".png";
      }
      if (stbi_write_png(path.c_str(), machine->current_w, machine->current_h, comp, pixels, comp * stride) == 0) {
        SDL_Log("Failed to save file: '%s'", path.c_str());
        return;
      }
      SDL_Log("saved file: '%s'", path.c_str());
    }).detach();

    return 1;
  };

  std::unique_ptr<pfd::save_file> save_dialog;

  float last_x = 0.0f;
  float last_y = 0.0f;

  bool quit = false;
  while (!quit) {
    SDL_Event event;

    float mx, my;
    SDL_GetMouseState(&mx, &my);
    last_x = mx;
    last_y = my;
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
            // camera.reset();
            machine->current_x = 0;
            machine->current_y = 0;
            machine->current_w = capture.width;
            machine->current_h = capture.height;
            machine->set_state<MoveState>();
            continue;
          } else if (code == SDLK_s && mod & SDL_KMOD_CTRL) {
            if (!save_dialog) save_dialog.reset(new pfd::save_file("Select a file", ".", {"Image Files (.png)", "*.png"}, pfd::opt::none));
          }

          break;
        }
        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
          if (event.button.button == SDL_BUTTON_LEFT) {
            camera.cancel_pan();

            SDL_SetCursor(move_cursor.get());
          } else if (event.button.button == SDL_BUTTON_RIGHT) {
            float mx, my;
            SDL_GetMouseState(&mx, &my);
            machine->set_state<DrawCropState>(mx, my);
            continue;
          }
          break;
        }
        case SDL_EVENT_MOUSE_BUTTON_UP: {
          if (event.button.button == SDL_BUTTON_LEFT) {
            float mx, my;
            SDL_GetMouseState(&mx, &my);

            float magnitude = std::sqrt(mx * mx + my * my);
            float nx        = (mx - last_x) / magnitude;
            float ny        = (my - last_y) / magnitude;
            float vx        = 1000.0f * nx;
            float vy        = 1000.0f * ny;
            camera.smooth_pan(vx, vy, 0.92, 10);

            if (!(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT))) {
              SDL_SetCursor(default_cursor);
            }
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
          float mx, my;
          SDL_GetMouseState(&mx, &my);
          machine->zoom(event.wheel.y > 0, mx, my);
          break;
        }
      }
    }

    if (save_dialog) {
      if (save_dialog.get()->ready()) {
        save_capture_from_dialog(save_dialog.get()->result());
        save_dialog.reset();
      }
    }

    machine->render_clear(config.background_color[0], config.background_color[1], config.background_color[2]);
    machine->draw_frame(machine);
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
