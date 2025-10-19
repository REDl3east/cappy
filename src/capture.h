#ifndef _CAPTURE_H_
#define _CAPTURE_H_

#include <stdbool.h>
#include <stdint.h>

typedef struct capture_rgb_t {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} capture_rgb_t;

typedef struct capture_t {
  int width;
  int height;
  capture_rgb_t* pixels;
  int stride;
} capture_t;

bool capture_screen(capture_t* capture);
void capture_free(capture_t* capture);

bool capture_in_bound(capture_t* capture, int x, int y);
bool capture_at(capture_t* capture, int x, int y, capture_rgb_t* rgb);

#endif