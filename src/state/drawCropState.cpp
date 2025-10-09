#include "drawCropState.h"
#include "moveState.h"
#include "renderer.h"

#include <cmath>
#include <format>

DrawCropState::DrawCropState(float x, float y) : start_crop({x, y}), end_crop(start_crop) {
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
          machine->current_x = start_crop.x;
          machine->current_y = start_crop.y;
          machine->current_w = end_crop.x - start_crop.x;
          machine->current_h = end_crop.y - start_crop.y;
          machine->set_state<MoveState>();
          return true;
        }
      }
      break;
    }
    case SDL_EVENT_MOUSE_BUTTON_DOWN: {
      if (!drawing) {
        if (event.button.button == SDL_BUTTON_RIGHT) {
          drawing = true;
          SDL_SetCursor(crosshair_cursor.get());

          if (resize_selection == ResizeSelection::N || resize_selection == ResizeSelection::S || resize_selection == ResizeSelection::E || resize_selection == ResizeSelection::W || resize_selection == ResizeSelection::SE) {
            start_crop = machine->get_camera().world_to_screen(start_crop);
            end_crop   = machine->get_camera().world_to_screen(end_crop);
          } else if (resize_selection == ResizeSelection::NW) {
            start_crop = machine->get_camera().world_to_screen(end_crop);
            end_crop   = machine->get_camera().world_to_screen(start_crop);
          } else if (resize_selection == ResizeSelection::NE) {
            start_crop = machine->get_camera().world_to_screen(start_crop.x, end_crop.y);
            end_crop   = machine->get_camera().world_to_screen(end_crop.x, start_crop.y);
          } else if (resize_selection == ResizeSelection::SW) {
            start_crop = machine->get_camera().world_to_screen(end_crop.x, start_crop.y);
            end_crop   = machine->get_camera().world_to_screen(start_crop.x, end_crop.y);
          } else if (resize_selection == ResizeSelection::CENTER) {
            start_crop = machine->get_camera().world_to_screen(start_crop);
            end_crop   = machine->get_camera().world_to_screen(end_crop);
          } else {
            SDL_SetCursor(SDL_GetDefaultCursor());
            drawing = false;
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
        if (start_crop.x == end_crop.x && start_crop.y == end_crop.y) {
          machine->set_state<MoveState>();
          return true;
        }

        drawing = false;

        float x1   = std::min(start_crop.x, end_crop.x);
        float y1   = std::min(start_crop.y, end_crop.y);
        float x2   = std::max(start_crop.x, end_crop.x);
        float y2   = std::max(start_crop.y, end_crop.y);
        start_crop = {x1, y1};
        end_crop   = {x2, y2};

        start_crop = machine->get_camera().screen_to_world(start_crop);
        end_crop   = machine->get_camera().screen_to_world(end_crop);

        start_crop = {std::round(start_crop.x), std::round(start_crop.y)};
        end_crop   = {std::round(end_crop.x), std::round(end_crop.y)};

        if (start_crop.x < machine->current_x) start_crop.x = machine->current_x;
        if (start_crop.x >= machine->current_x + machine->current_w) start_crop.x = machine->current_x + machine->current_w;
        if (start_crop.y < machine->current_y) start_crop.y = machine->current_y;
        if (start_crop.y >= machine->current_y + machine->current_h) start_crop.y = machine->current_y + machine->current_h;
        if (end_crop.x < machine->current_x) end_crop.x = machine->current_x;
        if (end_crop.x >= machine->current_x + machine->current_w) end_crop.x = machine->current_x + machine->current_w;
        if (end_crop.y < machine->current_y) end_crop.y = machine->current_y;
        if (end_crop.y >= machine->current_y + machine->current_h) end_crop.y = machine->current_y + machine->current_h;

        if (end_crop.x - start_crop.x == 0 || end_crop.y - start_crop.y == 0) {
          machine->set_state<MoveState>();
          return true;
        }

        SDL_SetCursor(SDL_GetDefaultCursor());

        return true;
      }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      if (drawing) {
        if (resize_selection != ResizeSelection::CENTER) {
          if (event.motion.state & SDL_BUTTON(SDL_BUTTON_LEFT)) {
            start_crop.x += event.motion.xrel;
            start_crop.y += event.motion.yrel;
            if (resize_selection == ResizeSelection::N || resize_selection == ResizeSelection::W) {
              end_crop.y += event.motion.yrel;
              end_crop.x += event.motion.xrel;
            } else if (resize_selection == ResizeSelection::E) {
              end_crop.y += event.motion.yrel;
            } else if (resize_selection == ResizeSelection::S) {
              end_crop.x += event.motion.xrel;
            }

            return false; // we still want to move the capture.
          }
        } else {
          if (event.motion.state & SDL_BUTTON(SDL_BUTTON_RIGHT)) {
            start_crop.x += event.motion.xrel;
            start_crop.y += event.motion.yrel;
            end_crop.x += event.motion.xrel;
            end_crop.y += event.motion.yrel;
            return false;
          }
        }

      } else {
        SDL_FPoint start_screen = machine->get_camera().world_to_screen(start_crop);
        SDL_FPoint end_screen   = machine->get_camera().world_to_screen(end_crop);

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
  CameraSmooth& camera = machine->get_camera();
  SDL_Renderer* r      = machine->get_renderer().get();

  SDL_FRect crop_rect;

  float mx, my;
  SDL_GetMouseState(&mx, &my);

  float text_padding = 10.0f;

  if (drawing) {
    if (resize_selection != ResizeSelection::CENTER) {
      if (resize_selection == ResizeSelection::N) {
        start_crop = {start_crop.x, my};
      } else if (resize_selection == ResizeSelection::E) {
        end_crop = {mx, end_crop.y};
      } else if (resize_selection == ResizeSelection::S) {
        end_crop = {end_crop.x, my};
      } else if (resize_selection == ResizeSelection::W) {
        start_crop = {mx, start_crop.y};
      } else {
        end_crop = {mx, my};
      }

      SDL_Keymod mod = SDL_GetModState();
      if (mod & SDL_KMOD_SHIFT) {
        if (start_crop.x < end_crop.x && start_crop.y < end_crop.y) { // quad 4
          end_crop.y += (end_crop.x - start_crop.x) - (end_crop.y - start_crop.y);
        } else if (start_crop.x < end_crop.x && start_crop.y >= end_crop.y) { // quad 1
          end_crop.y += (start_crop.x - end_crop.x) - (end_crop.y - start_crop.y);
        } else if (start_crop.x >= end_crop.x && start_crop.y < end_crop.y) { // quad 3
          end_crop.y -= (end_crop.x - start_crop.x) - (start_crop.y - end_crop.y);
        } else if (start_crop.x >= end_crop.x && start_crop.y >= end_crop.y) { // quad 2
          end_crop.y -= (start_crop.x - end_crop.x) - (start_crop.y - end_crop.y);
        }
      }
    }

    SDL_FPoint start_screen = camera.screen_to_world(start_crop);
    SDL_FPoint end_screen   = camera.screen_to_world(end_crop);

    if (camera.update()) {
      start_crop = camera.world_to_screen(start_screen);
      end_crop   = camera.world_to_screen(end_screen);
    }

    float x1 = std::min(start_crop.x, end_crop.x);
    float y1 = std::min(start_crop.y, end_crop.y);
    float x2 = std::max(start_crop.x, end_crop.x);
    float y2 = std::max(start_crop.y, end_crop.y);

    crop_rect.x = x1;
    crop_rect.y = y1;
    crop_rect.w = x2 - x1;
    crop_rect.h = y2 - y1;

  } else {
    SDL_FPoint start_screen = camera.world_to_screen(start_crop);
    SDL_FPoint end_screen   = camera.world_to_screen(end_crop);

    camera.update();

    crop_rect.x = start_screen.x;
    crop_rect.y = start_screen.y;
    crop_rect.w = end_screen.x - start_screen.x;
    crop_rect.h = end_screen.y - start_screen.y;
  }

  draw_rect_flashlight(machine->get_renderer(), crop_rect.x, crop_rect.y, crop_rect.w, crop_rect.h, 0.0f, 0.0f, 0.0f, 0.0f, 0.5f, 0.5f, 0.5f, 0.5f);

  SDL_SetRenderDrawColor(r, 200, 200, 200, 200);
  SDL_RenderLine(r, crop_rect.x, crop_rect.y, crop_rect.x + crop_rect.w, crop_rect.y);
  SDL_RenderLine(r, crop_rect.x, crop_rect.y, crop_rect.x, crop_rect.y + crop_rect.h);
  SDL_RenderLine(r, crop_rect.x + crop_rect.w, crop_rect.y + crop_rect.h, crop_rect.x + crop_rect.w, crop_rect.y);
  SDL_RenderLine(r, crop_rect.x + crop_rect.w, crop_rect.y + crop_rect.h, crop_rect.x, crop_rect.y + crop_rect.h);
}
