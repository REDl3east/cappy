#include <cmath>
#include <future>
#include <iostream>
#include <thread>

#include "SDL3/SDL.h"
#include "stb_image_write.h"

#include "cappyMachine.h"
#include "colorState.h"
#include "config.h"
#include "drawCropState.h"
#include "flashlightState.h"
#include "icon.h"
#include "moveState.h"

#define SAVE_FILE_EVENT (SDL_EVENT_USER + 1)

std::shared_ptr<SDL_Texture> create_capture_texture(std::shared_ptr<SDL_Renderer> renderer, Capture& capture);

int main(int argc, char** argv) {
  Uint32 flags = 0;
  Capture capture;

  if (!capture.capture()) {
    SDL_Log("Failed to capture screen!");
    return 1;
  }

  cappyConfig config;
  config_init(config);

  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_Log("Failed to init SDL!");
    return 1;
  }

  SDL_PropertiesID props = SDL_CreateProperties();
  SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "Cappy");
  SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, capture.width);
  SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, capture.height);
  SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, 0);
  SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, 0);

  if (config.window_fullscreen) {
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, 1);
  } else {
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, 1);
  }

  std::shared_ptr<SDL_Window> window = std::shared_ptr<SDL_Window>(SDL_CreateWindowWithProperties(props), SDL_DestroyWindow);
  if (!window) {
    SDL_Log("Failed to create window!");
    SDL_DestroyProperties(props);
    return 1;
  }
  SDL_DestroyProperties(props);

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

  TTF_Font* font = TTF_OpenFontIO(SDL_IOFromConstMem(advanced_pixel_7, sizeof(advanced_pixel_7)), SDL_TRUE, 36);
  if (!font) {
    SDL_Log("Failed to load font: %s", TTF_GetError());
    return 1;
  }

  std::shared_ptr<SDL_Surface> icon = std::shared_ptr<SDL_Surface>(SDL_CreateSurfaceFrom(icon_data, ICON_WIDTH, ICON_HEIGHT, ICON_WIDTH * 4, SDL_PIXELFORMAT_RGBA32), SDL_DestroySurface);
  SDL_SetWindowIcon(window.get(), icon.get());

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
          } else if (code == SDLK_g) {
            machine->toggle_grid();
          } else if (code == SDLK_r) {
            // camera.reset();
            machine->current_x = 0;
            machine->current_y = 0;
            machine->current_w = capture.width;
            machine->current_h = capture.height;
            machine->set_state<MoveState>();
            continue;
          } else if (code == SDLK_m) {
            SDL_MinimizeWindow(window.get());
          } else if (code == SDLK_s && mod & SDL_KMOD_CTRL) {
            static const SDL_DialogFileFilter filters[] = {
                {"PNG images", "png"},
                {NULL, NULL},
            };

            SDL_ShowSaveFileDialog([](void* userdata, const char* const* filelist, int filter) {
              if (filelist) {
                if (!*filelist) {
                  SDL_Log("Save dialog canceled.");
                  return;
                }

                SDL_Event event;
                SDL_memset(&event, 0, sizeof(event));
                event.type       = SAVE_FILE_EVENT;
                event.user.data1 = strdup(*filelist);
                SDL_PushEvent(&event);

              } else {
                SDL_Log("Error: %s\n", SDL_GetError());
              }
            },
                                   machine.get(), window.get(), filters, NULL);
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

        case SAVE_FILE_EVENT: {
          std::shared_ptr<SDL_Cursor> wait_cursor = std::shared_ptr<SDL_Cursor>(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT), SDL_DestroyCursor);
          SDL_SetCursor(wait_cursor.get());

          std::string path = std::string((char*)(event.user.data1));
          free(event.user.data1);

          constexpr int comp = 3;
          int stride         = machine->get_capture().stride;
          int index          = machine->current_y * stride + machine->current_x;
          RGB* pixels        = &machine->get_capture().pixels[index];

          if (!path.ends_with(".png")) {
            path += ".png";
          }
          if (stbi_write_png(path.c_str(), machine->current_w, machine->current_h, comp, pixels, comp * stride) == 0) {
            SDL_Log("Failed to save file: '%s'", path.c_str());
          } else {
            SDL_Log("Saved file: '%s'", path.c_str());
          }

          SDL_SetCursor(SDL_GetDefaultCursor());

          break;
        }
      }
    }

    machine->render_clear(config.background_color[0], config.background_color[1], config.background_color[2]);
    machine->render_capture();
    machine->render_grid(config.grid_size, config.grid_color[0], config.grid_color[1], config.grid_color[2]);
    machine->draw_frame(machine);

    machine->render_present();
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
