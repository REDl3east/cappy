#include "machine.h"
#include "renderer.h"

#include <cmath>

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

void MoveState::draw_frame(std::shared_ptr<CappyMachine> machine) {
  SDL_SetRenderDrawColor(machine->get_renderer().get(), 125, 125, 125, 255);
  SDL_RenderClear(machine->get_renderer().get());

  Capture& capture                     = machine->get_capture();
  Camera& camera                       = machine->get_camera();
  std::shared_ptr<SDL_Texture> texture = machine->get_texture();

  SDL_FPoint pos = camera.world_to_screen(0, 0);
  SDL_FRect r    = {pos.x, pos.y, (float)capture.width * camera.get_scale(), (float)capture.height * camera.get_scale()};
  SDL_RenderTexture(machine->get_renderer().get(), texture.get(), NULL, &r);

  SDL_RenderPresent(machine->get_renderer().get());
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

void ColorState::draw_frame(std::shared_ptr<CappyMachine> machine) {
  SDL_SetRenderDrawColor(machine->get_renderer().get(), 125, 125, 125, 255);
  SDL_RenderClear(machine->get_renderer().get());

  Capture& capture                     = machine->get_capture();
  Camera& camera                       = machine->get_camera();
  std::shared_ptr<SDL_Texture> texture = machine->get_texture();

  SDL_FPoint pos = camera.world_to_screen(0, 0);
  SDL_FRect r    = {pos.x, pos.y, (float)capture.width * camera.get_scale(), (float)capture.height * camera.get_scale()};
  SDL_RenderTexture(machine->get_renderer().get(), texture.get(), NULL, &r);

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
      text_texture   = std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(machine->get_renderer().get(), text_surface.get()), SDL_DestroyTexture);
      recompute_text = false;
    }

    SDL_FRect rect = {mx + panel_offset, my - panel_size - panel_offset, panel_size, panel_size};

    SDL_SetRenderDrawColor(machine->get_renderer().get(), rgb.r, rgb.g, rgb.b, 255);
    SDL_RenderFillRect(machine->get_renderer().get(), &rect);

    SDL_SetRenderDrawColor(machine->get_renderer().get(), 125, 125, 125, 255);
    SDL_RenderRect(machine->get_renderer().get(), &rect);

    SDL_FRect text_rect = {mx + panel_offset + (0.5f * (panel_size - text_surface->w)), my - panel_size - panel_offset - text_surface->h, (float)text_surface->w, (float)text_surface->h};

    SDL_FRect text_rect_back = {mx + panel_offset, my - panel_size - panel_offset - text_surface->h, panel_size, (float)text_surface->h};
    SDL_SetRenderDrawColor(machine->get_renderer().get(), 125, 125, 125, 255);
    SDL_RenderFillRect(machine->get_renderer().get(), &text_rect_back);

    SDL_SetRenderDrawColor(machine->get_renderer().get(), 125, 125, 125, 255);
    SDL_RenderRect(machine->get_renderer().get(), &text_rect_back);

    SDL_RenderTexture(machine->get_renderer().get(), text_texture.get(), NULL, &text_rect);
  }

  SDL_RenderPresent(machine->get_renderer().get());
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

void FlashlightState::draw_frame(std::shared_ptr<CappyMachine> machine) {
  SDL_SetRenderDrawColor(machine->get_renderer().get(), 125, 125, 125, 255);
  SDL_RenderClear(machine->get_renderer().get());

  Capture& capture                     = machine->get_capture();
  Camera& camera                       = machine->get_camera();
  std::shared_ptr<SDL_Texture> texture = machine->get_texture();

  SDL_FPoint pos = camera.world_to_screen(0, 0);
  SDL_FRect r    = {pos.x, pos.y, (float)capture.width * camera.get_scale(), (float)capture.height * camera.get_scale()};
  SDL_RenderTexture(machine->get_renderer().get(), texture.get(), NULL, &r);

  float x, y;
  SDL_GetMouseState(&x, &y);
  draw_circle_flashlight(machine->get_renderer(), x, y, size, 100,
                         255, 255, 255, 0,
                         255, 255, 255, 0,
                         0, 0, 0, 200);

  SDL_RenderPresent(machine->get_renderer().get());
}

bool DrawCropState::handle_event(std::shared_ptr<CappyMachine> machine, SDL_Event& event) {
  switch (event.type) {
    case SDL_EVENT_KEY_DOWN: {
      SDL_Keycode code = event.key.keysym.sym;
      SDL_Keymod mod   = SDL_GetModState();
      if (!drawing) {
        if (code == SDLK_x) {
          std::cout << "(" << start.x << ", " << start.y << ") ";
          std::cout << "(" << end.x << ", " << end.y << ")\n";
        }
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

        start = machine->get_camera().screen_to_world(start);
        end   = machine->get_camera().screen_to_world(end);

        start = {std::floor(start.x), std::floor(start.y)};
        end   = {std::ceil(end.x), std::ceil(end.y)};

        if (start.x < 0) start.x = 0;
        if (start.x >= machine->get_capture().width) start.x = machine->get_capture().width;
        if (start.y < 0) start.y = 0;
        if (start.y >= machine->get_capture().height) start.y = machine->get_capture().height;
        if (end.x < 0) end.x = 0;
        if (end.x >= machine->get_capture().width) end.x = machine->get_capture().width;
        if (end.y < 0) end.y = 0;
        if (end.y >= machine->get_capture().height) end.y = machine->get_capture().height;

        return true;
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      if (drawing && event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
        start.x += event.motion.xrel;
        start.y += event.motion.yrel;
        return false; // we still want to move the capture.
      }
      break;
    }

    case SDL_EVENT_MOUSE_WHEEL: {
      if (drawing) {
        Camera& camera = machine->get_camera();

        float mx, my;
        SDL_GetMouseState(&mx, &my);

        SDL_FPoint p1 = camera.screen_to_world(start);
        machine->zoom(event.wheel.y > 0, mx, my);
        start = camera.world_to_screen(p1);

        return true;
      }
      break;
    }
  }
  return false;
}

void DrawCropState::draw_frame(std::shared_ptr<CappyMachine> machine) {
  SDL_SetRenderDrawColor(machine->get_renderer().get(), 125, 125, 125, 255);
  SDL_RenderClear(machine->get_renderer().get());

  Capture& capture                     = machine->get_capture();
  Camera& camera                       = machine->get_camera();
  std::shared_ptr<SDL_Texture> texture = machine->get_texture();

  SDL_FPoint pos = camera.world_to_screen(0, 0);
  SDL_FRect r    = {pos.x, pos.y, (float)capture.width * camera.get_scale(), (float)capture.height * camera.get_scale()};
  SDL_RenderTexture(machine->get_renderer().get(), texture.get(), NULL, &r);

  if (drawing) {
    float mx, my;
    SDL_GetMouseState(&mx, &my);
    end = {mx, my};

    float x1 = std::min(start.x, end.x);
    float y1 = std::min(start.y, end.y);
    float x2 = std::max(start.x, end.x);
    float y2 = std::max(start.y, end.y);

    draw_rect_flashlight(machine->get_renderer(), x1, y1, x2 - x1, y2 - y1, 0, 0, 0, 0, 128, 128, 128, 128);
  } else {
    SDL_FPoint start_screen = camera.world_to_screen(start);
    SDL_FPoint end_screen   = camera.world_to_screen(end);
    
    // SDL_SetRenderDrawColor(machine->get_renderer().get(), 0, 0, 0, 255);
    // SDL_RenderLine(machine->get_renderer().get(), start_screen.x, start_screen.y, start_screen.x, end_screen.y);
    // SDL_RenderLine(machine->get_renderer().get(), start_screen.x, start_screen.y, end_screen.x, start_screen.y);
    // SDL_RenderLine(machine->get_renderer().get(), end_screen.x, end_screen.y, start_screen.x, end_screen.y);
    // SDL_RenderLine(machine->get_renderer().get(), end_screen.x, end_screen.y, end_screen.x, start_screen.y);

    draw_rect_flashlight(machine->get_renderer(), start_screen.x, start_screen.y, end_screen.x - start_screen.x, end_screen.y - start_screen.y, 0, 0, 0, 0, 128, 128, 128, 128);
  }

  SDL_RenderPresent(machine->get_renderer().get());
}
