#include "moveState.h"

MoveState::MoveState() {
  SDL_ShowCursor();
}

bool MoveState::handle_event(std::shared_ptr<CappyMachine> machine, SDL_Event& event) {
  switch (event.type) {
    case SDL_EVENT_KEY_DOWN: {
      SDL_Keycode code = event.key.keysym.sym;
      SDL_Keymod mod   = SDL_GetModState();
      if (code == SDLK_SPACE) {
      }
    }
  }
  return false;
}

void MoveState::draw_frame(std::shared_ptr<CappyMachine> machine) {
  CameraSmooth& camera = machine->get_camera();
  camera.update();

  machine->render_capture();
}