#include "app.h"
#include "renderer.h"
#include "state.h"

#include <math.h>

static void recompute_mouse_text(app_t* app, float selection_x, float selection_y, float width, float height);
static void recompute_header_text(app_t* app, int x, int y, int width, int height);
static bool point_in_rect(float x, float y, float rx, float ry, float rw, float rh);
static crop_resize_selection_t get_resize_direction(float x, float y, SDL_FPoint start_screen, SDL_FPoint end_screen);

bool app_event_draw(app_t* app, SDL_Event* event) {
  switch (event->type) {
    case SDL_EVENT_KEY_DOWN: {
      SDL_Keycode code = event->key.key;
      if (!app->drawing) {
        if (code == SDLK_X) {
          app->current_x = (int)(app->start_crop.x);
          app->current_y = (int)(app->start_crop.y);
          app->current_w = (int)(app->end_crop.x - app->start_crop.x);
          app->current_h = (int)(app->end_crop.y - app->start_crop.y);
          app->state     = APP_MOVE_STATE;
          SDL_SetCursor(app->default_cursor);
          SDL_ShowCursor();
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (!app->drawing) {
        if (event->button.button == SDL_BUTTON_RIGHT) {
          app->drawing        = true;
          app->recompute_text = true;
          SDL_SetCursor(app->crosshair_cursor);

          if (app->resize_selection == RESIZE_SELECTION_N || app->resize_selection == RESIZE_SELECTION_S || app->resize_selection == RESIZE_SELECTION_E || app->resize_selection == RESIZE_SELECTION_W || app->resize_selection == RESIZE_SELECTION_SE) {
            app->start_crop = camera_world_to_screen(&app->camera, app->start_crop.x, app->start_crop.y);
            app->end_crop   = camera_world_to_screen(&app->camera, app->end_crop.x, app->end_crop.y);
          } else if (app->resize_selection == RESIZE_SELECTION_NW) {
            app->start_crop = camera_world_to_screen(&app->camera, app->end_crop.x, app->end_crop.y);
            app->end_crop   = camera_world_to_screen(&app->camera, app->start_crop.x, app->start_crop.y);
          } else if (app->resize_selection == RESIZE_SELECTION_NE) {
            app->start_crop = camera_world_to_screen(&app->camera, app->start_crop.x, app->end_crop.y);
            app->end_crop   = camera_world_to_screen(&app->camera, app->end_crop.x, app->start_crop.y);
          } else if (app->resize_selection == RESIZE_SELECTION_SW) {
            app->start_crop = camera_world_to_screen(&app->camera, app->end_crop.x, app->start_crop.y);
            app->end_crop   = camera_world_to_screen(&app->camera, app->start_crop.x, app->end_crop.y);
          } else if (app->resize_selection == RESIZE_SELECTION_CENTER) {
            app->start_crop = camera_world_to_screen(&app->camera, app->start_crop.x, app->start_crop.y);
            app->end_crop   = camera_world_to_screen(&app->camera, app->end_crop.x, app->end_crop.y);
          } else {
            SDL_SetCursor(SDL_GetDefaultCursor());
            app->drawing        = false;
            app->recompute_text = false;
            return false;
          }

        } else if (event->button.button == SDL_BUTTON_LEFT) {
          return false;
        }
        return true;
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event->button.button == SDL_BUTTON_LEFT) {
        if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) {
          SDL_SetCursor(app->crosshair_cursor);
        }
        return false; // process smooth zoom
      } else if (event->button.button == SDL_BUTTON_RIGHT) {
        if (app->start_crop.x == app->end_crop.x && app->start_crop.y == app->end_crop.y) {
          app->state = APP_MOVE_STATE;
          SDL_SetCursor(app->default_cursor);
          SDL_ShowCursor();
          return true;
        }

        app->drawing = false;

        float x1        = MIN(app->start_crop.x, app->end_crop.x);
        float y1        = MIN(app->start_crop.y, app->end_crop.y);
        float x2        = MAX(app->start_crop.x, app->end_crop.x);
        float y2        = MAX(app->start_crop.y, app->end_crop.y);
        app->start_crop = (SDL_FPoint){x1, y1};
        app->end_crop   = (SDL_FPoint){x2, y2};

        app->start_crop = camera_screen_to_world(&app->camera, app->start_crop.x, app->start_crop.y);
        app->end_crop   = camera_screen_to_world(&app->camera, app->end_crop.x, app->end_crop.y);

        app->start_crop = (SDL_FPoint){roundf(app->start_crop.x), roundf(app->start_crop.y)};
        app->end_crop   = (SDL_FPoint){roundf(app->end_crop.x), roundf(app->end_crop.y)};

        if (app->start_crop.x < app->current_x) app->start_crop.x = (float)app->current_x;
        if (app->start_crop.x >= app->current_x + app->current_w) app->start_crop.x = (float)(app->current_x + app->current_w);
        if (app->start_crop.y < app->current_y) app->start_crop.y = (float)app->current_y;
        if (app->start_crop.y >= app->current_y + app->current_h) app->start_crop.y = (float)(app->current_y + app->current_h);
        if (app->end_crop.x < app->current_x) app->end_crop.x = (float)app->current_x;
        if (app->end_crop.x >= app->current_x + app->current_w) app->end_crop.x = (float)(app->current_x + app->current_w);
        if (app->end_crop.y < app->current_y) app->end_crop.y = (float)app->current_y;
        if (app->end_crop.y >= app->current_y + app->current_h) app->end_crop.y = (float)(app->current_y + app->current_h);

        if (app->end_crop.x - app->start_crop.x == 0 || app->end_crop.y - app->start_crop.y == 0) {
          app->state = APP_MOVE_STATE;
          SDL_SetCursor(app->default_cursor);
          SDL_ShowCursor();
          return true;
        }

        app->recompute_text = true;

        SDL_SetCursor(app->default_cursor);

        return true;
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      if (app->drawing) {
        if (app->resize_selection != RESIZE_SELECTION_CENTER) {
          if (event->motion.state & SDL_BUTTON_MASK(SDL_BUTTON_LEFT)) {
            app->start_crop.x += event->motion.xrel;
            app->start_crop.y += event->motion.yrel;
            if (app->resize_selection == RESIZE_SELECTION_N || app->resize_selection == RESIZE_SELECTION_W) {
              app->end_crop.y += event->motion.yrel;
              app->end_crop.x += event->motion.xrel;
            } else if (app->resize_selection == RESIZE_SELECTION_E) {
              app->end_crop.y += event->motion.yrel;
            } else if (app->resize_selection == RESIZE_SELECTION_S) {
              app->end_crop.x += event->motion.xrel;
            }

            return false; // we still want to move the capture.
          }
        } else {
          if (event->motion.state & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) {
            app->start_crop.x += event->motion.xrel;
            app->start_crop.y += event->motion.yrel;
            app->end_crop.x += event->motion.xrel;
            app->end_crop.y += event->motion.yrel;
            app->recompute_text = true;
            return false;
          }
        }

        app->recompute_text = true;
      } else {
        SDL_FPoint start_screen = camera_world_to_screen(&app->camera, app->start_crop.x, app->start_crop.y);
        SDL_FPoint end_screen   = camera_world_to_screen(&app->camera, app->end_crop.x, app->end_crop.y);

        if (!(event->motion.state & SDL_BUTTON_MASK(SDL_BUTTON_LEFT))) {
          app->resize_selection = get_resize_direction(event->motion.x, event->motion.y, start_screen, end_screen);

          if (app->resize_selection == RESIZE_SELECTION_N || app->resize_selection == RESIZE_SELECTION_S) {
            SDL_SetCursor(app->ns_cursor);
          } else if (app->resize_selection == RESIZE_SELECTION_E || app->resize_selection == RESIZE_SELECTION_W) {
            SDL_SetCursor(app->ew_cursor);
          } else if (app->resize_selection == RESIZE_SELECTION_NE || app->resize_selection == RESIZE_SELECTION_SW) {
            SDL_SetCursor(app->nesw_cursor);
          } else if (app->resize_selection == RESIZE_SELECTION_NW || app->resize_selection == RESIZE_SELECTION_SE) {
            SDL_SetCursor(app->nwse_cursor);
          } else {
            SDL_SetCursor(app->default_cursor);
          }
        }

        return false;
      }
      break;
    }

    case SDL_EVENT_MOUSE_WHEEL: {
      break;
    }
  }
  return false;
}

void app_iterate_draw(app_t* app) {
  float mx, my;
  SDL_GetMouseState(&mx, &my);

  if (app->camera.panning && SDL_GetMouseState(NULL, NULL) & SDL_BUTTON_MASK(SDL_BUTTON_RIGHT)) {
    app->recompute_text = true;
  }

  float text_padding = 25.0f;
  uint8_t alpha      = 150;

  if (app->drawing) {
    if (app->resize_selection != RESIZE_SELECTION_CENTER) {
      float mx, my;
      SDL_GetMouseState(&mx, &my);

      if (app->resize_selection == RESIZE_SELECTION_N) {
        app->start_crop = (SDL_FPoint){app->start_crop.x, my};
      } else if (app->resize_selection == RESIZE_SELECTION_E) {
        app->end_crop = (SDL_FPoint){mx, app->end_crop.y};
      } else if (app->resize_selection == RESIZE_SELECTION_S) {
        app->end_crop = (SDL_FPoint){app->end_crop.x, my};
      } else if (app->resize_selection == RESIZE_SELECTION_W) {
        app->start_crop = (SDL_FPoint){mx, app->start_crop.y};
      } else {
        app->end_crop = (SDL_FPoint){mx, my};
      }

      SDL_Keymod mod = SDL_GetModState();
      if (mod & SDL_KMOD_SHIFT) {
        if (app->start_crop.x < app->end_crop.x && app->start_crop.y < app->end_crop.y) { // quad 4
          app->end_crop.y += (app->end_crop.x - app->start_crop.x) - (app->end_crop.y - app->start_crop.y);
        } else if (app->start_crop.x < app->end_crop.x && app->start_crop.y >= app->end_crop.y) { // quad 1
          app->end_crop.y += (app->start_crop.x - app->end_crop.x) - (app->end_crop.y - app->start_crop.y);
        } else if (app->start_crop.x >= app->end_crop.x && app->start_crop.y < app->end_crop.y) { // quad 3
          app->end_crop.y -= (app->end_crop.x - app->start_crop.x) - (app->start_crop.y - app->end_crop.y);
        } else if (app->start_crop.x >= app->end_crop.x && app->start_crop.y >= app->end_crop.y) { // quad 2
          app->end_crop.y -= (app->start_crop.x - app->end_crop.x) - (app->start_crop.y - app->end_crop.y);
        }
      }
    }

    SDL_FPoint start_screen = camera_screen_to_world(&app->camera, app->start_crop.x, app->start_crop.y);
    SDL_FPoint end_screen   = camera_screen_to_world(&app->camera, app->end_crop.x, app->end_crop.y);

    if (camera_update(&app->camera)) {
      app->start_crop = camera_world_to_screen(&app->camera, start_screen.x, start_screen.y);
      app->end_crop   = camera_world_to_screen(&app->camera, end_screen.x, end_screen.y);
    }

    float width  = end_screen.x - start_screen.x;
    float height = end_screen.y - start_screen.y;

    float x1 = MIN(app->start_crop.x, app->end_crop.x);
    float y1 = MIN(app->start_crop.y, app->end_crop.y);
    float x2 = MAX(app->start_crop.x, app->end_crop.x);
    float y2 = MAX(app->start_crop.y, app->end_crop.y);

    draw_rect_flashlight(app->renderer, x1, y1, x2 - x1, y2 - y1, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.5f);

    SDL_SetRenderDrawColor(app->renderer, 200, 200, 200, alpha);
    SDL_RenderLine(app->renderer, x1, y1, x2, y1);
    SDL_RenderLine(app->renderer, x1, y1, x1, y2);
    SDL_RenderLine(app->renderer, x1, y2, x2, y2);
    SDL_RenderLine(app->renderer, x2, y1, x2, y2);

    if (app->recompute_text) {
      float selection_x, selection_y;
      if (app->resize_selection == RESIZE_SELECTION_CENTER) {
        selection_x = start_screen.x;
        selection_y = start_screen.y;
      } else if (width > 0 && height > 0) {
        selection_x = start_screen.x;
        selection_y = start_screen.y;
      } else if (width <= 0 && height > 0) {
        selection_x = start_screen.x + width;
        selection_y = start_screen.y;
      } else if (width > 0 && height <= 0) {
        selection_x = start_screen.x;
        selection_y = start_screen.y + height;
      } else {
        selection_x = start_screen.x + width;
        selection_y = start_screen.y + height;
      }

      recompute_mouse_text(app, selection_x, selection_y, fabsf(width), fabsf(height));
    }

    float offset = 50.0f;

    float text_x, text_y;
    if (width == 0 && height == 0 || width > 0 && height > 0) {
      text_x = mx + offset;
      text_y = my + offset;
    } else if (width <= 0 && height > 0) {
      text_x = mx - app->text_texture->w - offset;
      text_y = my + offset;
    } else if (width > 0 && height <= 0) {
      text_x = mx + offset;
      text_y = my - app->text_texture->h - offset;
    } else {
      text_x = mx - app->text_texture->w - offset;
      text_y = my - app->text_texture->h - offset;
    }

    SDL_FRect text_rect = {text_x, text_y, (float)app->text_texture->w, (float)app->text_texture->h};

    SDL_FRect text_boundry_rect = text_rect;
    text_boundry_rect.x -= text_padding;
    text_boundry_rect.y -= text_padding;
    text_boundry_rect.w += 2.0f * text_padding;
    text_boundry_rect.h += 2.0f * text_padding;

    SDL_SetRenderDrawColor(app->renderer, 125, 125, 125, alpha);
    SDL_RenderFillRect(app->renderer, &text_boundry_rect);
    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, alpha);
    SDL_RenderRect(app->renderer, &text_boundry_rect);

    SDL_RenderTexture(app->renderer, app->text_texture, NULL, &text_rect);

  } else {
    SDL_FPoint start_screen = camera_world_to_screen(&app->camera, app->start_crop.x, app->start_crop.y);
    SDL_FPoint end_screen   = camera_world_to_screen(&app->camera, app->end_crop.x, app->end_crop.y);

    camera_update(&app->camera);

    draw_rect_flashlight(app->renderer, start_screen.x, start_screen.y, end_screen.x - start_screen.x, end_screen.y - start_screen.y, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.5f);

    SDL_SetRenderDrawColor(app->renderer, 200, 200, 200, alpha);
    SDL_RenderLine(app->renderer, start_screen.x, start_screen.y, end_screen.x, start_screen.y);
    SDL_RenderLine(app->renderer, start_screen.x, start_screen.y, start_screen.x, end_screen.y);
    SDL_RenderLine(app->renderer, start_screen.x, end_screen.y, end_screen.x, end_screen.y);
    SDL_RenderLine(app->renderer, end_screen.x, start_screen.y, end_screen.x, end_screen.y);

    if (app->recompute_text) {
      int width  = (int)(app->end_crop.x - app->start_crop.x);
      int height = (int)(app->end_crop.y - app->start_crop.y);
      int x      = (int)(app->end_crop.x - width);
      int y      = (int)(app->end_crop.y - height);

      recompute_header_text(app, x, y, width, height);
    }

    int w, h;
    SDL_GetWindowSize(SDL_GetRenderWindow(app->renderer), &w, &h);

    SDL_FRect text_rect = {
        text_padding,
        text_padding,
        (float)app->text_texture->w,
        (float)app->text_texture->h,
    };

    SDL_FRect text_boundry_rect = {
        0,
        0,
        (float)app->text_texture->w + (text_padding * 2.0f),
        (float)app->text_texture->h + (text_padding * 2.0f),
    };

    SDL_SetRenderDrawColor(app->renderer, 125, 125, 125, alpha);
    SDL_RenderFillRect(app->renderer, &text_boundry_rect);

    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, alpha);
    SDL_RenderRect(app->renderer, &text_boundry_rect);

    SDL_RenderTexture(app->renderer, app->text_texture, NULL, &text_rect);
  }
}

static bool point_in_rect(float x, float y, float rx, float ry, float rw, float rh) {
  return (x >= rx && x <= (rx + rw) && y >= ry && y <= (ry + rh));
}

static crop_resize_selection_t get_resize_direction(float x, float y, SDL_FPoint start_screen, SDL_FPoint end_screen) {
  const float resize_rect_size = 15.0f;

  float screen_w      = end_screen.x - start_screen.x;
  float screen_h      = end_screen.y - start_screen.y;
  float corner_rect_w = screen_w / 2.0f <= resize_rect_size ? screen_w / 2.0f : resize_rect_size;
  float corner_rect_h = screen_h / 2.0f <= resize_rect_size ? screen_h / 2.0f : resize_rect_size;

  if (screen_h / 2.0f > resize_rect_size) {
    float vertical_rect_w = corner_rect_w;
    float vertical_rect_h = screen_h - 2.0f * corner_rect_h;

    if (point_in_rect(x, y, start_screen.x, start_screen.y + corner_rect_h, vertical_rect_w, vertical_rect_h)) {
      return RESIZE_SELECTION_W;
    } else if (point_in_rect(x, y, end_screen.x - vertical_rect_w, start_screen.y + corner_rect_h, vertical_rect_w, vertical_rect_h)) {
      return RESIZE_SELECTION_E;
    }
  }

  if (screen_w / 2.0f > resize_rect_size) {
    float horizontal_rect_w = screen_w / 2.0f <= resize_rect_size ? 0.0f : screen_w - 2.0f * corner_rect_w;
    float horizontal_rect_h = screen_w / 2.0f <= resize_rect_size ? 0.0f : corner_rect_h;

    if (point_in_rect(x, y, start_screen.x + corner_rect_w, start_screen.y, horizontal_rect_w, horizontal_rect_h)) {
      return RESIZE_SELECTION_N;
    } else if (point_in_rect(x, y, start_screen.x + corner_rect_w, end_screen.y - corner_rect_h, horizontal_rect_w, horizontal_rect_h)) {
      return RESIZE_SELECTION_S;
    }
  }

  if (point_in_rect(x, y, start_screen.x, start_screen.y, corner_rect_w, corner_rect_h)) {
    return RESIZE_SELECTION_NW;
  } else if (point_in_rect(x, y, start_screen.x, end_screen.y - corner_rect_h, corner_rect_w, corner_rect_h)) {
    return RESIZE_SELECTION_SW;
  } else if (point_in_rect(x, y, end_screen.x - corner_rect_w, end_screen.y - corner_rect_h, corner_rect_w, corner_rect_h)) {
    return RESIZE_SELECTION_SE;
  } else if (point_in_rect(x, y, end_screen.x - corner_rect_w, start_screen.y, corner_rect_w, corner_rect_h)) {
    return RESIZE_SELECTION_NE;
  }

  if (screen_h / 2.0f > resize_rect_size && screen_w / 2.0f > resize_rect_size) {
    if (point_in_rect(x, y, start_screen.x + corner_rect_w, start_screen.y + corner_rect_h, screen_w - 2.0f * corner_rect_w, screen_h - 2.0f * corner_rect_h)) {
      return RESIZE_SELECTION_CENTER;
    }
  }

  return RESIZE_SELECTION_NONE;
}

static void recompute_mouse_text(app_t* app, float selection_x, float selection_y, float width, float height) {
  char text_buf[128];
  SDL_snprintf(text_buf, sizeof(text_buf), "x=%.2f y=%.2f\nw=%.2f h=%.2f", selection_x, selection_y, width, height);

  SDL_Surface* text_surface = TTF_RenderText_Solid_Wrapped(app->font, text_buf, 0, (SDL_Color){255, 117, 24, 200}, 0);
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

static void recompute_header_text(app_t* app, int x, int y, int width, int height) {
  char text_buf[128];
  SDL_snprintf(text_buf, sizeof(text_buf), "x=%d y=%d\nw=%d h=%d", x, y, width, height);

  SDL_Surface* text_surface = TTF_RenderText_Solid_Wrapped(app->font, text_buf, 0, (SDL_Color){255, 117, 24, 200}, 0);
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