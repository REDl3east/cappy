#include "capture.h"

Capture::~Capture() {
  if (captured) delete pixels;
}

bool Capture::capture() {
  if (captured) return false;

  Display* display = XOpenDisplay(NULL);
  if (!display) {
    return false;
  }

  Window root = DefaultRootWindow(display);

  XWindowAttributes attr;
  if (!XGetWindowAttributes(display, root, &attr)) {
    XCloseDisplay(display);
    return false;
  }

  XImage* image = XGetImage(display, root, 0, 0, attr.width, attr.height, AllPlanes, ZPixmap);
  if (!image) {
    XCloseDisplay(display);
    return false;
  }

  width  = attr.width;
  stride = attr.width;
  height = attr.height;
  pixels = new RGB[width * height];

  for (int x = 0; x < width; x++) {
    for (int y = 0; y < height; y++) {
      int index       = y * width + x;
      unsigned long p = XGetPixel(image, x, y);

      pixels[index].r = (p >> 16) & 0xFF;
      pixels[index].g = (p >> 8) & 0xFF;
      pixels[index].b = (p >> 0) & 0xFF;
    }
  }

  XDestroyImage(image);
  XCloseDisplay(display);

  captured = true;

  return true;
}
