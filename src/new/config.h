#ifndef _CONFIG_H
#define _CONFIG_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

typedef struct config_t {
  bool window_fullscreen;
  int window_pre_crop[4];
  int flashlight_size;
  uint8_t flashlight_center_inner_color[4];
  uint8_t flashlight_center_outer_color[4];
  uint8_t flashlight_outer_color[4];
  uint8_t background_color[3];
  int grid_size;
  uint8_t grid_color[3];
} config_t;

bool config_init(config_t* config);
bool config_init_file(const char* file, config_t* config);
void config_init_default(config_t* config);
void config_print(const config_t* config);

#endif