#include "app.h"
#include "renderer.h"
#include "state.h"

static bool point_in_rect(float x, float y, float rx, float ry, float rw, float rh);
static crop_resize_selection_t get_resize_direction(float x, float y, SDL_FPoint start_screen, SDL_FPoint end_screen);

bool app_event_draw(app_t* app, SDL_Event* event) {
  switch (event->type) {
    case SDL_EVENT_KEY_DOWN: {
      SDL_Keycode code = event->key.key;
      if (!app->drawing) {
        if (code == SDLK_X) {
          app->current_x = (int)app->start_crop.x;
          app->current_y = (int)app->start_crop.y;
          app->current_w = (int)(app->end_crop.x - app->start_crop.x);
          app->current_h = (int)(app->end_crop.y - app->start_crop.y);
          app->state     = APP_MOVE_STATE;
          SDL_SetCursor(app->default_cursor);
          return true;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (!app->drawing) {
        if (event->button.button == SDL_BUTTON_RIGHT) {
          app->drawing = true;
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
            SDL_SetCursor(app->default_cursor);
            app->drawing = false;
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

        app->start_crop = (SDL_FPoint){SDL_roundf(app->start_crop.x), SDL_roundf(app->start_crop.y)};
        app->end_crop   = (SDL_FPoint){SDL_roundf(app->end_crop.x), SDL_roundf(app->end_crop.y)};

        if (app->start_crop.x < app->current_x) app->start_crop.x = (float)app->current_x;
        if (app->start_crop.x >= app->current_x + app->current_w) app->start_crop.x = app->current_x + (float)app->current_w;
        if (app->start_crop.y < app->current_y) app->start_crop.y = (float)app->current_y;
        if (app->start_crop.y >= app->current_y + app->current_h) app->start_crop.y = app->current_y + (float)app->current_h;
        if (app->end_crop.x < app->current_x) app->end_crop.x = (float)app->current_x;
        if (app->end_crop.x >= app->current_x + app->current_w) app->end_crop.x = app->current_x + (float)app->current_w;
        if (app->end_crop.y < app->current_y) app->end_crop.y = (float)app->current_y;
        if (app->end_crop.y >= app->current_y + app->current_h) app->end_crop.y = app->current_y + (float)app->current_h;

        if (app->end_crop.x - app->start_crop.x == 0 || app->end_crop.y - app->start_crop.y == 0) {
          app->state = APP_MOVE_STATE;
          SDL_SetCursor(app->default_cursor);
          return true;
        }

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
            return false;
          }
        }

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
            SDL_SetCursor(SDL_GetDefaultCursor());
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
  SDL_FRect crop_rect;

  float mx, my;
  SDL_GetMouseState(&mx, &my);

  float text_padding = 10.0f;

  if (app->drawing) {
    if (app->resize_selection != RESIZE_SELECTION_CENTER) {
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

    float x1 = MIN(app->start_crop.x, app->end_crop.x);
    float y1 = MIN(app->start_crop.y, app->end_crop.y);
    float x2 = MAX(app->start_crop.x, app->end_crop.x);
    float y2 = MAX(app->start_crop.y, app->end_crop.y);

    crop_rect.x = x1;
    crop_rect.y = y1;
    crop_rect.w = x2 - x1;
    crop_rect.h = y2 - y1;

  } else {
    SDL_FPoint start_screen = camera_world_to_screen(&app->camera, app->start_crop.x, app->start_crop.y);
    SDL_FPoint end_screen   = camera_world_to_screen(&app->camera, app->end_crop.x, app->end_crop.y);

    camera_update(&app->camera);

    crop_rect.x = start_screen.x;
    crop_rect.y = start_screen.y;
    crop_rect.w = end_screen.x - start_screen.x;
    crop_rect.h = end_screen.y - start_screen.y;
  }

  draw_rect_flashlight(app->renderer, crop_rect.x, crop_rect.y, crop_rect.w, crop_rect.h, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.5f);

  SDL_SetRenderDrawColor(app->renderer, 200, 200, 200, 200);
  SDL_RenderLine(app->renderer, crop_rect.x, crop_rect.y, crop_rect.x + crop_rect.w, crop_rect.y);
  SDL_RenderLine(app->renderer, crop_rect.x, crop_rect.y, crop_rect.x, crop_rect.y + crop_rect.h);
  SDL_RenderLine(app->renderer, crop_rect.x + crop_rect.w, crop_rect.y + crop_rect.h, crop_rect.x + crop_rect.w, crop_rect.y);
  SDL_RenderLine(app->renderer, crop_rect.x + crop_rect.w, crop_rect.y + crop_rect.h, crop_rect.x, crop_rect.y + crop_rect.h);
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
};