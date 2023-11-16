#include "drawCropState.h"
#include "moveState.h"
#include "renderer.h"

#include <cmath>
#include <format>

DrawCropState::DrawCropState(float x, float y) : start({x, y}), end(start) {
  crosshair_cursor = std::shared_ptr<SDL_Cursor>(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR), SDL_DestroyCursor);
  ns_cursor        = std::shared_ptr<SDL_Cursor>(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENS), SDL_DestroyCursor);
  ew_cursor        = std::shared_ptr<SDL_Cursor>(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZEWE), SDL_DestroyCursor);
  nwse_cursor      = std::shared_ptr<SDL_Cursor>(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENWSE), SDL_DestroyCursor);
  nesw_cursor      = std::shared_ptr<SDL_Cursor>(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_SIZENESW), SDL_DestroyCursor);

  SDL_ShowCursor();
  SDL_SetCursor(crosshair_cursor.get());
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

  float text_padding = 10.0f;

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

    if (camera.update()) {
      start = camera.world_to_screen(start_screen);
      end   = camera.world_to_screen(end_screen);
    }

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

    float offset = 25.0f;

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

    SDL_FRect text_boundry_rect = text_rect;
    text_boundry_rect.x -= text_padding;
    text_boundry_rect.y -= text_padding;
    text_boundry_rect.w += 2.0f * text_padding;
    text_boundry_rect.h += 2.0f * text_padding;

    SDL_SetRenderDrawColor(machine->get_renderer().get(), 125, 125, 125, 255);
    SDL_RenderFillRect(machine->get_renderer().get(), &text_boundry_rect);
    SDL_SetRenderDrawColor(machine->get_renderer().get(), 0, 0, 0, 255);
    SDL_RenderRect(machine->get_renderer().get(), &text_boundry_rect);

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

      text_surface   = std::shared_ptr<SDL_Surface>(TTF_RenderText_Solid_Wrapped(machine->get_font(), std::format("x: {} y: {}\nw: {} h: {}", x, y, width, height).c_str(), {255, 255, 255, 255}, 0), SDL_DestroySurface);
      text_texture   = std::shared_ptr<SDL_Texture>(SDL_CreateTextureFromSurface(machine->get_renderer().get(), text_surface.get()), SDL_DestroyTexture);
      recompute_text = false;
    }

    int w, h;
    SDL_GetWindowSize(SDL_GetRenderWindow(machine->get_renderer().get()), &w, &h);

    float rect_x = w - (float)text_surface->w - 2.0f * text_padding;
    float rect_y = h - (float)text_surface->h;

    SDL_FRect text_rect = {rect_x + text_padding, rect_y, (float)text_surface->w, (float)text_surface->h};

    SDL_FRect text_boundry_rect = text_rect;
    text_boundry_rect.x -= text_padding;
    text_boundry_rect.w += 2.0f * text_padding;

    SDL_SetRenderDrawColor(machine->get_renderer().get(), 125, 125, 125, 255);
    SDL_RenderFillRect(machine->get_renderer().get(), &text_boundry_rect);
    SDL_SetRenderDrawColor(machine->get_renderer().get(), 0, 0, 0, 255);
    SDL_RenderRect(machine->get_renderer().get(), &text_boundry_rect);

    SDL_RenderTexture(machine->get_renderer().get(), text_texture.get(), NULL, &text_rect);
  }

  SDL_RenderPresent(machine->get_renderer().get());
}