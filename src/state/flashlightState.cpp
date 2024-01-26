#include "flashlightState.h"
#include "moveState.h"
#include "renderer.h"

FlashlightState::FlashlightState() {
  SDL_HideCursor();
}

FlashlightState::~FlashlightState() {
  SDL_ShowCursor();
}

bool FlashlightState::handle_event(std::shared_ptr<CappyMachine> machine, SDL_Event& event) {
  switch (event.type) {
    case SDL_EVENT_KEY_DOWN: {
      SDL_Keycode code = event.key.keysym.sym;
      SDL_Keymod mod   = SDL_GetModState();
      if (code == SDLK_f) {
        machine->set_state<MoveState>();
        return true;
      }
      break;
    }
    case SDL_EVENT_MOUSE_WHEEL: {
      if ((SDL_GetModState() & SDL_KMOD_LSHIFT)) {
        zoom(event.wheel.y <= 0);
        return true;
      }

      break;
    }
  }
  return false;
}

void FlashlightState::draw_frame(std::shared_ptr<CappyMachine> machine) {
  CameraSmooth& camera = machine->get_camera();
  camera.update();

  update();


  const cappyConfig& config = machine->get_config();
  if (first_pass) {
    size = config.flashlight_size;
  }

  float x, y;
  SDL_GetMouseState(&x, &y);
  draw_circle_flashlight(machine->get_renderer(), x, y, size, 100,
                         config.flashlight_center_inner_color[0],
                         config.flashlight_center_inner_color[1],
                         config.flashlight_center_inner_color[2],
                         config.flashlight_center_inner_color[3],
                         config.flashlight_center_outer_color[0],
                         config.flashlight_center_outer_color[1],
                         config.flashlight_center_outer_color[2],
                         config.flashlight_center_outer_color[3],
                         config.flashlight_outer_color[0],
                         config.flashlight_outer_color[1],
                         config.flashlight_outer_color[2],
                         config.flashlight_outer_color[3]);

  first_pass = false;
}

void FlashlightState::zoom(float in) {
  zooming          = true;
  zoom_in          = in;
  zoom_tick        = SDL_GetTicks();
  zoom_elapsed     = 0.0f;
  zoom_size_per_ms = zoom_amount / zoom_ms;
}

bool FlashlightState::update() {
  if (!zooming) return false;

  float t = SDL_GetTicks();
  zoom_elapsed += t - zoom_tick;

  if (zoom_in) {
    size += zoom_size_per_ms;
  } else {
    size -= zoom_size_per_ms;
  }

  if (size <= 0) {
    size    = 0.0f;
    zooming = false;
  }

  if (zoom_elapsed > zoom_ms) {
    zooming = false;
  }

  zoom_tick = t;

  return true;
}