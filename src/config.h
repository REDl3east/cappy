#ifndef _CONFIG_H
#define _CONFIG_H

#include <cstdint>
#include <string>

typedef struct cappyConfig {
  bool window_fullscreen                   = false;
  int window_pre_crop[4]                   = {0, 0, 0, 0};
  int flashlight_size                      = 100;
  uint8_t flashlight_center_inner_color[4] = {255, 255, 255, 0};
  uint8_t flashlight_center_outer_color[4] = {255, 255, 255, 0};
  uint8_t flashlight_outer_color[4]        = {0, 0, 0, 255};
  uint8_t background_color[3]              = {50, 50, 50};
  int grid_size                            = 100;
  uint8_t grid_color[3]                    = {50, 50, 50};
} cappyConfig;

void config_init(const std::string& file, cappyConfig& config);
void config_init(cappyConfig& config);

#endif