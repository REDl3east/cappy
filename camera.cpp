#include "camera.h"

#include <cmath>
#include <numbers>

void Camera::set_position(SDL_FPoint position) {
  m_position = position;
}

SDL_FPoint Camera::get_position() const {
  return m_position;
}

void Camera::set_scale(float scale) {
  m_scale = scale;
}

float Camera::get_scale() const {
  return m_scale;
}

void Camera::zoom(float amount, float mousex, float mousey) {
  SDL_FPoint zoomPoint = screen_to_world({mousex, mousey});
  float initX          = zoomPoint.x;
  float initY          = zoomPoint.y;

  m_scale *= (1.0f + amount);

  zoomPoint  = screen_to_world({mousex, mousey});
  float endX = zoomPoint.x;
  float endY = zoomPoint.y;

  m_position.x -= (endX - initX);
  m_position.y -= (endY - initY);
}

SDL_FPoint Camera::update_relative(float xrel, float yrel) {
  return {xrel / m_scale, yrel / m_scale};
}

void Camera::pan(float xrel, float yrel) {
  SDL_FPoint rel = update_relative(xrel, yrel);
  m_position.x -= rel.x;
  m_position.y -= rel.y;
}

SDL_FPoint Camera::world_to_screen(float worldx, float worldy) {
  return {(worldx - m_position.x) * m_scale,
          (worldy - m_position.y) * m_scale};
}
SDL_FPoint Camera::screen_to_world(float screenx, float screeny) {
  return {(screenx / m_scale) + m_position.x,
          (screeny / m_scale) + m_position.y};
}

SDL_FPoint Camera::world_to_screen(const SDL_FPoint& worldPoint) {
  return world_to_screen(worldPoint.x, worldPoint.y);
}

SDL_FPoint Camera::screen_to_world(const SDL_FPoint& screenPoint) {
  return screen_to_world(screenPoint.x, screenPoint.y);
}