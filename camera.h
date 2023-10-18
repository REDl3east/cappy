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
    tick    = SDL_GetTicks();
    elapsed = tick;
  }

  void smooth_zoom(float amount, float mouseX, float mouseY, Uint64 milliseconds) {
    zooming       = true;
    tick          = SDL_GetTicks();
    elapsed       = 0.0f;
    ms            = milliseconds;
    amount_per_ms = amount / ms;
    x             = mouseX;
    y             = mouseY;
  }

  bool update() {
    if (!zooming) return false;
    float t = SDL_GetTicks();
    elapsed += t - tick;

    zoom(amount_per_ms, x, y);

    if (elapsed > ms) {
      zooming = false;
    }

    tick = t;
    return true;
  }

  bool is_zooming() {
    return zooming;
  }

private:
  float tick;
  float elapsed;
  float ms;
  float amount_per_ms;
  bool zooming = false;
  float x, y;
};

#endif