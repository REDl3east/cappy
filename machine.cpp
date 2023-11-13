#include "machine.h"
#include "renderer.h"

#include <cmath>
#include <format>

MoveState::MoveState() {
  SDL_ShowCursor();
}

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

  CameraSmooth& camera = machine->get_camera();
  camera.update();

  machine->render_capture();

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
  if (capture.at(mouse.x, mouse.y, rgb) && !(mouse.x < machine->current_x || mouse.x > machine->current_x + machine->current_w || mouse.y < machine->current_y || mouse.y > machine->current_y + machine->current_h)) {
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

      SDL_HideCursor();

      mx = p.x;
      my = p.y;
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
        zoom(event.wheel.y <= 0);
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

  CameraSmooth& camera = machine->get_camera();
  camera.update();

  update();

  machine->render_capture();

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
          machine->current_x = start.x;
          machine->current_y = start.y;
          machine->current_w = end.x - start.x;
          machine->current_h = end.y - start.y;
          machine->set_state<MoveState>();
          return true;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (!drawing) {
        if (event.button.button == SDL_BUTTON_RIGHT) {
          drawing        = true;
          recompute_text = true;
          SDL_SetCursor(crosshair_cursor.get());

          if (resize_selection == ResizeSelection::N || resize_selection == ResizeSelection::S || resize_selection == ResizeSelection::E || resize_selection == ResizeSelection::W || resize_selection == ResizeSelection::SE) {
            start = machine->get_camera().world_to_screen(start);
            end   = machine->get_camera().world_to_screen(end);
          } else if (resize_selection == ResizeSelection::NW) {
            start = machine->get_camera().world_to_screen(end);
            end   = machine->get_camera().world_to_screen(start);
          } else if (resize_selection == ResizeSelection::NE) {
            start = machine->get_camera().world_to_screen(start.x, end.y);
            end   = machine->get_camera().world_to_screen(end.x, start.y);
          } else if (resize_selection == ResizeSelection::SW) {
            start = machine->get_camera().world_to_screen(end.x, start.y);
            end   = machine->get_camera().world_to_screen(start.x, end.y);
          } else if (resize_selection == ResizeSelection::CENTER) {
            start = machine->get_camera().world_to_screen(start);
            end   = machine->get_camera().world_to_screen(end);
          } else {
            SDL_SetCursor(SDL_GetDefaultCursor());
            drawing        = false;
            recompute_text = false;
            return false;
          }

        } else if (event.button.button == SDL_BUTTON_LEFT) {
          return false;
        }
        return true;
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_UP: {
      if (event.button.button == SDL_BUTTON_LEFT) {
        if (SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
          SDL_SetCursor(crosshair_cursor.get());
        }
        return false; // process smooth zoom
      } else if (event.button.button == SDL_BUTTON_RIGHT) {
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

        start = {std::round(start.x), std::round(start.y)};
        end   = {std::round(end.x), std::round(end.y)};

        if (start.x < machine->current_x) start.x = machine->current_x;
        if (start.x >= machine->current_x + machine->current_w) start.x = machine->current_x + machine->current_w;
        if (start.y < machine->current_y) start.y = machine->current_y;
        if (start.y >= machine->current_y + machine->current_h) start.y = machine->current_y + machine->current_h;
        if (end.x < machine->current_x) end.x = machine->current_x;
        if (end.x >= machine->current_x + machine->current_w) end.x = machine->current_x + machine->current_w;
        if (end.y < machine->current_y) end.y = machine->current_y;
        if (end.y >= machine->current_y + machine->current_h) end.y = machine->current_y + machine->current_h;

        if (end.x - start.x == 0 || end.y - start.y == 0) {
          machine->set_state<MoveState>();
          return true;
        }

        recompute_text = true;

        SDL_SetCursor(SDL_GetDefaultCursor());

        return true;
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      if (drawing) {
        if (resize_selection != ResizeSelection::CENTER) {
          if (event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            start.x += event.motion.xrel;
            start.y += event.motion.yrel;
            if (resize_selection == ResizeSelection::N || resize_selection == ResizeSelection::W) {
              end.y += event.motion.yrel;
              end.x += event.motion.xrel;
            } else if (resize_selection == ResizeSelection::E) {
              end.y += event.motion.yrel;
            } else if (resize_selection == ResizeSelection::S) {
              end.x += event.motion.xrel;
            }

            return false; // we still want to move the capture.
          }
        } else {
          if (event.motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
            start.x += event.motion.xrel;
            start.y += event.motion.yrel;
            end.x += event.motion.xrel;
            end.y += event.motion.yrel;
            recompute_text = true;
            return false;
          }
        }

        recompute_text = true;
      } else {
        SDL_FPoint start_screen = machine->get_camera().world_to_screen(start);
        SDL_FPoint end_screen   = machine->get_camera().world_to_screen(end);

        auto getResizeDirection = [](int x, int y, SDL_FPoint start_screen, SDL_FPoint end_screen) -> ResizeSelection {
          auto isPointInRectangle = [x, y](int rx, int ry, int rw, int rh) {
            return (x >= rx && x <= (rx + rw) && y >= ry && y <= (ry + rh));
          };

          float screen_w      = end_screen.x - start_screen.x;
          float screen_h      = end_screen.y - start_screen.y;
          float corner_rect_w = screen_w / 2.0f <= resize_rect_size ? screen_w / 2.0f : resize_rect_size;
          float corner_rect_h = screen_h / 2.0f <= resize_rect_size ? screen_h / 2.0f : resize_rect_size;

          if (screen_h / 2.0f > resize_rect_size) {
            float vertical_rect_w = corner_rect_w;
            float vertical_rect_h = screen_h - 2.0f * corner_rect_h;

            if (isPointInRectangle(start_screen.x, start_screen.y + corner_rect_h, vertical_rect_w, vertical_rect_h)) {
              return ResizeSelection::W;
            } else if (isPointInRectangle(end_screen.x - vertical_rect_w, start_screen.y + corner_rect_h, vertical_rect_w, vertical_rect_h)) {
              return ResizeSelection::E;
            }
          }

          if (screen_w / 2.0f > resize_rect_size) {
            float horizontal_rect_w = screen_w / 2.0f <= resize_rect_size ? 0.0f : screen_w - 2.0f * corner_rect_w;
            float horizontal_rect_h = screen_w / 2.0f <= resize_rect_size ? 0.0f : corner_rect_h;

            if (isPointInRectangle(start_screen.x + corner_rect_w, start_screen.y, horizontal_rect_w, horizontal_rect_h)) {
              return ResizeSelection::N;
            } else if (isPointInRectangle(start_screen.x + corner_rect_w, end_screen.y - corner_rect_h, horizontal_rect_w, horizontal_rect_h)) {
              return ResizeSelection::S;
            }
          }

          if (isPointInRectangle(start_screen.x, start_screen.y, corner_rect_w, corner_rect_h)) {
            return ResizeSelection::NW;
          } else if (isPointInRectangle(start_screen.x, end_screen.y - corner_rect_h, corner_rect_w, corner_rect_h)) {
            return ResizeSelection::SW;
          } else if (isPointInRectangle(end_screen.x - corner_rect_w, end_screen.y - corner_rect_h, corner_rect_w, corner_rect_h)) {
            return ResizeSelection::SE;
          } else if (isPointInRectangle(end_screen.x - corner_rect_w, start_screen.y, corner_rect_w, corner_rect_h)) {
            return ResizeSelection::NE;
          }

          if (screen_h / 2.0f > resize_rect_size && screen_w / 2.0f > resize_rect_size) {
            if (isPointInRectangle(start_screen.x + corner_rect_w, start_screen.y + corner_rect_h, screen_w - 2.0f * corner_rect_w, screen_h - 2.0f * corner_rect_h)) {
              return ResizeSelection::CENTER;
            }
          }

          return ResizeSelection::NONE;
        };

        if (!(event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT))) {
          resize_selection = getResizeDirection(event.motion.x, event.motion.y, start_screen, end_screen);

          if (resize_selection == ResizeSelection::N || resize_selection == ResizeSelection::S) {
            SDL_SetCursor(ns_cursor.get());
          } else if (resize_selection == ResizeSelection::E || resize_selection == ResizeSelection::W) {
            SDL_SetCursor(ew_cursor.get());
          } else if (resize_selection == ResizeSelection::NE || resize_selection == ResizeSelection::SW) {
            SDL_SetCursor(nesw_cursor.get());
          } else if (resize_selection == ResizeSelection::NW || resize_selection == ResizeSelection::SE) {
            SDL_SetCursor(nwse_cursor.get());
          } else {
            SDL_SetCursor(SDL_GetDefaultCursor());
          }
        }

        return false;
      }
      break;
    }

    case SDL_EVENT_MOUSE_WHEEL: {
      break;
    }
  }
  return false;
}

void DrawCropState::draw_frame(std::shared_ptr<CappyMachine> machine) {
  SDL_SetRenderDrawColor(machine->get_renderer().get(), 125, 125, 125, 255);
  SDL_RenderClear(machine->get_renderer().get());

  CameraSmooth& camera = machine->get_camera();
  SDL_Renderer* r      = machine->get_renderer().get();

  machine->render_capture();

  float mx, my;
  SDL_GetMouseState(&mx, &my);

  if (camera.is_panning() && SDL_GetMouseState(NULL, NULL) & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
    recompute_text = true;
  }

  if (drawing) {
    if (resize_selection != ResizeSelection::CENTER) {
      float mx, my;
      SDL_GetMouseState(&mx, &my);

      if (resize_selection == ResizeSelection::N) {
        start = {start.x, my};
      } else if (resize_selection == ResizeSelection::E) {
        end = {mx, end.y};
      } else if (resize_selection == ResizeSelection::S) {
        end = {end.x, my};
      } else if (resize_selection == ResizeSelection::W) {
        start = {mx, start.y};
      } else {
        end = {mx, my};
      }

      SDL_Keymod mod = SDL_GetModState();
      if (mod & SDL_KMOD_SHIFT) {
        if (start.x < end.x && start.y < end.y) { // quad 4
          end.y += (end.x - start.x) - (end.y - start.y);
        } else if (start.x < end.x && start.y >= end.y) { // quad 1
          end.y += (start.x - end.x) - (end.y - start.y);
        } else if (start.x >= end.x && start.y < end.y) { // quad 3
          end.y -= (end.x - start.x) - (start.y - end.y);
        } else if (start.x >= end.x && start.y >= end.y) { // quad 2
          end.y -= (start.x - end.x) - (start.y - end.y);
        }
      }
    }

    SDL_FPoint start_screen = camera.screen_to_world(start);
    SDL_FPoint end_screen   = camera.screen_to_world(end);

    camera.update();

    start = camera.world_to_screen(start_screen);
    end   = camera.world_to_screen(end_screen);

    float width  = end_screen.x - start_screen.x;
    float height = end_screen.y - start_screen.y;

    float x1 = std::min(start.x, end.x);
    float y1 = std::min(start.y, end.y);
    float x2 = std::max(start.x, end.x);
    float y2 = std::max(start.y, end.y);

    draw_rect_flashlight(machine->get_renderer(), x1, y1, x2 - x1, y2 - y1, 0, 0, 0, 0, 128, 128, 128, 128);

    SDL_SetRenderDrawColor(r, 255, 0, 0, 255);
    SDL_RenderLine(r, x1, y1, x2, y1);
    SDL_RenderLine(r, x1, y1, x1, y2);
    SDL_RenderLine(r, x1, y2, x2, y2);
    SDL_RenderLine(r, x2, y1, x2, y2);

    if (recompute_text) {
      float selection_x, selection_y;
      if (resize_selection == ResizeSelection::CENTER) {
        selection_x = start_screen.x;
        selection_y = start_screen.y;
      } else if (width > 0 && height > 0) {
        selection_x = start_screen.x;
        selection_y = start_screen.y;
      } else if (width <= 0 && height > 0) {
        selection_x = start_screen.x + width;
        selection_y = start_screen.y;
      } else if (width > 0 && height <= 0) {
        selection_x = start_screen.x;
        selection_y = start_screen.y + height;
      } else {
        selection_x = start_screen.x + width;
        selection_y = start_screen.y + height;
      }

      text_surface   = std::shared_ptr<SDL_Surface>(TTF_RenderText_Solid_Wrapped(machine->get_font(), std::format("x: {:.2f} y: {:.2f}\nw: {:.2f} h: {:.2f}", selection_x, selection_y, std::abs(width), std::abs(height)).c_str(), {255, 255, 255, 255}, 0), SDL_DestroySurface);
      text_texture   = std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(machine->get_renderer().get(), text_surface.get()), SDL_DestroyTexture);
      recompute_text = false;
    }

    float offset = 15.0f;

    float text_x, text_y;
    if (width == 0 && height == 0 || width > 0 && height > 0) {
      text_x = mx + offset;
      text_y = my + offset;
    } else if (width <= 0 && height > 0) {
      text_x = mx - text_surface->w - offset;
      text_y = my + offset;
    } else if (width > 0 && height <= 0) {
      text_x = mx + offset;
      text_y = my - text_surface->h - offset;
    } else {
      text_x = mx - text_surface->w - offset;
      text_y = my - text_surface->h - offset;
    }

    SDL_FRect text_rect = {text_x, text_y, (float)text_surface->w, (float)text_surface->h};
    SDL_RenderTexture(machine->get_renderer().get(), text_texture.get(), NULL, &text_rect);

  } else {
    SDL_FPoint start_screen = camera.world_to_screen(start);
    SDL_FPoint end_screen   = camera.world_to_screen(end);

    camera.update();

    draw_rect_flashlight(machine->get_renderer(), start_screen.x, start_screen.y, end_screen.x - start_screen.x, end_screen.y - start_screen.y, 0, 0, 0, 0, 128, 128, 128, 128);

    SDL_SetRenderDrawColor(r, 255, 0, 0, 255);
    SDL_RenderLine(r, start_screen.x, start_screen.y, end_screen.x, start_screen.y);
    SDL_RenderLine(r, start_screen.x, start_screen.y, start_screen.x, end_screen.y);
    SDL_RenderLine(r, start_screen.x, end_screen.y, end_screen.x, end_screen.y);
    SDL_RenderLine(r, end_screen.x, start_screen.y, end_screen.x, end_screen.y);

    if (recompute_text) {
      int width  = end.x - start.x;
      int height = end.y - start.y;
      int x      = end.x - width;
      int y      = end.y - height;

      text_surface   = std::shared_ptr<SDL_Surface>(TTF_RenderText_Solid(machine->get_font(), std::format("x: {} y: {} w: {} h: {}", x, y, width, height).c_str(), {255, 255, 255, 255}), SDL_DestroySurface);
      text_texture   = std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(machine->get_renderer().get(), text_surface.get()), SDL_DestroyTexture);
      recompute_text = false;
    }

    int w, h;
    SDL_GetWindowSize(SDL_GetRenderWindow(machine->get_renderer().get()), &w, &h);

    float text_padding = 10.0f;

    float rect_x = w - (float)text_surface->w - 2.0f * text_padding;
    float rect_y = h - (float)text_surface->h;

    SDL_FRect text_rect = {rect_x + text_padding, rect_y, (float)text_surface->w, (float)text_surface->h};

    SDL_FRect r_rect;
    r_rect.x = text_rect.x - text_padding;
    r_rect.y = text_rect.y;
    r_rect.w = text_rect.w + 2.0f * text_padding;
    r_rect.h = text_rect.h;

    SDL_SetRenderDrawColor(machine->get_renderer().get(), 0, 0, 0, 255);
    SDL_RenderFillRect(machine->get_renderer().get(), &r_rect);
    SDL_RenderTexture(machine->get_renderer().get(), text_texture.get(), NULL, &text_rect);
  }

  SDL_RenderPresent(machine->get_renderer().get());
}
