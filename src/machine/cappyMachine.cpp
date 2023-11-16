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
}
