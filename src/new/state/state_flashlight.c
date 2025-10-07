#include "app.h"
#include "renderer.h"
#include "state.h"

static void flashlight_zoom(flashlight_zoom_t* zoom, bool in);
static bool flashlight_update(flashlight_zoom_t* zoom);

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

static void flashlight_zoom(flashlight_zoom_t* zoom, bool in) {
  zoom->zooming          = true;
  zoom->zoom_in          = in;
  zoom->zoom_tick        = (float)SDL_GetTicks();
  zoom->zoom_elapsed     = 0.0f;
  zoom->zoom_size_per_ms = zoom->zoom_amount / zoom->zoom_ms;
}

static bool flashlight_update(flashlight_zoom_t* zoom) {
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