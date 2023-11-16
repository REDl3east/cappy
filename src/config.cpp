#include "config.h"

#define SV_IMPLEMENTATION
#include "sv.h"

void config_parse_color(string_view value, uint8_t* color);

void config_handler(cappyConfig& config, string_view key, string_view value) {
  if (sv_compare(key, sv("flashlight_size"))) {
    sv_parse_int(value, &config.flashlight_size);
  } else if (sv_compare(key, sv("flashlight_center_inner_color"))) {
    config_parse_color(value, config.flashlight_center_inner_color);
  } else if (sv_compare(key, sv("flashlight_center_outer_color"))) {
    config_parse_color(value, config.flashlight_center_outer_color);
  } else if (sv_compare(key, sv("flashlight_outer_color"))) {
    config_parse_color(value, config.flashlight_outer_color);
  }
}

void config_init(const char* file, cappyConfig& config) {
  string_view file_data;
  if (!sv_read_file(file, &file_data)) {
    return;
  }

  string_view input = file_data;

  string_view line;
  sv_index_t pos;

  do {
    char c = sv_find_first_of_switch(input, sv("\n\r"), 0, &pos);
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
    line = sv_consume_until_first_not_of(sv_consume_until_last_not_of(line, sv(" \t\v\f\r")), sv(" \t\v\f\r"));

    if (sv_is_empty(line) || sv_front(line) == ';' || sv_front(line) == '=') continue;

    // remove comment
    sv_index_t comment_pos = sv_find_first_of_char(line, ';', 0);
    if (comment_pos != SV_NPOS) {
      line = sv_substr(line, 0, comment_pos - 1);
      if (sv_is_empty(line)) continue;
      line = sv_consume_until_first_not_of(sv_consume_until_last_not_of(line, sv(" \t\v\f\r")), sv(" \t\v\f\r"));
      if (sv_is_empty(line)) continue;
    }

    sv_index_t equal_pos = sv_find_first_of_char(line, '=', 0);
    if (equal_pos == SV_NPOS) continue;

    string_view key = sv_substr(line, 0, equal_pos);
    key             = sv_consume_until_first_not_of(sv_consume_until_last_not_of(key, sv(" \t\v\f\r")), sv(" \t\v\f\r"));

    string_view value = sv_substr(line, equal_pos + 1, SV_NPOS);
    value             = sv_consume_until_first_not_of(sv_consume_until_last_not_of(value, sv(" \t\v\f\r")), sv(" \t\v\f\r"));

    config_handler(config, key, value);

  } while (pos != SV_NPOS && !sv_is_empty(input));

  sv_read_file_free(file_data);
}

void config_parse_color(string_view value, uint8_t* color) {
  int index = 0;
  SV_FOR_SPLIT(token, value, sv(" ")) {
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