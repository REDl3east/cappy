#include "app.h"

#include "assets/icon.h"
#include "assets/intelone_mono_bold.h"
#include "sb.h"
#include "state/state.h"
#include "stb_image_write.h"
#include "sv.h"

#define SDL_MAIN_USE_CALLBACKS
#include "SDL3/SDL_main.h"

#include <math.h>
#include <stdio.h>

static SDL_Texture* create_capture_texture(SDL_Renderer* renderer, capture_t* capture);
static void app_iterate_grid(app_t* app, int grid_size, uint8_t r, uint8_t g, uint8_t b);

static void save_capture(const char* file, app_t* app);
static void save_dialog_callback(void* userdata, const char* const* filelist, int filter);

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
  SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, app->capture.width);
  SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, app->capture.height);
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

  app->font = TTF_OpenFontIO(SDL_IOFromConstMem(intelone_mono_bold, sizeof(intelone_mono_bold)), true, app->config.font_size);
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

  app->drawing          = false;
  app->start_crop.x     = 0;
  app->start_crop.y     = 0;
  app->end_crop.x       = 0;
  app->end_crop.y       = 0;
  app->resize_selection = RESIZE_SELECTION_NONE;

  app->flashlight_zoom.size             = app->config.flashlight_size;
  app->flashlight_zoom.zooming          = false;
  app->flashlight_zoom.zoom_in          = false;
  app->flashlight_zoom.zoom_amount      = 150.0f;
  app->flashlight_zoom.zoom_ms          = 25.0f;
  app->flashlight_zoom.zoom_tick        = 0.0f;
  app->flashlight_zoom.zoom_elapsed     = 0.0f;
  app->flashlight_zoom.zoom_size_per_ms = 0.0f;

  app->save_file_event = SDL_RegisterEvents(1);

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

#define INIT_CURSOR(name, id)                                         \
  do {                                                                \
    name = SDL_CreateSystemCursor(id);                                \
    if (name == NULL) {                                               \
      SDL_Log("Failed to create " #id " cursor: %s", SDL_GetError()); \
      return SDL_APP_FAILURE;                                         \
    }                                                                 \
  } while (0)

  INIT_CURSOR(app->default_cursor, SDL_SYSTEM_CURSOR_DEFAULT);
  INIT_CURSOR(app->move_cursor, SDL_SYSTEM_CURSOR_MOVE);
  INIT_CURSOR(app->crosshair_cursor, SDL_SYSTEM_CURSOR_CROSSHAIR);
  INIT_CURSOR(app->ns_cursor, SDL_SYSTEM_CURSOR_NS_RESIZE);
  INIT_CURSOR(app->ew_cursor, SDL_SYSTEM_CURSOR_EW_RESIZE);
  INIT_CURSOR(app->nwse_cursor, SDL_SYSTEM_CURSOR_NWSE_RESIZE);
  INIT_CURSOR(app->nesw_cursor, SDL_SYSTEM_CURSOR_NESW_RESIZE);
  INIT_CURSOR(app->wait_cursor, SDL_SYSTEM_CURSOR_WAIT);

#undef INIT_CURSOR

  SDL_SetCursor(app->default_cursor);

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
  } else if (app->state == APP_DRAW_STATE) {
    handled = app_event_draw(app, event);
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
        if (app->state == APP_MOVE_STATE || app->state == APP_COLOR_STATE || app->state == APP_DRAW_STATE) {
          app->state = APP_FLASHLIGHT_STATE;
          SDL_HideCursor();
        } else if (app->state == APP_FLASHLIGHT_STATE) {
          app->state = APP_MOVE_STATE;
          SDL_ShowCursor();
        } else {
          return SDL_APP_FAILURE;
        }
      } else if (code == SDLK_C) {
        if (app->state == APP_MOVE_STATE || app->state == APP_FLASHLIGHT_STATE) {
          app->state          = APP_COLOR_STATE;
          app->recompute_text = true;
        } else if (app->state == APP_DRAW_STATE) {
          app->state          = APP_COLOR_STATE;
          app->recompute_text = true;
          SDL_SetCursor(app->default_cursor);
        } else if (app->state == APP_COLOR_STATE) {
          app->state = APP_MOVE_STATE;
          SDL_ShowCursor();
        } else {
          return SDL_APP_FAILURE;
        }

      } else if (code == SDLK_G) {
        app->grid_enabled = !app->grid_enabled;
      } else if (code == SDLK_R) {
        // camera_reset(&app->camera);
        app->current_x = 0;
        app->current_y = 0;
        app->current_w = app->capture.width;
        app->current_h = app->capture.height;
        app->state     = APP_MOVE_STATE;
        SDL_ShowCursor();
      } else if (code == SDLK_M) {
        SDL_MinimizeWindow(app->window);
      } else if (code == SDLK_S && mod & SDL_KMOD_CTRL) {
        static const SDL_DialogFileFilter filters[] = {
            {"PNG images", "png"}};
        SDL_ShowSaveFileDialog(save_dialog_callback, (void*)app, app->window, filters, SDL_arraysize(filters), NULL);
      }

      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        app->camera.panning = false;

        SDL_SetCursor(app->move_cursor);
      } else if (event->button.button == SDL_BUTTON_RIGHT) {
        if (app->state == APP_MOVE_STATE || app->state == APP_FLASHLIGHT_STATE || app->state == APP_COLOR_STATE || app->state == APP_DRAW_STATE) {
          float mx, my;
          SDL_GetMouseState(&mx, &my);

          app->state = APP_DRAW_STATE;

          app->resize_selection = RESIZE_SELECTION_NONE;
          app->recompute_text   = true;
          app->drawing          = true;
          app->start_crop.x     = mx;
          app->start_crop.y     = my;
          app->end_crop.x       = mx;
          app->end_crop.y       = my;

          SDL_ShowCursor();
          SDL_SetCursor(app->crosshair_cursor);
        } else {
          return SDL_APP_FAILURE;
        }
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
  }

  // custom events
  if (event->type == app->save_file_event) {
    SDL_SetCursor(app->wait_cursor);

    char* path = (char*)(event->user.data1);

    save_capture(path, app);

    SDL_SetCursor(app->default_cursor);
    SDL_free(path);
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

  // render capture texture
  SDL_FPoint pos = camera_world_to_screen(&app->camera, (float)(float)(float)app->current_x, (float)app->current_y);
  SDL_FRect r1   = {(float)app->current_x, (float)app->current_y, (float)app->current_w, (float)app->current_h};
  SDL_FRect r2   = {pos.x, pos.y, (float)app->current_w * app->camera.scale, (float)app->current_h * app->camera.scale};
  SDL_RenderTexture(app->renderer, app->capture_texture, &r1, &r2);

  // render grid
  app_iterate_grid(app, app->config.grid_size, app->config.grid_color[0], app->config.grid_color[1], app->config.grid_color[2]);

  // render state
  if (app->state == APP_MOVE_STATE) {
    app_iterate_move(app);
  } else if (app->state == APP_COLOR_STATE) {
    app_iterate_color(app);
  } else if (app->state == APP_FLASHLIGHT_STATE) {
    app_iterate_flashlight(app);
  } else if (app->state == APP_DRAW_STATE) {
    app_iterate_draw(app);
  } else {
    return SDL_APP_FAILURE;
  }

  return SDL_APP_CONTINUE;
}

void app_quit(app_t* app, SDL_AppResult result) {
  if (app != NULL) {
    capture_free(&app->capture);
    if (app->capture_texture != NULL) SDL_DestroyTexture(app->capture_texture);
    if (app->font != NULL) TTF_CloseFont(app->font);
    if (app->icon != NULL) SDL_DestroySurface(app->icon);
    if (app->move_cursor != NULL) SDL_DestroyCursor(app->move_cursor);
    if (app->default_cursor != NULL) SDL_DestroyCursor(app->default_cursor);
    if (app->crosshair_cursor != NULL) SDL_DestroyCursor(app->crosshair_cursor);
    if (app->ns_cursor != NULL) SDL_DestroyCursor(app->ns_cursor);
    if (app->ew_cursor != NULL) SDL_DestroyCursor(app->ew_cursor);
    if (app->nwse_cursor != NULL) SDL_DestroyCursor(app->nwse_cursor);
    if (app->nesw_cursor != NULL) SDL_DestroyCursor(app->nesw_cursor);
    if (app->wait_cursor != NULL) SDL_DestroyCursor(app->wait_cursor);

    TTF_Quit();
    SDL_Quit();
  }
}

static SDL_Texture* create_capture_texture(SDL_Renderer* renderer, capture_t* capture) {
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

static void app_iterate_grid(app_t* app, int grid_size, uint8_t r, uint8_t g, uint8_t b) {
  if (!app->grid_enabled) return;

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

static void save_capture(const char* file, app_t* app) {
  cstring_view sv     = sv_create_from_cstr(file);
  string_builder_t sb = {0};

  if (sv_starts_with(sv, svl("file://"))) {
    sv = sv_remove_prefix(sv, 7);
  } else if (sv_starts_with(sv, svl("file:/"))) {
    sv = sv_remove_prefix(sv, 6);
  }

  sb_append_buffer(&sb, sv.data, sv.length);

  if (!sv_ends_with(sv, svl(".png"))) {
    sb_append_buffer(&sb, ".png", 4);
  }

  sb_append_null(&sb);

  const int comp              = 3;
  const int stride            = app->capture.stride;
  const int index             = app->current_y * stride + app->current_x;
  const capture_rgb_t* pixels = app->capture.pixels + index;

  if (stbi_write_png(sb.string, app->current_w, app->current_h, comp, pixels, comp * stride) == 0) {
    SDL_Log("Failed to save file: '%s'", sb.string);
  } else {
    SDL_Log("Saved file: '%s'", sb.string);
  }

  sb_free(&sb);
}

static void save_dialog_callback(void* userdata, const char* const* filelist, int filter) {
  app_t* app = (app_t*)userdata;
  if (filelist) {
    if (!*filelist) {
      SDL_Log("Save dialog canceled.");
      return;
    }

    SDL_Event event  = {0};
    event.type       = app->save_file_event;
    event.user.data1 = SDL_strdup(*filelist);
    SDL_PushEvent(&event);
  } else {
    SDL_Log("Error: %s\n", SDL_GetError());
  }
}