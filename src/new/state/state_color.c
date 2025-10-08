#include "app.h"
#include "state.h"

#include "sb.h"

typedef void (*color_formatter_t)(const capture_rgb_t* rgb, string_builder_t* sb);

static void handle_clipboard(app_t* app, int x, int y, color_formatter_t formatter);
static void to_decimal_string(const capture_rgb_t* rgb, string_builder_t* sb);
static void to_decimal_sep_string(const capture_rgb_t* rgb, string_builder_t* sb);
static void to_hex_string(const capture_rgb_t* rgb, string_builder_t* sb);
static void to_hex_sep_string(const capture_rgb_t* rgb, string_builder_t* sb);
static void to_binary_string(const capture_rgb_t* rgb, string_builder_t* sb);
static void to_binary_sep_string(const capture_rgb_t* rgb, string_builder_t* sb);

bool app_event_color(app_t* app, SDL_Event* event) {
  switch (event->type) {
    case SDL_EVENT_KEY_DOWN: {
      SDL_Keycode code = event->key.key;
      SDL_Keymod mod   = SDL_GetModState();

      float mx, my;
      SDL_GetMouseState(&mx, &my);
      SDL_FPoint mouse = camera_screen_to_world(&app->camera, mx, my);
      int x            = (int)roundf(mouse.x);
      int y            = (int)roundf(mouse.y);

      if (code == SDLK_D && (mod & SDL_KMOD_CTRL) && (mod & SDL_KMOD_LSHIFT)) {
        handle_clipboard(app, x, y, to_decimal_sep_string);
      } else if (code == SDLK_D && (mod & SDL_KMOD_CTRL)) {
        handle_clipboard(app, x, y, to_decimal_string);
      } else if (code == SDLK_H && (mod & SDL_KMOD_CTRL) && (mod & SDL_KMOD_LSHIFT)) {
        handle_clipboard(app, x, y, to_hex_sep_string);
      } else if (code == SDLK_H && (mod & SDL_KMOD_CTRL)) {
        handle_clipboard(app, x, y, to_hex_string);
      } else if (code == SDLK_B && (mod & SDL_KMOD_CTRL) && (mod & SDL_KMOD_LSHIFT)) {
        handle_clipboard(app, x, y, to_binary_sep_string);
      } else if (code == SDLK_B && (mod & SDL_KMOD_CTRL)) {
        handle_clipboard(app, x, y, to_binary_string);
      }
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

static void handle_clipboard(app_t* app, int x, int y, color_formatter_t formatter) {
  capture_rgb_t rgb;
  if (capture_at(&app->capture, x, y, &rgb) && !(x < app->current_x || x > app->current_x + app->current_w - 1 || y < app->current_y || y > app->current_y + app->current_h - 1)) {
    string_builder_t sb = {0};
    formatter(&rgb, &sb);
    sb_append_null(&sb);
    SDL_SetClipboardText(sb.string);
    sb_free(&sb);
  }
}

static void to_decimal_string(const capture_rgb_t* rgb, string_builder_t* sb) {
  int x = (rgb->r << 16) | (rgb->g << 8) | rgb->b;
  sb_appendf(sb, "%d", x);
}

static void to_decimal_sep_string(const capture_rgb_t* rgb, string_builder_t* sb) {
  sb_appendf(sb, "%d, %d, %d", rgb->r, rgb->g, rgb->b);
}

static void to_hex_string(const capture_rgb_t* rgb, string_builder_t* sb) {
  sb_appendf(sb, "0x%02X%02X%02X", rgb->r, rgb->g, rgb->b);
}

static void to_hex_sep_string(const capture_rgb_t* rgb, string_builder_t* sb) {
  sb_appendf(sb, "0x%02X, 0x%02X, 0x%02X", rgb->r, rgb->g, rgb->b);
}

static void to_binary_string(const capture_rgb_t* rgb, string_builder_t* sb) {
  sb_appendf(sb, "0b%08b%08b%08b", rgb->r, rgb->g, rgb->b);
}

static void to_binary_sep_string(const capture_rgb_t* rgb, string_builder_t* sb) {
  sb_appendf(sb, "0x%08b, 0x%08b, 0x%08b", rgb->r, rgb->g, rgb->b);
}
