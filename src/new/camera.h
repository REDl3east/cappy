#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "SDL3/SDL.h"

#include <stdbool.h>

typedef struct camera_t {
  SDL_FPoint position;
  float scale;

  bool zooming;
  float zoom_scale_per_ms;
  float zoom_x;
  float zoom_y;
  float zoom_tick;
  float zoom_elapsed;
  float zoom_ms;

  bool panning;
  float pan_vx;
  float pan_vy;
  float pan_damping;
  float pan_tick;
  float pan_elapsed;
  float pan_ms;

} camera_t;

void camera_reset(camera_t* camera);
void camera_zoom(camera_t* camera, float amount, float mouse_x, float mouse_y);
SDL_FPoint camera_update_relative(camera_t* camera, float x_rel, float y_rel);
void camera_pan(camera_t* camera, float x_rel, float y_rel);
SDL_FPoint camera_world_to_screen(camera_t* camera, float world_x, float world_y);
SDL_FPoint camera_screen_to_world(camera_t* camera, float screen_x, float screen_y);

void camera_smooth_zoom(camera_t* camera, float amount, float mouse_x, float mouse_y, Uint64 milliseconds);
void camera_smooth_pan(camera_t* camera, float v_x, float v_y, float damping, Uint64 milliseconds);
bool camera_update();

#endif