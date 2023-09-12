#ifndef _MACHINE_H_
#define _MACHINE_H_

#include <memory>

std::string toDecimalString(const RGB& color);
std::string toDecimalSepString(const RGB& color);
std::string toHexString(const RGB& color);
std::string toHexSepString(const RGB& color);
std::string toBinaryString(const RGB& color);
std::string toBinarySepString(const RGB& color);

#define DEFINE_STATE(state_name, machine_name) class state_name : public State<machine_name>

#define DEFINE_STATE_INNER(state_name, machine_name)                                                                       \
public:                                                                                                                    \
  static constexpr StateType type = StateType::state_name;                                                                 \
  StateType get_state_type() const override {                                                                              \
    return type;                                                                                                           \
  }                                                                                                                        \
  bool handle_event(std::shared_ptr<machine_name> machine, SDL_Event& event) override;                                     \
  void draw_frame(std::shared_ptr<CappyMachine> machine, std::shared_ptr<SDL_Renderer> renderer, Camera& camera) override; \
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
  virtual ~State()                                                                                            = default;
  virtual bool handle_event(std::shared_ptr<T> machine, SDL_Event& event)                                     = 0;
  virtual void draw_frame(std::shared_ptr<T> machine, std::shared_ptr<SDL_Renderer> renderer, Camera& camera) = 0;
  virtual StateType get_state_type() const                                                                    = 0;
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

  void draw_frame(std::shared_ptr<T> machine, std::shared_ptr<SDL_Renderer> renderer, Camera& camera) {
    currentState->draw_frame(machine, renderer, camera);
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
  CappyMachine(Capture& c, std::shared_ptr<SDL_Texture> t) : capture(c), texture(t) {
    SDL_RWops* font_mem = SDL_RWFromConstMem(advanced_pixel_7, sizeof(advanced_pixel_7));
    if (!font_mem) {
      std::cerr << "Failed to get font from memory\n";
      return;
    }

    font = TTF_OpenFontRW(font_mem, SDL_TRUE, 36);
    if (!font) {
      std::cerr << "Failed to load font: " << TTF_GetError() << '\n';
      return;
    }
  }
  ~CappyMachine() {
  }
  Capture& get_capture() { return capture; }
  std::shared_ptr<SDL_Texture> get_texture() { return texture; }
  TTF_Font* get_font() { return font; }

private:
  Capture& capture;
  std::shared_ptr<SDL_Texture> texture;
  TTF_Font* font;
};

DEFINE_STATE(MoveState, CappyMachine) {
  DEFINE_STATE_INNER(MoveState, CappyMachine);
};

DEFINE_STATE(ColorState, CappyMachine) {
  DEFINE_STATE_INNER(ColorState, CappyMachine);

public:
  ColorState() {
  }

private:
  float panel_size   = 200.0f;
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

DEFINE_STATE(DrawCrop, CappyMachine) {
  DEFINE_STATE_INNER(DrawCrop, CappyMachine);
};

DEFINE_STATE(ShowCrop, CappyMachine) {
  DEFINE_STATE_INNER(ShowCrop, CappyMachine);
};

bool MoveState::handle_event(std::shared_ptr<CappyMachine> machine, SDL_Event& event) {
  switch (event.type) {
    case SDL_EVENT_KEY_DOWN: {
      SDL_Keycode code = event.key.keysym.sym;
      SDL_Keymod mod   = SDL_GetModState();
      if (code == SDLK_SPACE) {
        // machine->set_state<ColorState>();
        // return true;
      }
    }
  }
  return false;
}

void MoveState::draw_frame(std::shared_ptr<CappyMachine> machine, std::shared_ptr<SDL_Renderer> renderer, Camera& camera) {
  SDL_SetRenderDrawColor(renderer.get(), 125, 125, 125, 255);
  SDL_RenderClear(renderer.get());

  Capture& capture                     = machine->get_capture();
  std::shared_ptr<SDL_Texture> texture = machine->get_texture();

  SDL_FPoint pos = camera.world_to_screen(0, 0);
  SDL_FRect r    = {pos.x, pos.y, (float)capture.width * camera.get_scale(), (float)capture.height * camera.get_scale()};
  SDL_RenderTexture(renderer.get(), texture.get(), NULL, &r);

  SDL_RenderPresent(renderer.get());
}

bool ColorState::handle_event(std::shared_ptr<CappyMachine> machine, SDL_Event& event) {
  switch (event.type) {
    case SDL_EVENT_KEY_DOWN: {
      SDL_Keycode code = event.key.keysym.sym;
      SDL_Keymod mod   = SDL_GetModState();
      if (code == SDLK_c) {
        machine->set_state<MoveState>();
        return true;
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      recompute_text = true;
      break;
    }
  }
  return false;
}

void ColorState::draw_frame(std::shared_ptr<CappyMachine> machine, std::shared_ptr<SDL_Renderer> renderer, Camera& camera) {
  SDL_SetRenderDrawColor(renderer.get(), 125, 125, 125, 255);
  SDL_RenderClear(renderer.get());

  Capture& capture                     = machine->get_capture();
  std::shared_ptr<SDL_Texture> texture = machine->get_texture();

  SDL_FPoint pos = camera.world_to_screen(0, 0);
  SDL_FRect r    = {pos.x, pos.y, (float)capture.width * camera.get_scale(), (float)capture.height * camera.get_scale()};
  SDL_RenderTexture(renderer.get(), texture.get(), NULL, &r);

  float mx, my;
  SDL_GetMouseState(&mx, &my);
  SDL_FPoint mouse = camera.screen_to_world(mx, my);

  RGB rgb;
  if (capture.at(mouse.x, mouse.y, rgb)) {
    const Uint8* key_state = SDL_GetKeyboardState(NULL);
    SDL_Keymod mod         = SDL_GetModState();
    if (key_state[SDL_SCANCODE_D] && mod & SDL_KMOD_CTRL) {
      if (mod & SDL_KMOD_SHIFT) {
        SDL_SetClipboardText(toDecimalSepString(rgb).c_str());
      } else {
        SDL_SetClipboardText(toDecimalString(rgb).c_str());
      }
    } else if (key_state[SDL_SCANCODE_H] && mod & SDL_KMOD_CTRL) {
      if (mod & SDL_KMOD_SHIFT) {
        SDL_SetClipboardText(toHexSepString(rgb).c_str());
      } else {
        SDL_SetClipboardText(toHexString(rgb).c_str());
      }
    } else if (key_state[SDL_SCANCODE_B] && mod & SDL_KMOD_CTRL) {
      if (mod & SDL_KMOD_SHIFT) {
        SDL_SetClipboardText(toBinarySepString(rgb).c_str());
      } else {
        SDL_SetClipboardText(toBinaryString(rgb).c_str());
      }
    }

    if (recompute_text) {
      text_surface = std::shared_ptr<SDL_Surface>(TTF_RenderText_Solid(machine->get_font(), toDecimalSepString(rgb).c_str(), {255, 255, 255, 255}), SDL_DestroySurface);
      text_texture = std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(renderer.get(), text_surface.get()), SDL_DestroyTexture);
      recompute_text    = false;
    }

    SDL_FRect rect = {mx + panel_offset, my - panel_size - panel_offset, panel_size, panel_size};

    SDL_SetRenderDrawColor(renderer.get(), rgb.r, rgb.g, rgb.b, 255);
    SDL_RenderFillRect(renderer.get(), &rect);

    SDL_SetRenderDrawColor(renderer.get(), 125, 125, 125, 255);
    SDL_RenderRect(renderer.get(), &rect);

    SDL_FRect text_rect = {mx + panel_offset + (0.5f * (panel_size - text_surface->w)), my - panel_size - panel_offset - text_surface->h, (float)text_surface->w, (float)text_surface->h};

    SDL_FRect text_rect_back = {mx + panel_offset, my - panel_size - panel_offset - text_surface->h, panel_size, (float)text_surface->h};
    SDL_SetRenderDrawColor(renderer.get(), 125, 125, 125, 255);
    SDL_RenderFillRect(renderer.get(), &text_rect_back);

    SDL_SetRenderDrawColor(renderer.get(), 125, 125, 125, 255);
    SDL_RenderRect(renderer.get(), &text_rect_back);

    SDL_RenderTexture(renderer.get(), text_texture.get(), NULL, &text_rect);
  }

  SDL_RenderPresent(renderer.get());
}

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
        size += event.wheel.y > 0 ? -10.0f : 10.f;
        if (size <= 0) size = 0.0f;
        return true;
      }

      break;
    }
  }
  return false;
}

void draw_circle_flashlight(std::shared_ptr<SDL_Renderer> renderer, float x, float y, float radius, int edges, uint8_t cr, uint8_t cg, uint8_t cb, uint8_t ca, uint8_t cor, uint8_t cog, uint8_t cob, uint8_t coa, uint8_t otr, uint8_t otg, uint8_t otb, uint8_t ota);

void FlashlightState::draw_frame(std::shared_ptr<CappyMachine> machine, std::shared_ptr<SDL_Renderer> renderer, Camera& camera) {
  SDL_SetRenderDrawColor(renderer.get(), 125, 125, 125, 255);
  SDL_RenderClear(renderer.get());

  Capture& capture                     = machine->get_capture();
  std::shared_ptr<SDL_Texture> texture = machine->get_texture();

  SDL_FPoint pos = camera.world_to_screen(0, 0);
  SDL_FRect r    = {pos.x, pos.y, (float)capture.width * camera.get_scale(), (float)capture.height * camera.get_scale()};
  SDL_RenderTexture(renderer.get(), texture.get(), NULL, &r);

  float x, y;
  SDL_GetMouseState(&x, &y);
  draw_circle_flashlight(renderer, x, y, size, 100,
                         255, 255, 255, 0,
                         255, 255, 255, 0,
                         0, 0, 0, 200);

  SDL_RenderPresent(renderer.get());
}

bool DrawCrop::handle_event(std::shared_ptr<CappyMachine> machine, SDL_Event& event) {
  return false;
}

void DrawCrop::draw_frame(std::shared_ptr<CappyMachine> machine, std::shared_ptr<SDL_Renderer> renderer, Camera& camera) {
  SDL_SetRenderDrawColor(renderer.get(), 125, 125, 125, 255);
  SDL_RenderClear(renderer.get());

  Capture& capture                     = machine->get_capture();
  std::shared_ptr<SDL_Texture> texture = machine->get_texture();

  SDL_FPoint pos = camera.world_to_screen(0, 0);
  SDL_FRect r    = {pos.x, pos.y, (float)capture.width * camera.get_scale(), (float)capture.height * camera.get_scale()};
  SDL_RenderTexture(renderer.get(), texture.get(), NULL, &r);

  SDL_RenderPresent(renderer.get());
}

bool ShowCrop::handle_event(std::shared_ptr<CappyMachine> machine, SDL_Event& event) {
  return false;
}

void ShowCrop::draw_frame(std::shared_ptr<CappyMachine> machine, std::shared_ptr<SDL_Renderer> renderer, Camera& camera) {
  SDL_SetRenderDrawColor(renderer.get(), 125, 125, 125, 255);
  SDL_RenderClear(renderer.get());

  Capture& capture                     = machine->get_capture();
  std::shared_ptr<SDL_Texture> texture = machine->get_texture();

  SDL_FPoint pos = camera.world_to_screen(0, 0);
  SDL_FRect r    = {pos.x, pos.y, (float)capture.width * camera.get_scale(), (float)capture.height * camera.get_scale()};
  SDL_RenderTexture(renderer.get(), texture.get(), NULL, &r);

  SDL_RenderPresent(renderer.get());
}

std::string toDecimalString(const RGB& color) {
  int x = (color.r << 16) | (color.g << 8) | color.b;
  std::stringstream stream;
  stream << x;
  return stream.str();
}

std::string toDecimalSepString(const RGB& color) {
  std::stringstream stream;
  stream << static_cast<int>(color.r) << ", " << static_cast<int>(color.g) << ", " << static_cast<int>(color.b);
  return stream.str();
}

std::string toHexString(const RGB& color) {
  std::stringstream stream;
  stream << "0x" << std::setfill('0') << std::setw(2) << std::hex;
  stream << static_cast<int>(color.r) << std::setw(2) << static_cast<int>(color.g) << std::setw(2) << static_cast<int>(color.b);
  return stream.str();
}

std::string toHexSepString(const RGB& color) {
  std::stringstream stream;
  stream << "0x" << std::setfill('0') << std::setw(2) << std::hex;
  stream << static_cast<int>(color.r) << std::setw(2) << ", ";
  stream << "0x" << std::setfill('0') << std::setw(2) << std::hex;
  stream << static_cast<int>(color.g) << std::setw(2) << ", ";
  stream << "0x" << std::setfill('0') << std::setw(2) << std::hex;
  stream << static_cast<int>(color.b) << std::setw(2);
  return stream.str();
}

std::string toBinaryString(const RGB& color) {
  return "0b" + std::bitset<8>(color.r).to_string() + std::bitset<8>(color.g).to_string() + std::bitset<8>(color.b).to_string();
}

std::string toBinarySepString(const RGB& color) {
  return "0b" + std::bitset<8>(color.r).to_string() + ", 0b" + std::bitset<8>(color.g).to_string() + ", 0b" + std::bitset<8>(color.b).to_string();
}

SDL_FPoint SDL_PointMid(float x1, float y1, float x2, float y2) {
  return SDL_FPoint{
      (x1 + x2) * 0.5f,
      (y1 + y2) * 0.5f,
  };
}

#endif