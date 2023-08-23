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

private:
  SDL_FPoint m_position = {0.0f, 0.0f};
  float m_scale         = 1.0f;

  void rotate_point(SDL_FPoint& point, float angle, const SDL_FPoint& pivot);
};

#endif