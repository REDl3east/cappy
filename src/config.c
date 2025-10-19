#include "config.h"
#include "sv.h"

#include "SDL3/SDL.h"

static void config_handler(config_t* config, cstring_view key, cstring_view value);
static void config_parse_color(cstring_view value, uint8_t* color);
static void config_parse_color3(cstring_view value, uint8_t* color);
static void config_parse_bound(cstring_view value, int* bound);

bool config_init(config_t* config) {
  char tmp_buffer[256];
  char* path = SDL_GetPrefPath("", "cappy");
  SDL_snprintf(tmp_buffer, 256, "%s%s", path, "cappy.ini");

  if (!SDL_GetPathInfo(tmp_buffer, NULL)) { // file not exists
    SDL_IOStream* io = SDL_IOFromFile(tmp_buffer, "w");

    if (io == NULL) {
      SDL_free(path);
      return false;
    }

    const char* config_str =
        "window                        = borderless\n"
        "window_pre_crop               = 0 0 0 0\n"
        "flashlight_size               = 150\n"
        "flashlight_center_inner_color = 255 255 204 25\n"
        "flashlight_center_outer_color = 255 255 204 25\n"
        "flashlight_outer_color        = 51 51 0 50\n"
        "background_color              = 50 50 50\n"
        "font_size                     = 36\n"
        "grid_size                     = 100\n"
        "grid_color                    = 200 200 200\n";

    SDL_WriteIO(io, config_str, SDL_strlen(config_str));

    SDL_CloseIO(io);
  }

  bool ret = config_init_file(tmp_buffer, config);
  SDL_free(path);
  return ret;
}

bool config_init_file(const char* file, config_t* config) {
  SDL_Log("Loading config file: '%s'", file);
  config_init_default(config);

  size_t file_size = 0;
  char* file_data  = SDL_LoadFile(file, &file_size);
  if (file_data == NULL) {
    return false;
  }

  cstring_view file_data_sv = sv_create(file_data, (sv_index_t)file_size);

  cstring_view input = file_data_sv;

  cstring_view line;
  sv_index_t pos;

  do {
    char c = sv_find_first_of_switch(input, svl("\n\r"), 0, &pos);
    if (c == '\n') {
      line  = sv_substr(input, 0, pos);
      input = sv_remove_prefix(input, pos + 1);
    } else if (c == '\r') {
      line  = sv_substr(input, 0, pos);
      input = sv_remove_prefix(input, pos + 1);
      if (input.length >= 1 && sv_front(input) == '\n') {
        input = sv_remove_prefix(input, 1);
      }
    } else if (c == '\0') { // not found
      line = input;
    }

    // trim whitespace
    line = sv_consume_until_first_not_of(sv_consume_until_last_not_of(line, svl(" \t\v\f\r")), svl(" \t\v\f\r"));

    if (sv_is_empty(line) || sv_front(line) == ';' || sv_front(line) == '=') continue;

    // remove comment
    sv_index_t comment_pos = sv_find_first_of_char(line, ';', 0);
    if (comment_pos != SV_NPOS) {
      line = sv_substr(line, 0, comment_pos - 1);
      if (sv_is_empty(line)) continue;
      line = sv_consume_until_first_not_of(sv_consume_until_last_not_of(line, svl(" \t\v\f\r")), svl(" \t\v\f\r"));
      if (sv_is_empty(line)) continue;
    }

    sv_index_t equal_pos = sv_find_first_of_char(line, '=', 0);
    if (equal_pos == SV_NPOS) continue;

    cstring_view key = sv_substr(line, 0, equal_pos);
    key              = sv_consume_until_first_not_of(sv_consume_until_last_not_of(key, svl(" \t\v\f\r")), svl(" \t\v\f\r"));

    cstring_view value = sv_substr(line, equal_pos + 1, SV_NPOS);
    value              = sv_consume_until_first_not_of(sv_consume_until_last_not_of(value, svl(" \t\v\f\r")), svl(" \t\v\f\r"));

    config_handler(config, key, value);

  } while (pos != SV_NPOS && !sv_is_empty(input));

  SDL_free(file_data);
  return true;
}

void config_init_default(config_t* config) {
  config->window_fullscreen = false;
  config->flashlight_size   = 150;
  config->grid_size         = 100;
  config->font_size         = 36;

  config->window_pre_crop[0] = 0;
  config->window_pre_crop[1] = 0;
  config->window_pre_crop[2] = 0;
  config->window_pre_crop[3] = 0;

  config->flashlight_center_inner_color[0] = 255;
  config->flashlight_center_inner_color[1] = 255;
  config->flashlight_center_inner_color[2] = 204;
  config->flashlight_center_inner_color[3] = 25;

  config->flashlight_center_outer_color[0] = 255;
  config->flashlight_center_outer_color[1] = 255;
  config->flashlight_center_outer_color[2] = 204;
  config->flashlight_center_outer_color[3] = 25;

  config->flashlight_outer_color[0] = 51;
  config->flashlight_outer_color[1] = 51;
  config->flashlight_outer_color[2] = 0;
  config->flashlight_outer_color[3] = 50;

  config->background_color[0] = 50;
  config->background_color[1] = 50;
  config->background_color[2] = 50;

  config->grid_color[0] = 50;
  config->grid_color[1] = 50;
  config->grid_color[2] = 50;
}

static void config_handler(config_t* config, cstring_view key, cstring_view value) {
  if (sv_compare(key, svl("flashlight_size"))) {
    sv_parse_int(value, &config->flashlight_size);
  } else if (sv_compare(key, svl("flashlight_center_inner_color"))) {
    config_parse_color(value, config->flashlight_center_inner_color);
  } else if (sv_compare(key, svl("flashlight_center_outer_color"))) {
    config_parse_color(value, config->flashlight_center_outer_color);
  } else if (sv_compare(key, svl("flashlight_outer_color"))) {
    config_parse_color(value, config->flashlight_outer_color);
  } else if (sv_compare(key, svl("window"))) {
    if (sv_compare_insensitive(value, svl("fullscreen"))) {
      config->window_fullscreen = true;
    } else if (sv_compare_insensitive(value, svl("borderless"))) {
      config->window_fullscreen = false;
    }
  } else if (sv_compare(key, svl("window_pre_crop"))) {
    config_parse_bound(value, config->window_pre_crop);
  } else if (sv_compare(key, svl("background_color"))) {
    config_parse_color3(value, config->background_color);
  } else if (sv_compare(key, svl("grid_size"))) {
    sv_parse_int(value, &config->grid_size);
  } else if (sv_compare(key, svl("grid_color"))) {
    config_parse_color3(value, config->grid_color);
  } else if (sv_compare(key, svl("font_size"))) {
    sv_parse_int(value, &config->font_size);
  }
}

static void config_parse_color(cstring_view value, uint8_t* color) {
  int index          = 0;
  cstring_view token = sv_empty;
  cstring_view delim = sv_create(" ", 1);
  for (cstring_view sv = sv_split_next(value, delim, &token); !sv_is_empty(sv) || !sv_is_empty(token); sv = sv_split_next(sv, delim, &token)) {
    int value;
    if (!sv_parse_int(token, &value)) return;

    if (value > 255) value = 255;
    if (value < 0) value = 0;

    color[index] = value;

    index++;
    if (index == 4) break;
  }

  // set rest of color values to 255
  for (int i = index; i < 4; i++) {
    color[i] = 255;
  }
}

static void config_parse_color3(cstring_view value, uint8_t* color) {
  int index          = 0;
  cstring_view token = sv_empty;
  cstring_view delim = sv_create(" ", 1);
  for (cstring_view sv = sv_split_next(value, delim, &token); !sv_is_empty(sv) || !sv_is_empty(token); sv = sv_split_next(sv, delim, &token)) {
    int value;
    if (!sv_parse_int(token, &value)) return;

    if (value > 255) value = 255;
    if (value < 0) value = 0;

    color[index] = value;

    index++;
    if (index == 3) break;
  }

  // set rest of color values to 255
  for (int i = index; i < 3; i++) {
    color[i] = 255;
  }
}

static void config_parse_bound(cstring_view value, int* bounds) {
  int index          = 0;
  cstring_view token = sv_empty;
  cstring_view delim = sv_create(" ", 1);
  for (cstring_view sv = sv_split_next(value, delim, &token); !sv_is_empty(sv) || !sv_is_empty(token); sv = sv_split_next(sv, delim, &token)) {
    int value;
    if (!sv_parse_int(token, &value)) return;

    bounds[index] = value;

    index++;
    if (index == 4) break;
  }

  // set rest of bound values to -1
  for (int i = index; i < 4; i++) {
    bounds[i] = -1;
  }
}

void config_print(const config_t* config) {
  if (config == NULL) return;
  SDL_Log("window                        = %s", config->window_fullscreen ? "fullscreen" : "borderless");
  SDL_Log("flashlight_size               = %d", config->flashlight_size);
  SDL_Log("grid_size                     = %d", config->grid_size);
  SDL_Log("font_size                     = %d", config->font_size);
  SDL_Log("window_pre_crop               = %d %d %d %d", config->window_pre_crop[0], config->window_pre_crop[1], config->window_pre_crop[2], config->window_pre_crop[3]);
  SDL_Log("flashlight_center_inner_color = %d %d %d %d", config->flashlight_center_inner_color[0], config->flashlight_center_inner_color[1], config->flashlight_center_inner_color[2], config->flashlight_center_inner_color[3]);
  SDL_Log("flashlight_center_outer_color = %d %d %d %d", config->flashlight_center_outer_color[0], config->flashlight_center_outer_color[1], config->flashlight_center_outer_color[2], config->flashlight_center_outer_color[3]);
  SDL_Log("flashlight_outer_color        = %d %d %d %d", config->flashlight_outer_color[0], config->flashlight_outer_color[1], config->flashlight_outer_color[2], config->flashlight_outer_color[3]);
  SDL_Log("background_color              = %d %d %d", config->background_color[0], config->background_color[1], config->background_color[2]);
  SDL_Log("grid_color                    = %d %d %d", config->grid_color[0], config->grid_color[1], config->grid_color[2]);
}