#include "app.h"
#include "state.h"

bool app_event_color(app_t* app, SDL_Event* event) {
  // TODO:
  // auto handle_clipboard = [this, machine](auto func) {
  //   Capture& capture     = machine->get_capture();
  //   CameraSmooth& camera = machine->get_camera();
  //   float mx, my;
  //   SDL_GetMouseState(&mx, &my);
  //   SDL_FPoint mouse = camera.screen_to_world(mx, my);
  //   mouse.x          = std::round(mouse.x);
  //   mouse.y          = std::round(mouse.y);
  //   RGB rgb;
  //   if (capture.at(mouse.x, mouse.y, rgb) && !(mouse.x < machine->current_x || mouse.x > machine->current_x + machine->current_w - 1 || mouse.y < machine->current_y || mouse.y > machine->current_y + machine->current_h - 1)) {
  //     SDL_SetClipboardText(func(rgb).c_str());
  //   }
  // };

  switch (event->type) {
    case SDL_EVENT_KEY_DOWN: {
      SDL_Keycode code = event->key.key;
      SDL_Keymod mod   = SDL_GetModState();
      if (code == SDLK_C) {
        app->state = APP_MOVE_STATE;
        SDL_ShowCursor();
        return true;
      }
      // TODO:
      // else if (code == SDLK_d && (mod & SDL_KMOD_CTRL) && (mod & SDL_KMOD_LSHIFT)) {
      //   handle_clipboard(toDecimalSepString);
      // } else if (code == SDLK_d && (mod & SDL_KMOD_CTRL)) {
      //   handle_clipboard(toDecimalString);
      // } else if (code == SDLK_h && (mod & SDL_KMOD_CTRL) && (mod & SDL_KMOD_LSHIFT)) {
      //   handle_clipboard(toHexSepString);
      // } else if (code == SDLK_h && (mod & SDL_KMOD_CTRL)) {
      //   handle_clipboard(toHexString);
      // } else if (code == SDLK_b && (mod & SDL_KMOD_CTRL) && (mod & SDL_KMOD_LSHIFT)) {
      //   handle_clipboard(toBinarySepString);
      // } else if (code == SDLK_b && (mod & SDL_KMOD_CTRL)) {
      //   handle_clipboard(toBinaryString);
      // }
      break;
    }
    case SDL_EVENT_MOUSE_MOTION: {
      app->recompute_text = true;
      break;
    }
  }
  return false;
}

static void check_recompute_text(app_t* app, int x, int y, capture_rgb_t rgb) {
  if (app->recompute_text) {
    char text_buf[128];
    SDL_snprintf(text_buf, sizeof(text_buf), "r: %3d g: %3d b: %3d\nx: %d y: %d", rgb.r, rgb.g, rgb.b, x, y);

    SDL_Color white = {255, 255, 255, 255};

    SDL_Surface* text_surface = TTF_RenderText_Solid_Wrapped(app->font, text_buf, 0, white, 0);
    if (text_surface == NULL) {
      SDL_Log("Failed to render text surface: %s", SDL_GetError());
    } else {
      if (app->text_texture != NULL) {
        SDL_DestroyTexture(app->text_texture);
        app->text_texture = NULL;
      }

      SDL_Texture* texture = SDL_CreateTextureFromSurface(app->renderer, text_surface);
      if (texture == NULL) {
        SDL_Log("Failed to create text texture: %s", SDL_GetError());
      } else {
        app->text_texture = texture;
      }

      SDL_DestroySurface(text_surface);
    }

    app->recompute_text = false;
  }
}

void app_iterate_color(app_t* app) {
  camera_update(&app->camera);

  float mx, my;
  SDL_GetMouseState(&mx, &my);
  SDL_FPoint mouse = camera_screen_to_world(&app->camera, mx, my);
  mouse.x          = roundf(mouse.x);
  mouse.y          = roundf(mouse.y);

  capture_rgb_t rgb;
  if (capture_at(&app->capture, (int)mouse.x, (int)mouse.y, &rgb) && !(mouse.x < app->current_x || mouse.x > app->current_x + app->current_w - 1 || mouse.y < app->current_y || mouse.y > app->current_y + app->current_h - 1)) {
    if (app->camera.scale > 7.5f) {
      SDL_FPoint p = camera_world_to_screen(&app->camera, mouse.x, mouse.y);

      float r          = rgb.r / 255.0f;
      float g          = rgb.g / 255.0f;
      float b          = rgb.b / 255.0f;
      float brightness = 0.299f * r + 0.587f * g + 0.114f * b;

      SDL_FRect r1 = {
          p.x,
          p.y,
          app->camera.scale,
          app->camera.scale,
      };

      if (brightness > 0.5f) {
        SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
      } else {
        SDL_SetRenderDrawColor(app->renderer, 255, 255, 255, 255);
      }

      int size = (int)(app->camera.scale / 7.5f);
      for (int i = 0; i < size; i++) {
        SDL_RenderRect(app->renderer, &r1);
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

    check_recompute_text(app, (int)mouse.x, (int)mouse.y, rgb);

    const float panel_width  = 275.0f;
    const float panel_offset = 50.0f;

    SDL_FRect text_panel = {
        mx,
        my - app->text_texture->h - 1,
        panel_width,
        (float)app->text_texture->h,
    };
    text_panel.x += panel_offset;
    text_panel.y -= panel_offset;

    SDL_SetRenderDrawColor(app->renderer, 125, 125, 125, 255);
    SDL_RenderFillRect(app->renderer, &text_panel);
    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_RenderRect(app->renderer, &text_panel);

    SDL_FRect color_panel = {
        mx,
        my - text_panel.w - app->text_texture->h,
        panel_width,
        panel_width,
    };
    color_panel.x += panel_offset;
    color_panel.y -= panel_offset;

    SDL_SetRenderDrawColor(app->renderer, rgb.r, rgb.g, rgb.b, 255);
    SDL_RenderFillRect(app->renderer, &color_panel);

    SDL_SetRenderDrawColor(app->renderer, 0, 0, 0, 255);
    SDL_RenderRect(app->renderer, &color_panel);

    SDL_FRect text_rect = {
        mx + (0.5f * (panel_width - app->text_texture->w)),
        my - app->text_texture->h - 1,
        (float)app->text_texture->w,
        (float)app->text_texture->h,
    };
    text_rect.x += panel_offset;
    text_rect.y -= panel_offset;

    SDL_RenderTexture(app->renderer, app->text_texture, NULL, &text_rect);
  } else {
    SDL_ShowCursor();
  }
}