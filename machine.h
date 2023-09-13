#ifndef _MACHINE_H_
#define _MACHINE_H_

#include <memory>

std::string toDecimalString(const RGB& color);
std::string toDecimalSepString(const RGB& color);
std::string toHexString(const RGB& color);
std::string toHexSepString(const RGB& color);
std::string toBinaryString(const RGB& color);
std::string toBinarySepString(const RGB& color);

SDL_FPoint SDL_PointMid(float x1, float y1, float x2, float y2);
SDL_FPoint SDL_PointMid(const SDL_FPoint& p1, const SDL_FPoint& p2);
void draw_circle_flashlight(std::shared_ptr<SDL_Renderer> renderer, float x, float y, float radius, int edges, uint8_t cr, uint8_t cg, uint8_t cb, uint8_t ca, uint8_t cor, uint8_t cog, uint8_t cob, uint8_t coa, uint8_t otr, uint8_t otg, uint8_t otb, uint8_t ota);
void draw_rect_flashlight(std::shared_ptr<SDL_Renderer> renderer, float x, float y, float w, float h, uint8_t inr, uint8_t ing, uint8_t inb, uint8_t ina, uint8_t outr, uint8_t outg, uint8_t outb, uint8_t outa);

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
  DrawCropState,
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
    font = TTF_OpenFontRW(SDL_RWFromConstMem(advanced_pixel_7, sizeof(advanced_pixel_7)), SDL_TRUE, 36);
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

DEFINE_STATE(DrawCropState, CappyMachine) {
  DEFINE_STATE_INNER(DrawCropState, CappyMachine);

public:
  DrawCropState(float x, float y) : start({x, y}), end(start) {}

private:
  SDL_FPoint start;
  SDL_FPoint end;
  bool drawing = true;
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
      text_surface   = std::shared_ptr<SDL_Surface>(TTF_RenderText_Solid(machine->get_font(), toDecimalSepString(rgb).c_str(), {255, 255, 255, 255}), SDL_DestroySurface);
      text_texture   = std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(renderer.get(), text_surface.get()), SDL_DestroyTexture);
      recompute_text = false;
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

bool DrawCropState::handle_event(std::shared_ptr<CappyMachine> machine, SDL_Event& event) {
  switch (event.type) {
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (drawing && event.button.button == SDL_BUTTON_LEFT) return true; // we ignore left down when drawing

      if (event.button.button == SDL_BUTTON_LEFT) {
        machine->set_state<MoveState>();
        return false; // we want to icon to change in main event loop
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event.button.button == SDL_BUTTON_RIGHT) {
        if (start.x == end.x && start.y == end.y) {
          machine->set_state<MoveState>();
          return true;
        }

        drawing = false;

        float x1 = std::min(start.x, end.x);
        float y1 = std::min(start.y, end.y);
        float x2 = std::max(start.x, end.x);
        float y2 = std::max(start.y, end.y);
        start    = {x1, y1};
        end      = {x2, y2};

        return true;
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      if (drawing && (event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT))) return true; // we ignore left motion when drawing
      break;
    }

    case SDL_EVENT_MOUSE_WHEEL: {
      if (drawing) return true; // we ignore scroll wheel when drawing
      break;
    }
  }
  return false;
}

void DrawCropState::draw_frame(std::shared_ptr<CappyMachine> machine, std::shared_ptr<SDL_Renderer> renderer, Camera& camera) {
  SDL_SetRenderDrawColor(renderer.get(), 255, 125, 125, 255);
  SDL_RenderClear(renderer.get());

  Capture& capture                     = machine->get_capture();
  std::shared_ptr<SDL_Texture> texture = machine->get_texture();

  SDL_FPoint pos = camera.world_to_screen(0, 0);
  SDL_FRect r    = {pos.x, pos.y, (float)capture.width * camera.get_scale(), (float)capture.height * camera.get_scale()};
  SDL_RenderTexture(renderer.get(), texture.get(), NULL, &r);

  if (drawing) {
    float mx, my;
    SDL_GetMouseState(&mx, &my);
    end = {mx, my};

    float x1 = std::min(start.x, end.x);
    float y1 = std::min(start.y, end.y);
    float x2 = std::max(start.x, end.x);
    float y2 = std::max(start.y, end.y);

    draw_rect_flashlight(renderer, x1, y1, x2 - x1, y2 - y1, 0, 0, 0, 0, 128, 128, 128, 128);
  } else {
    draw_rect_flashlight(renderer, start.x, start.y, end.x - start.x, end.y - start.y, 0, 0, 0, 0, 128, 128, 128, 128);
  }

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

SDL_FPoint SDL_PointMid(const SDL_FPoint& p1, const SDL_FPoint& p2) {
  return SDL_PointMid(p1.x, p1.y, p2.x, p2.y);
}

void drawTriangle(std::shared_ptr<SDL_Renderer> renderer, float x1, float y1, float x2, float y2, float x3, float y3, Uint8 r, Uint8 g, Uint8 b, Uint8 a) {
  SDL_Vertex v[3] = {0};
  v[0].position   = {x1, y1};
  v[1].position   = {x2, y2};
  v[2].position   = {x3, y3};
  v[0].color      = {r, g, b, a};
  v[1].color      = {r, g, b, a};
  v[2].color      = {r, g, b, a};
  SDL_RenderGeometry(renderer.get(), nullptr, v, 3, nullptr, 0);
}

void drawTriangle(std::shared_ptr<SDL_Renderer> renderer,
                  float x1, float y1, float x2, float y2, float x3, float y3,
                  Uint8 r1, Uint8 g1, Uint8 b1, Uint8 a1,
                  Uint8 r2, Uint8 g2, Uint8 b2, Uint8 a2,
                  Uint8 r3, Uint8 g3, Uint8 b3, Uint8 a3) {
  SDL_Vertex v[3] = {0};
  v[0].position   = {x1, y1};
  v[1].position   = {x2, y2};
  v[2].position   = {x3, y3};
  v[0].color      = {r1, g1, b1, a1};
  v[1].color      = {r2, g2, b2, a2};
  v[2].color      = {r3, g3, b3, a3};
  SDL_RenderGeometry(renderer.get(), nullptr, v, 3, nullptr, 0);
}

void draw_circle_flashlight(std::shared_ptr<SDL_Renderer> renderer, float x, float y, float radius, int edges,
                            uint8_t cr, uint8_t cg, uint8_t cb, uint8_t ca,
                            uint8_t cor, uint8_t cog, uint8_t cob, uint8_t coa,
                            uint8_t otr, uint8_t otg, uint8_t otb, uint8_t ota) {
  if (edges < 5) { // wont work with edges less than 5
    return;
  }

  int width, height;
  SDL_GetCurrentRenderOutputSize(renderer.get(), &width, &height);

  radius = std::abs(radius);

  SDL_FPoint world_bound[4] = {
      {0, 0},
      {(float)width, 0},
      {(float)width, (float)height},
      {0, (float)height},
  };

  SDL_FPoint world_bound_mid[4] = {
      SDL_PointMid(world_bound[0], world_bound[1]),
      SDL_PointMid(world_bound[1], world_bound[2]),
      SDL_PointMid(world_bound[2], world_bound[3]),
      SDL_PointMid(world_bound[3], world_bound[0]),
  };

  SDL_FPoint bounds[4] = {
      {x - radius, y - radius},
      {x + radius, y - radius},
      {x + radius, y + radius},
      {x - radius, y + radius},
  };

  SDL_FPoint bound_mids[4] = {
      SDL_PointMid(bounds[0], bounds[1]),
      SDL_PointMid(bounds[1], bounds[2]),
      SDL_PointMid(bounds[2], bounds[3]),
      SDL_PointMid(bounds[3], bounds[0]),
  };

  std::vector<SDL_Vertex> v;

  // LIGHT

  for (int i = 0; i < 4; i++) {
    drawTriangle(renderer,
                 world_bound[i].x, world_bound[i].y,
                 world_bound[(i + 1) % 4].x, world_bound[(i + 1) % 4].y,
                 bound_mids[i].x, bound_mids[i].y,
                 otr, otg, otb, ota);
  }

  float a0 = 2 * std::numbers::pi_v<float> / edges;
  float r1 = std::numbers::pi_v<float> * 0.5f + (std::numbers::pi_v<float> / edges);
  for (int i = 0; i < edges; ++i) {
    float a1 = std::fmod((a0 * i + r1), 2.0f * std::numbers::pi_v<float>);
    float a2 = std::fmod((a0 * (i + 1) + r1), 2.0f * std::numbers::pi_v<float>);
    if (a1 < 0) a1 += 2.0f * std::numbers::pi_v<float>;
    if (a2 < 0) a2 += 2.0f * std::numbers::pi_v<float>;

    float x1 = x + radius * std::cos(a1);
    float y1 = y + radius * std::sin(a1);
    float x2 = x + radius * std::cos(a2);
    float y2 = y + radius * std::sin(a2);

    // OUTSIDE LIGHT IN BOUNDS

    int index         = -1;
    bool in_same_quad = false;

    if (a1 < std::numbers::pi_v<float> / 2) {
      in_same_quad = a2 < std::numbers::pi_v<float> / 2;
      index        = 0;
    } else if (a1 < std::numbers::pi_v<float>) {
      in_same_quad = a2 < std::numbers::pi_v<float>;
      index        = 1;
    } else if (a1 < 3 * std::numbers::pi_v<float> / 2) {
      in_same_quad = a2 < 3 * std::numbers::pi_v<float> / 2;
      index        = 2;
    } else {
      in_same_quad = a2 >= std::numbers::pi_v<float> / 2;
      index        = 3;
    }

    if (in_same_quad) {
      drawTriangle(renderer,
                   x1, y1,
                   x2, y2,
                   world_bound[(index + 2) % 4].x, world_bound[(index + 2) % 4].y,
                   otr, otg, otb, ota);
    } else {
      drawTriangle(renderer,
                   x1, y1,
                   bound_mids[(index + 2) % 4].x, bound_mids[(index + 2) % 4].y,
                   world_bound[(index + 2) % 4].x, world_bound[(index + 2) % 4].y,
                   otr, otg, otb, ota);

      drawTriangle(renderer,
                   x1, y1,
                   x2, y2,
                   bound_mids[(index + 2) % 4].x, bound_mids[(index + 2) % 4].y,
                   otr, otg, otb, ota);

      drawTriangle(renderer,
                   x2, y2,
                   bound_mids[(index + 2) % 4].x, bound_mids[(index + 2) % 4].y,
                   world_bound[(index + 3) % 4].x, world_bound[(index + 3) % 4].y,
                   otr, otg, otb, ota);
    }

    // LIGHT
    drawTriangle(renderer,
                 x, y,
                 x1, y1,
                 x2, y2,
                 cr, cg, cb, ca,
                 cor, cog, cob, coa,
                 cor, cog, cob, coa);
  }
}

void draw_rect_flashlight(std::shared_ptr<SDL_Renderer> renderer, float x, float y, float w, float h, uint8_t inr, uint8_t ing, uint8_t inb, uint8_t ina, uint8_t outr, uint8_t outg, uint8_t outb, uint8_t outa) {
  SDL_FPoint rect_bounds[4] = {
      {x, y},
      {x + w, y},
      {x + w, y + h},
      {x, y + h},
  };

  int width, height;
  SDL_GetCurrentRenderOutputSize(renderer.get(), &width, &height);

  drawTriangle(renderer,
               rect_bounds[0].x, rect_bounds[0].y,
               rect_bounds[1].x, rect_bounds[1].y,
               rect_bounds[2].x, rect_bounds[2].y,
               inr, ing, inb, ina);
  drawTriangle(renderer,
               rect_bounds[0].x, rect_bounds[0].y,
               rect_bounds[2].x, rect_bounds[2].y,
               rect_bounds[3].x, rect_bounds[3].y,
               inr, ing, inb, ina);

  drawTriangle(renderer,
               0.0f, 0.0f,
               width, 0.0f,
               width, rect_bounds[0].y,
               outr, outg, outb, outa);
  drawTriangle(renderer,
               0.0f, 0.0f,
               0.0f, rect_bounds[0].y,
               width, rect_bounds[0].y,
               outr, outg, outb, outa);

  drawTriangle(renderer,
               width, height,
               width, rect_bounds[2].y,
               0.0f, rect_bounds[2].y,
               outr, outg, outb, outa);
  drawTriangle(renderer,
               width, height,
               0.0f, height,
               0.0f, rect_bounds[2].y,
               outr, outg, outb, outa);

  drawTriangle(renderer,
               0.0f, rect_bounds[0].y,
               rect_bounds[0].x, rect_bounds[0].y,
               rect_bounds[3].x, rect_bounds[3].y,
               outr, outg, outb, outa);
  drawTriangle(renderer,
               0.0f, rect_bounds[0].y,
               0.0f, rect_bounds[3].y,
               rect_bounds[3].x, rect_bounds[3].y,
               outr, outg, outb, outa);

  drawTriangle(renderer,
               rect_bounds[1].x, rect_bounds[1].y,
               width, rect_bounds[1].y,
               width, rect_bounds[2].y,
               outr, outg, outb, outa);
  drawTriangle(renderer,
               rect_bounds[1].x, rect_bounds[1].y,
               rect_bounds[2].x, rect_bounds[2].y,
               width, rect_bounds[2].y,
               outr, outg, outb, outa);
}

#endif