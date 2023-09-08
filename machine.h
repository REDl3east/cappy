#ifndef _MACHINE_H_
#define _MACHINE_H_

#include <memory>

#define DEFINE_STATE(state_name, machine_name) class state_name : public State<machine_name>

#define DEFINE_STATE_INNER(state_name, machine_name)                                  \
public:                                                                               \
  static constexpr StateType type = StateType::state_name;                            \
  StateType get_state_type() const override {                                         \
    return type;                                                                      \
  }                                                                                   \
  bool handle_event(std::shared_ptr<machine_name> machine, SDL_Event& event) override; \
  void draw_frame(std::shared_ptr<SDL_Renderer> renderer, Camera& camera) override;                   \
                                                                                      \
private:

enum class StateType {
  MoveState,
  ColorState,
  FlashlightState,
  DrawCrop,
  ShowCrop
};

template <class T>
class State {
public:
  virtual ~State()                                                       = default;
  virtual bool handle_event(std::shared_ptr<T> machine, SDL_Event& event) = 0;
  virtual void draw_frame(std::shared_ptr<SDL_Renderer> renderer, Camera& camera)        = 0;
  virtual StateType get_state_type() const                               = 0;
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

  void draw_frame(std::shared_ptr<SDL_Renderer> renderer, Camera& camera) {
    currentState->draw_frame(renderer, camera);
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
  CappyMachine(Capture& c, std::shared_ptr<SDL_Texture> t) : capture(c), texture(t) {}
  Capture& get_capture() { return capture; }
  std::shared_ptr<SDL_Texture> get_texture() { return texture; }

private:
  Capture& capture;
  std::shared_ptr<SDL_Texture> texture;
};

DEFINE_STATE(MoveState, CappyMachine) {
  DEFINE_STATE_INNER(MoveState, CappyMachine);
};

DEFINE_STATE(ColorState, CappyMachine) {
  DEFINE_STATE_INNER(ColorState, CappyMachine);
};

DEFINE_STATE(FlashlightState, CappyMachine) {
  DEFINE_STATE_INNER(FlashlightState, CappyMachine);
};

DEFINE_STATE(DrawCrop, CappyMachine) {
  DEFINE_STATE_INNER(DrawCrop, CappyMachine);
};

DEFINE_STATE(ShowCrop, CappyMachine) {
  DEFINE_STATE_INNER(ShowCrop, CappyMachine);
};

bool MoveState::handle_event(std::shared_ptr<CappyMachine> machine, SDL_Event& event) {
  return false;
}

void MoveState::draw_frame(std::shared_ptr<SDL_Renderer> renderer, Camera& camera) {
  
}

bool ColorState::handle_event(std::shared_ptr<CappyMachine> machine, SDL_Event& event) {
  return false;
}

void ColorState::draw_frame(std::shared_ptr<SDL_Renderer> renderer, Camera& camera) {
}

bool FlashlightState::handle_event(std::shared_ptr<CappyMachine> machine, SDL_Event& event) {
  return false;
}

void FlashlightState::draw_frame(std::shared_ptr<SDL_Renderer> renderer, Camera& camera) {
}

bool DrawCrop::handle_event(std::shared_ptr<CappyMachine> machine, SDL_Event& event) {
  return false;
}

void DrawCrop::draw_frame(std::shared_ptr<SDL_Renderer> renderer, Camera& camera) {
}

bool ShowCrop::handle_event(std::shared_ptr<CappyMachine> machine, SDL_Event& event) {
  return false;
}

void ShowCrop::draw_frame(std::shared_ptr<SDL_Renderer> renderer, Camera& camera) {
}

#endif