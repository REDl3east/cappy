#ifndef _CAPTURE_H_
#define _CAPTURE_H_

#include <X11/Xlib.h>
#include <X11/Xutil.h>
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
  bool write_pnm(const std::string& filename);

private:
  bool captured = false;
  int width;
  int height;
  int stride;
  RGB* pixels;
};




#endif