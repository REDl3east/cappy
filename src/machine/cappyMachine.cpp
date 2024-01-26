#include "cappyMachine.h"
#include "renderer.h"

#include <cmath>
#include <format>

CappyMachine::CappyMachine(cappyConfig& config, std::shared_ptr<SDL_Renderer> r, Capture& c, std::shared_ptr<SDL_Texture> t, CameraSmooth& cam, TTF_Font* f) : config(config), renderer(r), capture(c), texture(t), camera(cam), font(f) {
  current_w = c.width;
  current_h = c.height;
}

Capture& CappyMachine::get_capture() {
  return capture;
}

std::shared_ptr<SDL_Renderer>& CappyMachine::get_renderer() {
  return renderer;
}

CameraSmooth& CappyMachine::get_camera() {
  return camera;
}

std::shared_ptr<SDL_Texture>& CappyMachine::get_texture() {
  return texture;
}

TTF_Font* CappyMachine::get_font() {
  return font;
}

const cappyConfig& CappyMachine::get_config() {
  return config;
}

void CappyMachine::zoom(bool zoom_in, float mousex, float mousey) {
  float scale = camera.get_scale();
  if (zoom_in) {
    if (scale <= max_scale) {
      camera.smooth_zoom(zoom_in_factor, mousex, mousey, zoom_in_ms);
    }
  } else {
    if (scale >= min_scale) {
      camera.smooth_zoom(-1.0f * zoom_out_factor, mousex, mousey, zoom_out_ms);
    }
  }
}

void CappyMachine::render_capture() {
  SDL_FPoint pos = camera.world_to_screen(current_x, current_y);
  SDL_FRect r1   = {(float)current_x, (float)current_y, (float)current_w, (float)current_h};
  SDL_FRect r2   = {pos.x, pos.y, (float)current_w * camera.get_scale(), (float)current_h * camera.get_scale()};
  SDL_RenderTexture(renderer.get(), texture.get(), &r1, &r2);

  if (is_grid_enabled()) {
    render_grid();
  }
}

void CappyMachine::render_clear(uint8_t r, uint8_t g, uint8_t b) {
  SDL_SetRenderDrawColor(get_renderer().get(), r, g, b, 255);
  SDL_RenderClear(get_renderer().get());
}

void CappyMachine::render_grid() {
  int grid_solid_every = 10;

  int x1 = current_x;
  int y1 = current_y;
  int x2 = current_x + current_w;
  int y2 = current_y + current_h;

  SDL_SetRenderDrawColor(renderer.get(), 200, 200, 200, 75);

  // Draw vertical grid lines
  for (int x = x1; x <= x2; ++x) {
    if (x % grid_solid_every == 0) continue;
    SDL_FPoint start = camera.world_to_screen(x, y1);
    SDL_FPoint end   = camera.world_to_screen(x, y2);
    SDL_RenderLine(renderer.get(), start.x, start.y, end.x, end.y);
  }

  // Draw horizontal grid lines
  for (int y = y1; y <= y2; ++y) {
    if (y % grid_solid_every == 0) continue;
    SDL_FPoint start = camera.world_to_screen(x1, y);
    SDL_FPoint end   = camera.world_to_screen(x2, y);
    SDL_RenderLine(renderer.get(), start.x, start.y, end.x, end.y);
  }

  SDL_SetRenderDrawColor(renderer.get(), 200, 200, 200, 150); // Set color to semi-transparent gray

  // Draw solid vertical grid lines
  for (int x = x1; x <= x2; ++x) {
    if (x % grid_solid_every != 0) continue;
    SDL_FPoint start = camera.world_to_screen(x, y1);
    SDL_FPoint end   = camera.world_to_screen(x, y2);
    SDL_RenderLine(renderer.get(), start.x, start.y, end.x, end.y);
  }

  // Draw solid horizontal grid lines
  for (int y = y1; y <= y2; ++y) {
    if (y % grid_solid_every != 0) continue;
    SDL_FPoint start = camera.world_to_screen(x1, y);
    SDL_FPoint end   = camera.world_to_screen(x2, y);
    SDL_RenderLine(renderer.get(), start.x, start.y, end.x, end.y);
  }
}