#include "app.h"

#include "assets/advanced_pixel_7.h"
#include "assets/icon.h"
#include "renderer.h"

#define SDL_MAIN_USE_CALLBACKS
#include "SDL3/SDL_main.h"

#include <math.h>
#include <stdio.h>

SDL_Texture* create_capture_texture(SDL_Renderer* renderer, capture_t* capture);
void render_grid(app_t* app, int grid_size, uint8_t r, uint8_t g, uint8_t b);

SDL_AppResult app_init(app_t* app, int argc, char** argv) {
  if (!capture_screen(&app->capture)) {
    SDL_Log("Failed to capture screen!");
    return SDL_APP_FAILURE;
  }

  if (!config_init(&app->config)) {
    SDL_Log("Failed to initialize config!");
    return SDL_APP_FAILURE;
  }
  config_print(&app->config);

  if (!SDL_Init(SDL_INIT_VIDEO)) {
    SDL_Log("Failed to init SDL: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_PropertiesID props = SDL_CreateProperties();
  SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "Cappy");
  SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, 1920);
  SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, 1080);
  // SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, app->capture.width);
  // SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, app->capture.height);
  SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, 0);
  SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, 0);

  if (app->config.window_fullscreen) {
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, 1);
  } else {
    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, 1);
  }

  app->window = SDL_CreateWindowWithProperties(props);
  SDL_DestroyProperties(props);

  if (app->window == NULL) {
    SDL_Log("Failed to create window: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  app->renderer = SDL_CreateRenderer(app->window, NULL);
  if (app->renderer == NULL) {
    SDL_Log("Failed to create renderer!");
    return SDL_APP_FAILURE;
  }

  SDL_SetRenderVSync(app->renderer, 1);
  SDL_SetRenderDrawBlendMode(app->renderer, SDL_BLENDMODE_BLEND);

  app->capture_texture = create_capture_texture(app->renderer, &app->capture);
  if (app->capture_texture == NULL) {
    SDL_Log("Failed to create capture texture: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_SetTextureScaleMode(app->capture_texture, SDL_SCALEMODE_NEAREST);

  if (!TTF_Init()) {
    SDL_Log("Failed to init TTF!: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  app->font = TTF_OpenFontIO(SDL_IOFromConstMem(advanced_pixel_7, sizeof(advanced_pixel_7)), true, 36);
  if (app->font == NULL) {
    SDL_Log("Failed to load font: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  app->icon = SDL_CreateSurfaceFrom(ICON_WIDTH, ICON_HEIGHT, SDL_PIXELFORMAT_RGBA32, icon_data, ICON_WIDTH * 4);
  if (app->icon == NULL) {
    SDL_Log("Failed to create icon: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  SDL_SetWindowIcon(app->window, app->icon);

  camera_reset(&app->camera);

  app->state          = APP_MOVE_STATE;
  app->recompute_text = true;

  app->flashlight_zoom.size             = 150.0f;
  app->flashlight_zoom.zooming          = false;
  app->flashlight_zoom.zoom_in          = false;
  app->flashlight_zoom.zoom_amount      = 150.0f;
  app->flashlight_zoom.zoom_ms          = 25.0f;
  app->flashlight_zoom.zoom_tick        = 0.0f;
  app->flashlight_zoom.zoom_elapsed     = 0.0f;
  app->flashlight_zoom.zoom_size_per_ms = 0.0f;

  // setting bounds in capture, if width or height is <= 0
  // then it sets the crop to the capture width or height.
  if (app->config.window_pre_crop[0] < 0) app->config.window_pre_crop[0] = 0;
  if (app->config.window_pre_crop[1] < 0) app->config.window_pre_crop[1] = 0;
  if (app->config.window_pre_crop[2] <= 0) app->config.window_pre_crop[2] = app->capture_texture->w;
  if (app->config.window_pre_crop[3] <= 0) app->config.window_pre_crop[3] = app->capture_texture->h;

  if (app->config.window_pre_crop[0] > app->capture_texture->w) app->config.window_pre_crop[0] = app->capture_texture->w;
  if (app->config.window_pre_crop[1] > app->capture_texture->h) app->config.window_pre_crop[1] = app->capture_texture->h;
  if (app->config.window_pre_crop[2] > app->capture_texture->w) app->config.window_pre_crop[2] = app->capture_texture->w;
  if (app->config.window_pre_crop[3] > app->capture_texture->h) app->config.window_pre_crop[3] = app->capture_texture->h;

  // set x and y to top left most point
  // and calculate width and height
  app->current_x = (int)MIN(app->config.window_pre_crop[0], app->config.window_pre_crop[2]);
  app->current_y = (int)MIN(app->config.window_pre_crop[1], app->config.window_pre_crop[3]);
  app->current_w = (int)abs(app->config.window_pre_crop[2] - app->config.window_pre_crop[0]);
  app->current_h = (int)abs(app->config.window_pre_crop[3] - app->config.window_pre_crop[1]);

  app->move_cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_MOVE);
  if (app->move_cursor == NULL) {
    SDL_Log("Failed to create move cursor: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  app->default_cursor = SDL_GetDefaultCursor();
  if (app->default_cursor == NULL) {
    SDL_Log("Failed to create default cursor: %s", SDL_GetError());
    return SDL_APP_FAILURE;
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult app_event(app_t* app, SDL_Event* event) {
  bool handled = false;
  if (app->state == APP_MOVE_STATE) {
    handled = app_event_move(app, event);
  } else if (app->state == APP_COLOR_STATE) {
    handled = app_event_color(app, event);
  } else if (app->state == APP_FLASHLIGHT_STATE) {
    handled = app_event_flashlight(app, event);
  } else {
    return SDL_APP_FAILURE;
  }

  if (handled) return SDL_APP_CONTINUE;

  switch (event->type) {
    case SDL_EVENT_QUIT: {
      return SDL_APP_SUCCESS;
    }
    case SDL_EVENT_KEY_DOWN: {
      SDL_Keycode code = event->key.key;
      SDL_Keymod mod   = SDL_GetModState();
      if (code == SDLK_Q) {
        return SDL_APP_SUCCESS;
      } else if (code == SDLK_F) {
        if (app->state == APP_MOVE_STATE || app->state == APP_COLOR_STATE) {
          app->state                      = APP_FLASHLIGHT_STATE;
        } else if (app->state == APP_FLASHLIGHT_STATE) {
          app->state = APP_MOVE_STATE;
        } else {
          return SDL_APP_FAILURE;
        }
      } else if (code == SDLK_C) {
        if (app->state == APP_MOVE_STATE || app->state == APP_FLASHLIGHT_STATE) {
          app->state          = APP_COLOR_STATE;
          app->recompute_text = true;
        } else if (app->state == APP_COLOR_STATE) {
          app->state = APP_MOVE_STATE;
        } else {
          return SDL_APP_FAILURE;
        }

      } else if (code == SDLK_G) {
        app->grid_enabled = !app->grid_enabled;
      } // else if (code == SDLK_r) {
      // TODO:

      //   // camera.reset();
      //   machine->current_x = 0;
      //   machine->current_y = 0;
      //   machine->current_w = capture.width;
      //   machine->current_h = capture.height;
      //   machine->set_state<MoveState>();
      //   continue;
      // }
      else if (code == SDLK_M) {
        SDL_MinimizeWindow(app->window);
      }
      // TODO:
      // else if (code == SDLK_s && mod & SDL_KMOD_CTRL) {
      //   static const SDL_DialogFileFilter filters[] = {
      //       {"PNG images", "png"},
      //       {NULL, NULL},
      //   };

      //   SDL_ShowSaveFileDialog([](void* userdata, const char* const* filelist, int filter) {
      //     if (filelist) {
      //       if (!*filelist) {
      //         SDL_Log("Save dialog canceled.");
      //         return;
      //       }

      //       SDL_Event event;
      //       SDL_memset(&event, 0, sizeof(event));
      //       event.type       = SAVE_FILE_EVENT;
      //       event.user.data1 = strdup(*filelist);
      //       SDL_PushEvent(&event);

      //     } else {
      //       SDL_Log("Error: %s\n", SDL_GetError());
      //     }
      //   },
      //                          machine.get(), window.get(), filters, NULL);
      // }

      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        app->camera.panning = false;

        SDL_SetCursor(app->move_cursor);
      } else if (event->button.button == SDL_BUTTON_RIGHT) {
        // TODO:
        // float mx, my;
        // SDL_GetMouseState(&mx, &my);
        // machine->set_state<DrawCropState>(mx, my);
        // continue;
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        float mx, my;
        SDL_GetMouseState(&mx, &my);

        float magnitude = sqrtf(mx * mx + my * my);
        float nx        = (mx - app->last_x) / magnitude;
        float ny        = (my - app->last_y) / magnitude;
        float vx        = 1000.0f * nx;
        float vy        = 1000.0f * ny;
        camera_smooth_pan(&app->camera, vx, vy, 0.92f, 10);

        if (!(SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT))) {
          SDL_SetCursor(app->default_cursor);
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      if ((event->motion.state & SDL_BUTTON_MASK(SDL_BUTTON_LEFT))) {
        camera_pan(&app->camera, event->motion.xrel, event->motion.yrel);
      }
      break;
    }

    case SDL_EVENT_MOUSE_WHEEL: {
      float mousex, mousey;
      SDL_GetMouseState(&mousex, &mousey);

      float zoom_in_factor  = 3.0f;
      Uint64 zoom_in_ms     = 150;
      float zoom_out_factor = 3.0f;
      Uint64 zoom_out_ms    = 100;

      float max_scale = 100.0f;
      float min_scale = 0.25f;

      float scale = app->camera.scale;
      if (event->wheel.y > 0) {
        if (scale <= max_scale) {
          camera_smooth_zoom(&app->camera, zoom_in_factor, mousex, mousey, zoom_in_ms);
        }
      } else {
        if (scale >= min_scale) {
          camera_smooth_zoom(&app->camera, -1.0f * zoom_out_factor, mousex, mousey, zoom_out_ms);
        }
      }
      break;
    }

      // TODO:
      // case SAVE_FILE_EVENT: {
      //   std::shared_ptr<SDL_Cursor> wait_cursor = std::shared_ptr<SDL_Cursor>(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_WAIT), SDL_DestroyCursor);
      //   SDL_SetCursor(wait_cursor.get());

      //   std::string path = std::string((char*)(event.user.data1));
      //   free(event.user.data1);

      //   constexpr int comp = 3;
      //   int stride         = machine->get_capture().stride;
      //   int index          = machine->current_y * stride + machine->current_x;
      //   RGB* pixels        = &machine->get_capture().pixels[index];

      //   if (path.starts_with("file://")) {
      //     path.erase(0, 7);
      //   } else if (path.starts_with("file:/")) {
      //     path.erase(0, 6);
      //   }

      //   if (!path.ends_with(".png")) {
      //     path += ".png";
      //   }

      //   if (stbi_write_png(path.c_str(), machine->current_w, machine->current_h, comp, pixels, comp * stride) == 0) {
      //     SDL_Log("Failed to save file: '%s': %s", path.c_str(), strerror(errno));
      //   } else {
      //     SDL_Log("Saved file: '%s'", path.c_str());
      //   }

      //   SDL_SetCursor(SDL_GetDefaultCursor());

      //   break;
      // }
  }

  return SDL_APP_CONTINUE;
}

SDL_AppResult app_iterate(app_t* app) {
  float mx, my;
  SDL_GetMouseState(&mx, &my);
  app->last_x = mx;
  app->last_y = my;

  SDL_SetRenderDrawColor(app->renderer, app->config.background_color[0], app->config.background_color[1], app->config.background_color[2], 255);
  SDL_RenderClear(app->renderer);

  // render texture
  SDL_FPoint pos = camera_world_to_screen(&app->camera, (float)(float)(float)app->current_x, (float)app->current_y);
  SDL_FRect r1   = {(float)app->current_x, (float)app->current_y, (float)app->current_w, (float)app->current_h};
  SDL_FRect r2   = {pos.x, pos.y, (float)app->current_w * app->camera.scale, (float)app->current_h * app->camera.scale};
  SDL_RenderTexture(app->renderer, app->capture_texture, &r1, &r2);

  // render grid
  render_grid(app, app->config.grid_size, app->config.grid_color[0], app->config.grid_color[1], app->config.grid_color[2]);

  // render state
  if (app->state == APP_MOVE_STATE) {
    app_iterate_move(app);
  } else if (app->state == APP_COLOR_STATE) {
    app_iterate_color(app);
  } else if (app->state == APP_FLASHLIGHT_STATE) {
    app_iterate_flashlight(app);
  } else {
    return SDL_APP_FAILURE;
  }

  return SDL_APP_CONTINUE;
}

void app_quit(app_t* app, SDL_AppResult result) {
  // free anything allocated within app variable.
  if (app != NULL) {
    if (app->capture_texture != NULL) SDL_DestroyTexture(app->capture_texture);
    if (app->font != NULL) TTF_CloseFont(app->font);
    if (app->icon != NULL) SDL_DestroySurface(app->icon);
    if (app->move_cursor != NULL) SDL_DestroyCursor(app->move_cursor);
    if (app->default_cursor != NULL) SDL_DestroyCursor(app->default_cursor);
    capture_free(&app->capture);

    TTF_Quit();
    SDL_Quit();
  }
}

bool app_event_move(app_t* app, SDL_Event* event) {
  return false;
}

void app_iterate_move(app_t* app) {
  camera_update(&app->camera);
}

bool app_event_color(app_t* app, SDL_Event* event) {
  // TODO:
  // auto handle_clipboard = [this, machine](auto func) {
  //   Capture& capture     = machine->get_capture();
  //   CameraSmooth& camera = machine->get_camera();
  //   float mx, my;
  //   SDL_GetMouseState(&mx, &my);
  //   SDL_FPoint mouse = camera.screen_to_world(mx, my);
  //   mouse.x          = std::round(mouse.x);
  //   mouse.y          = std::round(mouse.y);
  //   RGB rgb;
  //   if (capture.at(mouse.x, mouse.y, rgb) && !(mouse.x < machine->current_x || mouse.x > machine->current_x + machine->current_w - 1 || mouse.y < machine->current_y || mouse.y > machine->current_y + machine->current_h - 1)) {
  //     SDL_SetClipboardText(func(rgb).c_str());
  //   }
  // };

  switch (event->type) {
    case SDL_EVENT_KEY_DOWN: {
      SDL_Keycode code = event->key.key;
      SDL_Keymod mod   = SDL_GetModState();
      if (code == SDLK_C) {
        app->state = APP_MOVE_STATE;
        SDL_ShowCursor();
        return true;
      }
      // TODO:
      // else if (code == SDLK_d && (mod & SDL_KMOD_CTRL) && (mod & SDL_KMOD_LSHIFT)) {
      //   handle_clipboard(toDecimalSepString);
      // } else if (code == SDLK_d && (mod & SDL_KMOD_CTRL)) {
      //   handle_clipboard(toDecimalString);
      // } else if (code == SDLK_h && (mod & SDL_KMOD_CTRL) && (mod & SDL_KMOD_LSHIFT)) {
      //   handle_clipboard(toHexSepString);
      // } else if (code == SDLK_h && (mod & SDL_KMOD_CTRL)) {
      //   handle_clipboard(toHexString);
      // } else if (code == SDLK_b && (mod & SDL_KMOD_CTRL) && (mod & SDL_KMOD_LSHIFT)) {
      //   handle_clipboard(toBinarySepString);
      // } else if (code == SDLK_b && (mod & SDL_KMOD_CTRL)) {
      //   handle_clipboard(toBinaryString);
      // }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      app->recompute_text = true;
      break;
    }
  }
  return false;
}

static void check_recompute_text(app_t* app, int x, int y, capture_rgb_t rgb) {
  if (app->recompute_text) {
    char text_buf[128];
    SDL_snprintf(text_buf, sizeof(text_buf), "r: %3d g: %3d b: %3d\nx: %d y: %d", rgb.r, rgb.g, rgb.b, x, y);

    SDL_Color white = {255, 255, 255, 255};

    SDL_Surface* text_surface = TTF_RenderText_Solid_Wrapped(app->font, text_buf, 0, white, 0);
    if (text_surface == NULL) {
      SDL_Log("Failed to render text surface: %s", SDL_GetError());
    } else {
      if (app->text_texture != NULL) {
        SDL_DestroyTexture(app->text_texture);
        app->text_texture = NULL;
      }

      SDL_Texture* texture = SDL_CreateTextureFromSurface(app->renderer, text_surface);
      if (texture == NULL) {
        SDL_Log("Failed to create text texture: %s", SDL_GetError());
      } else {
        app->text_texture = texture;
      }

      SDL_DestroySurface(text_surface);
    }

    app->recompute_text = false;
  }
}

void app_iterate_color(app_t* app) {
  camera_update(&app->camera);

  float mx, my;
  SDL_GetMouseState(&mx, &my);
  SDL_FPoint mouse = camera_screen_to_world(&app->camera, mx, my);
  mouse.x          = roundf(mouse.x);
  mouse.y          = roundf(mouse.y);

  capture_rgb_t rgb;
  if (capture_at(&app->capture, (int)mouse.x, (int)mouse.y, &rgb) && !(mouse.x < app->current_x || mouse.x > app->current_x + app->current_w - 1 || mouse.y < app->current_y || mouse.y > app->current_y + app->current_h - 1)) {
    if (app->camera.scale > 7.5f) {
      SDL_FPoint p = camera_world_to_screen(&app->camera, mouse.x, mouse.y);

      float r          = rgb.r / 255.0f;
      float g          = rgb.g / 255.0f;
      float b          = rgb.b / 255.0f;
      float brightness = 0.299f * r + 0.587f * g + 0.114f * b;

      SDL_FRect r1 = {
          p.x,
          p.y,
          app->camera.scale,
          app->camera.scale,
      };

      if (brightness > 0.5f) {
        SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
      } else {
        SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);
      }

      int size = (int)(app->camera.scale / 7.5f);
      for (int i = 0; i < size; i++) {
        SDL_RenderRect(app->renderer, &r1);
        r1.x += 1;
        r1.y += 1;
        r1.w -= 2;
        r1.h -= 2;
      }

      mx = p.x;
      my = p.y;

      SDL_HideCursor();
    } else {
      SDL_ShowCursor();
    }

    check_recompute_text(app, (int)mouse.x, (int)mouse.y, rgb);

    const float panel_width  = 275.0f;
    const float panel_offset = 50.0f;

    SDL_FRect text_panel = {
        mx,
        my - app->text_texture->h - 1,
        panel_width,
        (float)app->text_texture->h,
    };
    text_panel.x += panel_offset;
    text_panel.y -= panel_offset;

    SDL_SetRenderDrawColor(app->renderer, 125, 125, 125, 255);
    SDL_RenderFillRect(app->renderer, &text_panel);
    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_RenderRect(app->renderer, &text_panel);

    SDL_FRect color_panel = {
        mx,
        my - text_panel.w - app->text_texture->h,
        panel_width,
        panel_width,
    };
    color_panel.x += panel_offset;
    color_panel.y -= panel_offset;

    SDL_SetRenderDrawColor(app->renderer, rgb.r, rgb.g, rgb.b, 255);
    SDL_RenderFillRect(app->renderer, &color_panel);

    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_RenderRect(app->renderer, &color_panel);

    SDL_FRect text_rect = {
        mx + (0.5f * (panel_width - app->text_texture->w)),
        my - app->text_texture->h - 1,
        (float)app->text_texture->w,
        (float)app->text_texture->h,
    };
    text_rect.x += panel_offset;
    text_rect.y -= panel_offset;

    SDL_RenderTexture(app->renderer, app->text_texture, NULL, &text_rect);
  } else {
    SDL_ShowCursor();
  }
}

void flashlight_zoom(flashlight_zoom_t* zoom, bool in) {
  zoom->zooming          = true;
  zoom->zoom_in          = in;
  zoom->zoom_tick        = (float)SDL_GetTicks();
  zoom->zoom_elapsed     = 0.0f;
  zoom->zoom_size_per_ms = zoom->zoom_amount / zoom->zoom_ms;
}

bool flashlight_update(flashlight_zoom_t* zoom) {
  if (!zoom->zooming) return false;

  float tick = (float)SDL_GetTicks();
  zoom->zoom_elapsed += tick - zoom->zoom_tick;

  if (zoom->zoom_in) {
    zoom->size += zoom->zoom_size_per_ms;
  } else {
    zoom->size -= zoom->zoom_size_per_ms;
  }

  if (zoom->size <= 0) {
    zoom->size    = 0.0f;
    zoom->zooming = false;
  }

  if (zoom->zoom_elapsed > zoom->zoom_ms) {
    zoom->zooming = false;
  }

  zoom->zoom_tick = tick;

  return true;
}

bool app_event_flashlight(app_t* app, SDL_Event* event) {
  switch (event->type) {
    case SDL_EVENT_MOUSE_WHEEL: {
      if ((SDL_GetModState() & SDL_KMOD_LSHIFT)) {
        flashlight_zoom(&app->flashlight_zoom, event->wheel.y <= 0);
        return true;
      }

      break;
    }
  }
  return false;
}

void app_iterate_flashlight(app_t* app) {
  camera_update(&app->camera);

  flashlight_update(&app->flashlight_zoom);

  float x, y;
  SDL_GetMouseState(&x, &y);
  draw_circle_flashlight(app->renderer, x, y, app->flashlight_zoom.size, 100,
                         (float)app->config.flashlight_center_inner_color[0] / 255.0f,
                         (float)app->config.flashlight_center_inner_color[1] / 255.0f,
                         (float)app->config.flashlight_center_inner_color[2] / 255.0f,
                         (float)app->config.flashlight_center_inner_color[3] / 255.0f,
                         (float)app->config.flashlight_center_outer_color[0] / 255.0f,
                         (float)app->config.flashlight_center_outer_color[1] / 255.0f,
                         (float)app->config.flashlight_center_outer_color[2] / 255.0f,
                         (float)app->config.flashlight_center_outer_color[3] / 255.0f,
                         (float)app->config.flashlight_outer_color[0] / 255.0f,
                         (float)app->config.flashlight_outer_color[1] / 255.0f,
                         (float)app->config.flashlight_outer_color[2] / 255.0f,
                         (float)app->config.flashlight_outer_color[3] / 255.0f);
}

SDL_Texture* create_capture_texture(SDL_Renderer* renderer, capture_t* capture) {
  if (renderer == NULL || capture == NULL || capture->pixels == NULL) return NULL;

  SDL_Surface* surface = SDL_CreateSurfaceFrom(capture->width, capture->height, SDL_PIXELFORMAT_RGB24, capture->pixels, capture->stride * 3);
  if (surface == NULL) {
    SDL_Log("Failed to create surface from capture pixels: %s", SDL_GetError());
    return NULL;
  }

  SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
  if (texture == NULL) {
    SDL_Log("Failed to create texture from surface: %s", SDL_GetError());
    SDL_DestroySurface(surface);
    return NULL;
  }

  SDL_DestroySurface(surface);
  return texture;
}

void render_grid(app_t* app, int grid_size, uint8_t r, uint8_t g, uint8_t b) {
  if (!app->grid_enabled) {
    return;
  }

  int x1 = app->current_x;
  int y1 = app->current_y;
  int x2 = app->current_x + app->current_w;
  int y2 = app->current_y + app->current_h;

  int lines_rendered = 0;

  if (app->camera.scale > 7.5f) {
    SDL_SetRenderDrawColor(app->renderer, r, g, b, 75);
    // Draw vertical grid lines
    for (int x = x1; x <= x2; ++x) {
      if (grid_size > 0 && x % grid_size == 0) continue;
      SDL_FPoint start = camera_world_to_screen(&app->camera, (float)x, (float)y1);
      SDL_FPoint end   = camera_world_to_screen(&app->camera, (float)x, (float)y2);
      SDL_RenderLine(app->renderer, start.x, start.y, end.x, end.y);
      lines_rendered++;
    }

    // Draw horizontal grid lines
    for (int y = y1; y <= y2; ++y) {
      if (grid_size > 0 && y % grid_size == 0) continue;
      SDL_FPoint start = camera_world_to_screen(&app->camera, (float)x1, (float)y);
      SDL_FPoint end   = camera_world_to_screen(&app->camera, (float)x2, (float)y);
      SDL_RenderLine(app->renderer, start.x, start.y, end.x, end.y);
      lines_rendered++;
    }
  }

  if (grid_size > 0) {
    SDL_SetRenderDrawColor(app->renderer, r, g, b, 150); // Set color to semi-transparent gray

    // Draw solid vertical grid lines
    for (int x = x1; x <= x2; ++x) {
      if (x % grid_size != 0) continue;
      SDL_FPoint start = camera_world_to_screen(&app->camera, (float)x, (float)y1);
      SDL_FPoint end   = camera_world_to_screen(&app->camera, (float)x, (float)y2);
      SDL_RenderLine(app->renderer, start.x, start.y, end.x, end.y);
      lines_rendered++;
    }

    // Draw solid horizontal grid lines
    for (int y = y1; y <= y2; ++y) {
      if (y % grid_size != 0) continue;
      SDL_FPoint start = camera_world_to_screen(&app->camera, (float)x1, (float)y);
      SDL_FPoint end   = camera_world_to_screen(&app->camera, (float)x2, (float)y);
      SDL_RenderLine(app->renderer, start.x, start.y, end.x, end.y);
      lines_rendered++;
    }
  }
}