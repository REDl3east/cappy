#ifndef _MACHINE_H_
#define _MACHINE_H_

#include <memory>

#include "SDL3_ttf/SDL_ttf.h"
#include "advanced_pixel_7.h"

#include "camera.h"
#include "capture.h"

#define DEFINE_STATE(state_name, machine_name) class state_name : public State<machine_name>

#define DEFINE_STATE_INNER(state_name, machine_name)                                   \
public:                                                                                \
  static constexpr StateType type = StateType::state_name;                             \
  StateType get_state_type() const override {                                          \
    return type;                                                                       \
  }                                                                                    \
  bool handle_event(std::shared_ptr<machine_name> machine, SDL_Event& event) override; \
  void draw_frame(std::shared_ptr<CappyMachine> machine) override;                     \
                                                                                       \
private:

enum class StateType {
  MoveState,
  ColorState,
  FlashlightState,
  DrawCropState,
};

template <class T>
class State {
public:
  virtual ~State()                                                        = default;
  virtual bool handle_event(std::shared_ptr<T> machine, SDL_Event& event) = 0;
  virtual void draw_frame(std::shared_ptr<T> machine)                     = 0;
  virtual StateType get_state_type() const                                = 0;
};

template <class T>
class Machine : public std::enable_shared_from_this<T> {
private:
  std::shared_ptr<State<T>> currentState;

public:
  template <typename S, typename... Args>
  void set_state(Args&&... args) {
    currentState = std::make_shared<S>(std::forward<Args>(args)...);
  }

  bool handle_event(SDL_Event& event) {
    return currentState->handle_event(this->shared_from_this(), event);
  }

  void draw_frame(std::shared_ptr<T> machine) {
    currentState->draw_frame(machine);
  }

  StateType get_current_state_type() const {
    return currentState->get_state_type();
  }

  template <typename S>
  bool is_state_active() const {
    return S::type == get_current_state_type();
  }

  template <typename S>
  std::shared_ptr<S> get_state() {
    if (is_state_active<S>()) {
      return std::dynamic_pointer_cast<S>(currentState);
    }
    return nullptr;
  }
};

class CappyMachine : public Machine<CappyMachine> {
public:
  CappyMachine(std::shared_ptr<SDL_Renderer> r, Capture& c, std::shared_ptr<SDL_Texture> t, CameraSmooth& cam, TTF_Font* f) : renderer(r), capture(c), texture(t), camera(cam), font(f) {
    current_w = c.width;
    current_h = c.height;
  }
  ~CappyMachine() {
  }
  Capture& get_capture() { return capture; }
  std::shared_ptr<SDL_Renderer>& get_renderer() { return renderer; }
  CameraSmooth& get_camera() { return camera; }
  std::shared_ptr<SDL_Texture>& get_texture() { return texture; }
  TTF_Font* get_font() { return font; }

  void zoom(bool zoom_in, float mousex, float mousey) {
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

  void render_capture() {
    SDL_FPoint pos = camera.world_to_screen(current_x, current_y);
    SDL_FRect r1   = {(float)current_x, (float)current_y, (float)current_w, (float)current_h};
    SDL_FRect r2   = {pos.x, pos.y, (float)current_w * camera.get_scale(), (float)current_h * camera.get_scale()};
    SDL_RenderTexture(renderer.get(), texture.get(), &r1, &r2);
  }

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

DEFINE_STATE(MoveState, CappyMachine) {
  DEFINE_STATE_INNER(MoveState, CappyMachine);
};

DEFINE_STATE(ColorState, CappyMachine) {
  DEFINE_STATE_INNER(ColorState, CappyMachine);

public:
  ~ColorState() {
    SDL_ShowCursor();
  }

private:
  float panel_width  = 275.0f;
  float panel_offset = 50.0f;

  std::shared_ptr<SDL_Surface> text_surface;
  std::shared_ptr<SDL_Texture> text_texture;
  bool recompute_text = true;
};

DEFINE_STATE(FlashlightState, CappyMachine) {
  DEFINE_STATE_INNER(FlashlightState, CappyMachine);

public:
  FlashlightState();
  ~FlashlightState();

private:
  float size = 300.0f;
};

enum class ResizeSelection {
  NONE,
  CENTER,
  N,
  E,
  S,
  W,
  NE,
  SE,
  SW,
  NW,
};

DEFINE_STATE(DrawCropState, CappyMachine) {
  DEFINE_STATE_INNER(DrawCropState, CappyMachine);

public:
  DrawCropState(float x, float y) : start({x, y}), end(start) {
    crosshair_cursor = std::shared_ptr<SDL_Cursor>(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR), SDL_DestroyCursor);
    ns_cursor        = std::shared_ptr<SDL_Cursor>(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS), SDL_DestroyCursor);
    ew_cursor        = std::shared_ptr<SDL_Cursor>(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE), SDL_DestroyCursor);
    nwse_cursor      = std::shared_ptr<SDL_Cursor>(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE), SDL_DestroyCursor);
    nesw_cursor      = std::shared_ptr<SDL_Cursor>(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW), SDL_DestroyCursor);

    SDL_SetCursor(crosshair_cursor.get());
  }

private:
  SDL_FPoint start;
  SDL_FPoint end;
  bool drawing                            = true;
  static constexpr float resize_rect_size = 15.0f;
  ResizeSelection resize_selection        = ResizeSelection::NONE;
  std::shared_ptr<SDL_Cursor> crosshair_cursor;
  std::shared_ptr<SDL_Cursor> ns_cursor;
  std::shared_ptr<SDL_Cursor> ew_cursor;
  std::shared_ptr<SDL_Cursor> nwse_cursor;
  std::shared_ptr<SDL_Cursor> nesw_cursor;
  std::shared_ptr<SDL_Cursor> move_cursor;

  std::shared_ptr<SDL_Surface> text_surface;
  std::shared_ptr<SDL_Texture> text_texture;
  bool recompute_text = true;
};

#endif