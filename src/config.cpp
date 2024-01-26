#include "config.h"

#define SV_IMPLEMENTATION
#include "sv.h"

#include "SDL3/SDL_log.h"
#include "cfgpath.h"

#include <filesystem>
#include <fstream>
#include <iostream>

void config_parse_color(string_view value, uint8_t* color);
void config_parse_color3(string_view value, uint8_t* color);
void config_parse_bound(string_view value, int* bound);

void config_handler(cappyConfig& config, string_view key, string_view value) {
  if (sv_compare(key, svl("flashlight_size"))) {
    sv_parse_int(value, &config.flashlight_size);
  } else if (sv_compare(key, svl("flashlight_center_inner_color"))) {
    config_parse_color(value, config.flashlight_center_inner_color);
  } else if (sv_compare(key, svl("flashlight_center_outer_color"))) {
    config_parse_color(value, config.flashlight_center_outer_color);
  } else if (sv_compare(key, svl("flashlight_outer_color"))) {
    config_parse_color(value, config.flashlight_outer_color);
  } else if (sv_compare(key, svl("window_fullscreen"))) {
    if (sv_compare_insensitive(value, svl("true")) || sv_compare(value, svl("1"))) {
      config.window_fullscreen = true;
    } else if (sv_compare_insensitive(value, svl("false")) || sv_compare(value, svl("0"))) {
      config.window_fullscreen = false;
    }
  } else if (sv_compare(key, svl("window_pre_crop"))) {
    config_parse_bound(value, config.window_pre_crop);
  } else if (sv_compare(key, svl("background_color"))) {
    config_parse_color3(value, config.background_color);
  } else if (sv_compare(key, svl("grid_size"))) {
    sv_parse_int(value, &config.grid_size);
  } else if (sv_compare(key, svl("grid_color"))) {
    config_parse_color3(value, config.grid_color);
  }
}

void config_init(const std::string& file, cappyConfig& config) {
  SDL_Log("Reading config file: '%s'", file.c_str());

  string_view file_data;
  if (!sv_read_file(file.c_str(), &file_data)) {
    SDL_Log("Failed to read config file: '%s'", file.c_str());
    return;
  }

  string_view input = file_data;

  string_view line;
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

    string_view key = sv_substr(line, 0, equal_pos);
    key             = sv_consume_until_first_not_of(sv_consume_until_last_not_of(key, svl(" \t\v\f\r")), svl(" \t\v\f\r"));

    string_view value = sv_substr(line, equal_pos + 1, SV_NPOS);
    value             = sv_consume_until_first_not_of(sv_consume_until_last_not_of(value, svl(" \t\v\f\r")), svl(" \t\v\f\r"));

    config_handler(config, key, value);

  } while (pos != SV_NPOS && !sv_is_empty(input));

  sv_read_file_free(file_data);
}

void config_init(cappyConfig& config) {
  char path[MAX_PATH];
  get_user_config_folder(path, MAX_PATH, "cappy");

  const std::filesystem::path config_path = std::string(path) + "cappy.ini";

  if (!std::filesystem::exists(config_path)) {
    std::ofstream file(config_path);
    file << "window_fullscreen             = false\n"
            "window_pre_crop               = 0 0 0 0\n"
            "flashlight_size               = 150\n"
            "flashlight_center_inner_color = 255 255 204 25\n"
            "flashlight_center_outer_color = 255 255 204 25\n"
            "flashlight_outer_color        = 51 51 0 50\n"
            "background_color              = 50 50 50\n"
            "grid_size                     = 100\n"
            "grid_color                    = 200 200 200\n";
    file.close();
  }

  config_init(config_path.string(), config);
}

void config_parse_color(string_view value, uint8_t* color) {
  int index = 0;
  SV_FOR_SPLIT(token, value, svl(" ")) {
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

void config_parse_color3(string_view value, uint8_t* color) {
  int index = 0;
  SV_FOR_SPLIT(token, value, svl(" ")) {
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

void config_parse_bound(string_view value, int* bounds) {
  int index = 0;
  SV_FOR_SPLIT(token, value, svl(" ")) {
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