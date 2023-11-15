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
  CappyMachine(std::shared_ptr<SDL_Renderer> r, Capture& c, std::shared_ptr<SDL_Texture> t, CameraSmooth& cam, TTF_Font* f);
  Capture& get_capture();
  std::shared_ptr<SDL_Renderer>& get_renderer();
  CameraSmooth& get_camera();
  std::shared_ptr<SDL_Texture>& get_texture();
  TTF_Font* get_font();
  void zoom(bool zoom_in, float mousex, float mousey);
  void render_capture();

  int current_x = 0;
  int current_y = 0;
  int current_w = 0;
  int current_h = 0;

private:
  std::shared_ptr<SDL_Renderer> renderer;
  Capture& capture;
  CameraSmooth& camera;
  std::shared_ptr<SDL_Texture> texture;
  TTF_Font* font;

  float zoom_in_factor  = 3.0f;
  Uint64 zoom_in_ms     = 150;
  float zoom_out_factor = 3.0f;
  Uint64 zoom_out_ms    = 100;

  float max_scale = 100.0f;
  float min_scale = 0.25f;
};


#endif 