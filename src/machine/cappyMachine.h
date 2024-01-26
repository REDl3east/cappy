#ifndef _CAPPY_MACHINE_H
#define _CAPPY_MACHINE_H

#include "machine.h"

enum class StateType {
  MoveState,
  ColorState,
  FlashlightState,
  DrawCropState,
};

class CappyMachine : public Machine<CappyMachine, StateType> {
public:
  CappyMachine(cappyConfig& config, std::shared_ptr<SDL_Renderer> r, Capture& c, std::shared_ptr<SDL_Texture> t, CameraSmooth& cam, TTF_Font* f);
  Capture& get_capture();
  std::shared_ptr<SDL_Renderer>& get_renderer();
  CameraSmooth& get_camera();
  std::shared_ptr<SDL_Texture>& get_texture();
  TTF_Font* get_font();
  const cappyConfig& get_config();
  void zoom(bool zoom_in, float mousex, float mousey);
  void render_capture();
  void render_clear(uint8_t r, uint8_t g, uint8_t b);
  void render_grid();

  bool is_grid_enabled() {
    return grid_enabled;
  }

  void enable_grid() {
    grid_enabled = true;
  }

  void disable_grid() {
    grid_enabled = false;
  }

  void toggle_grid() {
    grid_enabled = !grid_enabled;
  }

  static std::shared_ptr<CappyMachine> make(cappyConfig& config, std::shared_ptr<SDL_Renderer> r, Capture& c, std::shared_ptr<SDL_Texture> t, CameraSmooth& cam, TTF_Font* f) {
    return std::make_shared<CappyMachine>(config, r, c, t, cam, f);
  }

  int current_x = 0;
  int current_y = 0;
  int current_w = 0;
  int current_h = 0;

private:
  std::shared_ptr<SDL_Renderer> renderer;
  Capture& capture;
  CameraSmooth& camera;
  cappyConfig& config;
  std::shared_ptr<SDL_Texture> texture;
  TTF_Font* font;

  float zoom_in_factor  = 3.0f;
  Uint64 zoom_in_ms     = 150;
  float zoom_out_factor = 3.0f;
  Uint64 zoom_out_ms    = 100;

  float max_scale = 100.0f;
  float min_scale = 0.25f;

  bool grid_enabled = false;
};

#endif