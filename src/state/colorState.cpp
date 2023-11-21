#include "colorState.h"
#include "moveState.h"

#include <cmath>
#include <format>

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
  machine->render_capture();

  Capture& capture     = machine->get_capture();
  CameraSmooth& camera = machine->get_camera();
  camera.update();

  float mx, my;
  SDL_GetMouseState(&mx, &my);
  SDL_FPoint mouse = camera.screen_to_world(mx, my);
  mouse.x          = std::round(mouse.x);
  mouse.y          = std::round(mouse.y);

  RGB rgb;
  if (capture.at(mouse.x, mouse.y, rgb) && !(mouse.x < machine->current_x || mouse.x > machine->current_x + machine->current_w - 1 || mouse.y < machine->current_y || mouse.y > machine->current_y + machine->current_h - 1)) {
    // TODO: add this key stuff to event handler, it should not check every frame.
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

    if (camera.get_scale() > 7.5f) {
      SDL_FPoint p = camera.world_to_screen(mouse.x, mouse.y);

      float r          = rgb.r / 255.0f;
      float g          = rgb.g / 255.0f;
      float b          = rgb.b / 255.0f;
      float brightness = 0.299 * r + 0.587 * g + 0.114 * b;

      SDL_FRect r1{
          p.x,
          p.y,
          camera.get_scale(),
          camera.get_scale(),
      };

      if (brightness > 0.5f) {
        SDL_SetRenderDrawColor(machine->get_renderer().get(), 0, 0, 0, 255);
      } else {
        SDL_SetRenderDrawColor(machine->get_renderer().get(), 255, 255, 255, 255);
      }

      int size = camera.get_scale() / 7.5;
      for (int i = 0; i < size; i++) {
        SDL_RenderRect(machine->get_renderer().get(), &r1);
        r1.x += 1;
        r1.y += 1;
        r1.w -= 2;
        r1.h -= 2;
      }

      mx = p.x;
      my = p.y;

      SDL_HideCursor();
    } else {
      SDL_ShowCursor();
    }

    if (recompute_text) {
      std::string text = std::format("r: {:3} g: {:3} b: {:3}\nx: {} y: {}", rgb.r, rgb.g, rgb.b, (int)mouse.x, (int)mouse.y);
      text_surface     = std::shared_ptr<SDL_Surface>(TTF_RenderText_Solid_Wrapped(machine->get_font(), text.c_str(), {255, 255, 255, 255}, 0), SDL_DestroySurface);
      text_texture     = std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(machine->get_renderer().get(), text_surface.get()), SDL_DestroyTexture);
      recompute_text   = false;
    }

    SDL_FRect text_panel = {
        mx,
        my - text_surface->h - 1,
        panel_width,
        (float)text_surface->h,
    };
    text_panel.x += panel_offset;
    text_panel.y -= panel_offset;

    SDL_SetRenderDrawColor(machine->get_renderer().get(), 125, 125, 125, 255);
    SDL_RenderFillRect(machine->get_renderer().get(), &text_panel);
    SDL_SetRenderDrawColor(machine->get_renderer().get(), 0, 0, 0, 255);
    SDL_RenderRect(machine->get_renderer().get(), &text_panel);

    SDL_FRect color_panel = {
        mx,
        my - text_panel.w - text_surface->h,
        panel_width,
        panel_width,
    };
    color_panel.x += panel_offset;
    color_panel.y -= panel_offset;

    SDL_SetRenderDrawColor(machine->get_renderer().get(), rgb.r, rgb.g, rgb.b, 255);
    SDL_RenderFillRect(machine->get_renderer().get(), &color_panel);

    SDL_SetRenderDrawColor(machine->get_renderer().get(), 0, 0, 0, 255);
    SDL_RenderRect(machine->get_renderer().get(), &color_panel);

    SDL_FRect text_rect = {
        mx + (0.5f * (panel_width - text_surface->w)),
        my - text_surface->h - 1,
        (float)text_surface->w,
        (float)text_surface->h,
    };
    text_rect.x += panel_offset;
    text_rect.y -= panel_offset;

    SDL_RenderTexture(machine->get_renderer().get(), text_texture.get(), NULL, &text_rect);
  } else {
    SDL_ShowCursor();
  }

  SDL_RenderPresent(machine->get_renderer().get());
}