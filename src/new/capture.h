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
capture_rgb_t* capture_at(capture_t* capture, int x, int y);

// struct Capture {
// public:
//   ~Capture();

//   bool capture();
//   bool capture(const char* filename);

//   bool in_bound(int x, int y) {
//     if (!captured) return false;
//     if (x >= width || x < 0) return false;
//     if (y >= height || y < 0) return false;
//     return true;
//   }

//   bool at(int x, int y, RGB& rgb) {
//     if (!captured) return false;
//     if (x >= width || x < 0) return false;
//     if (y >= height || y < 0) return false;

//     int index = y * width + x;
//     rgb       = pixels[index];

//     return true;
//   }

//   bool captured = false;
//   int width;
//   int height;
//   int stride;
//   RGB* pixels;

// private:
// };

// std::string toDecimalString(const RGB& color);
// std::string toDecimalSepString(const RGB& color);
// std::string toHexString(const RGB& color);
// std::string toHexSepString(const RGB& color);
// std::string toBinaryString(const RGB& color);
// std::string toBinarySepString(const RGB& color);

#endif