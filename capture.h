#ifndef _CAPTURE_H_
#define _CAPTURE_H_

#include <fstream>
#include <iostream>

struct RGB {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

struct Capture {
public:
  ~Capture();

  bool capture();

  bool in_bound(int x, int y) {
    if (!captured) return false;
    if (x >= width || x < 0) return false;
    if (y >= height || y < 0) return false;
    return true;
  }

  bool at(int x, int y, RGB& rgb) {
    if (!captured) return false;
    if (x >= width || x < 0) return false;
    if (y >= height || y < 0) return false;

    int index = y * width + x;
    rgb       = pixels[index];

    return true;
  }

  bool captured = false;
  int width;
  int height;
  int stride;
  RGB* pixels;

private:
};

#endif