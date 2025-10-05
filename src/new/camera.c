#include "camera.h"

void camera_reset(camera_t* camera) {
  camera->position.x = 0.0f;
  camera->position.y = 0.0f;
  camera->scale      = 1.0f;

  camera->zooming           = false;
  camera->zoom_scale_per_ms = 0.0f;
  camera->zoom_x            = 0.0f;
  camera->zoom_y            = 0.0f;
  camera->zoom_tick         = 0.0f;
  camera->zoom_elapsed      = 0.0f;
  camera->zoom_ms           = 0.0f;

  camera->panning     = false;
  camera->pan_vx      = 0.0f;
  camera->pan_vy      = 0.0f;
  camera->pan_damping = 0.0f;
  camera->pan_tick    = 0.0f;
  camera->pan_elapsed = 0.0f;
  camera->pan_ms      = 0.0f;
}

void camera_zoom(camera_t* camera, float amount, float mouse_x, float mouse_y) {
  SDL_FPoint zoom_point = camera_screen_to_world(camera, mouse_x, mouse_y);
  float init_x          = zoom_point.x;
  float init_y          = zoom_point.y;

  camera->scale *= (1.0f + amount);

  zoom_point = camera_screen_to_world(camera, mouse_x, mouse_y);
  float endX = zoom_point.x;
  float endY = zoom_point.y;

  camera->position.x -= (endX - init_x);
  camera->position.y -= (endY - init_y);
}

SDL_FPoint camera_update_relative(camera_t* camera, float x_rel, float y_rel) {
  SDL_FPoint p;
  p.x = x_rel / camera->scale;
  p.y = y_rel / camera->scale;
  return p;
}

void camera_pan(camera_t* camera, float x_rel, float y_rel) {
  SDL_FPoint rel = camera_update_relative(camera, x_rel, y_rel);
  camera->position.x -= rel.x;
  camera->position.y -= rel.y;
}

SDL_FPoint camera_world_to_screen(camera_t* camera, float world_x, float world_y) {
  SDL_FPoint p;
  p.x = (world_x - camera->position.x) * camera->scale;
  p.y = (world_y - camera->position.y) * camera->scale;
  return p;
}

SDL_FPoint camera_screen_to_world(camera_t* camera, float screen_x, float screen_y) {
  SDL_FPoint p;
  p.x = (screen_x / camera->scale) + camera->position.x;
  p.y = (screen_y / camera->scale) + camera->position.y;
  return p;
}

void camera_smooth_zoom(camera_t* camera, float amount, float mouse_x, float mouse_y, Uint64 milliseconds) {
  camera->zooming           = true;
  camera->zoom_x            = mouse_x;
  camera->zoom_y            = mouse_y;
  camera->zoom_ms           = (float)milliseconds;
  camera->zoom_tick         = (float)SDL_GetTicks();
  camera->zoom_elapsed      = 0.0f;
  camera->zoom_scale_per_ms = amount / camera->zoom_ms;
}

void camera_smooth_pan(camera_t* camera, float v_x, float v_y, float damping, Uint64 milliseconds) {
  camera->panning     = true;
  camera->pan_vx      = v_x;
  camera->pan_vy      = v_y;
  camera->pan_damping = damping;
  camera->pan_ms      = (float)milliseconds;
  camera->pan_tick    = (float)SDL_GetTicks();
  camera->pan_elapsed = 0.0f;
}

bool camera_update(camera_t* camera) {
  if (!camera->zooming && !camera->panning) return false;

  float tick = (float)SDL_GetTicks();

  if (camera->zooming) {
    camera->zoom_elapsed += tick - camera->zoom_tick;

    camera_zoom(camera, camera->zoom_scale_per_ms, camera->zoom_x, camera->zoom_y);

    if (camera->zoom_elapsed > camera->zoom_ms) {
      camera->zooming = false;
    }

    camera->zoom_tick = tick;
  }

  if (camera->panning) {
    camera->pan_elapsed += tick - camera->pan_tick;

    camera->pan_vx *= camera->pan_damping;
    camera->pan_vy *= camera->pan_damping;

    // TODO: PANS FOREVER, GIVE AN EPSILON
    if (camera->pan_vx == 0.0f && camera->pan_vy == 0.0f) {
      camera->panning = false;
      return false;
    } else {
      if (camera->pan_elapsed > camera->pan_ms) {
        camera_pan(camera, camera->pan_vx, camera->pan_vy);
        camera->pan_elapsed = 0.0f;
      }
    }

    camera->pan_tick = tick;
  }

  return true;
}
