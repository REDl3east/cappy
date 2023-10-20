#ifndef _CAMERA_H_
#define _CAMERA_H_

#include "SDL3/SDL.h"

class Camera {
public:
  Camera() {}
  void reset() {
    m_scale    = 1.0f;
    m_position = {0.0f, 0.0f};
  }

  void set_position(SDL_FPoint position);
  SDL_FPoint get_position() const;

  void set_scale(float scale);
  float get_scale() const;

  void zoom(float amount, float mouseX, float mouseY);
  SDL_FPoint update_relative(float xrel, float yrel);
  void pan(float xrel, float yrel);

  SDL_FPoint world_to_screen(float worldx, float worldy);
  SDL_FPoint screen_to_world(float screenx, float screeny);

  SDL_FPoint world_to_screen(const SDL_FPoint& worldPoint);
  SDL_FPoint screen_to_world(const SDL_FPoint& screenPoint);

protected:
  SDL_FPoint m_position = {0.0f, 0.0f};
  float m_scale         = 1.0f;

  void rotate_point(SDL_FPoint& point, float angle, const SDL_FPoint& pivot);
};

class CameraSmooth : public Camera {
public:
  CameraSmooth() : Camera() {
  }

  void smooth_zoom(float amount, float mouseX, float mouseY, Uint64 milliseconds) {
    zooming           = true;
    zoom_x            = mouseX;
    zoom_y            = mouseY;
    zoom_ms           = milliseconds;
    zoom_tick         = SDL_GetTicks();
    zoom_elapsed      = 0.0f;
    zoom_scale_per_ms = amount / zoom_ms;
  }

  void cancel_zoom() {
    zooming = false;
  }

  void smooth_pan(float vx, float vy, float damping, Uint64 milliseconds) {
    panning     = true;
    pan_vx      = vx;
    pan_vy      = vy;
    pan_damping = damping;
    pan_ms      = milliseconds;
    pan_tick    = SDL_GetTicks();
    pan_elapsed = 0.0f;
  }

  void cancel_pan() {
    panning = false;
  }

  bool update() {
    if (!zooming && !panning) return false;

    float t = SDL_GetTicks();

    if (zooming) {
      zoom_elapsed += t - zoom_tick;

      zoom(zoom_scale_per_ms, zoom_x, zoom_y);

      if (zoom_elapsed > zoom_ms) {
        zooming = false;
      }

      zoom_tick = t;
    }

    if (panning) {
      pan_elapsed += t - pan_tick;

      pan_vx *= pan_damping;
      pan_vy *= pan_damping;

      // TODO: PANS FOREVER, GIVE AN EPSILON
      if (pan_vx == 0.0f && pan_vy == 0.0f) {
        panning = false;
        return false;
      } else {
        if (pan_elapsed > pan_ms) {
          pan(pan_vx, pan_vy);
          pan_elapsed = 0.0f;
        }
      }

      pan_tick = t;
    }

    return true;
  }

  bool is_zooming() const {
    return zooming;
  }
  bool is_panning() const {
    return panning;
  }
  bool is_running() const {
    return is_panning() || is_zooming();
  }

private:
  bool zooming            = false;
  float zoom_scale_per_ms = 0.0f;
  float zoom_x            = 0.0f;
  float zoom_y            = 0.0f;
  float zoom_tick         = 0.0f;
  float zoom_elapsed      = 0.0f;
  float zoom_ms           = 0.0f;

  bool panning      = false;
  float pan_vx      = 0.0f;
  float pan_vy      = 0.0f;
  float pan_damping = 0.0f;
  float pan_tick    = 0.0f;
  float pan_elapsed = 0.0f;
  float pan_ms      = 0.0f;
};

#endif