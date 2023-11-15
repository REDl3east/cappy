#ifndef _MACHINE_H_
#define _MACHINE_H_

#include <memory>

#include "SDL3_ttf/SDL_ttf.h"
#include "advanced_pixel_7.h"

#include "camera.h"
#include "capture.h"

#define DEFINE_STATE(StateName, Machine) class StateName : public State<Machine, Machine::EnumType>

#define DEFINE_STATE_INNER(StateName, Machine)                                    \
public:                                                                           \
  static constexpr Machine::EnumType type = Machine::EnumType::StateName;         \
  Machine::EnumType get_state_type() const override {                             \
    return type;                                                                  \
  }                                                                               \
  bool handle_event(std::shared_ptr<Machine> machine, SDL_Event& event) override; \
  void draw_frame(std::shared_ptr<CappyMachine> machine) override;                \
                                                                                  \
private:

template <class T, class StateEnum>
class State {
public:
  virtual ~State()                                                        = default;
  virtual bool handle_event(std::shared_ptr<T> machine, SDL_Event& event) = 0;
  virtual void draw_frame(std::shared_ptr<T> machine)                     = 0;
  virtual StateEnum get_state_type() const                                = 0;
};

template <class Self, class StateEnum>
class Machine : public std::enable_shared_from_this<Self> {
private:
  std::shared_ptr<State<Self, StateEnum>> currentState;

public:
  using EnumType = StateEnum;

  template <typename S, typename... Args>
  void set_state(Args&&... args) {
    currentState = std::make_shared<S>(std::forward<Args>(args)...);
  }

  bool handle_event(SDL_Event& event) {
    return currentState->handle_event(this->shared_from_this(), event);
  }

  void draw_frame(std::shared_ptr<Self> machine) {
    currentState->draw_frame(machine);
  }

  StateEnum get_current_state_type() const {
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


#endif