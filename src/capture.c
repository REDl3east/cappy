#include "capture.h"

#include <stdlib.h>

#if __linux__
  #include <X11/Xlib.h>
  #include <X11/Xutil.h>
#elif _WIN32
  #include <windows.h>
#endif

#if __linux__
bool capture_screen_x11(capture_t* capture);
#elif _WIN32
bool capture_screen_windows(capture_t* capture);
#endif

bool capture_screen(capture_t* capture) {
#if __linux__
  return capture_screen_x11(capture);
#elif _WIN32
  return capture_screen_windows(capture);
#else
  return false;
#endif
}

#if __linux__

bool capture_screen_x11(capture_t* capture) {
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

  capture->width  = attr.width;
  capture->stride = attr.width;
  capture->height = attr.height;
  capture->pixels = (capture_rgb_t*)calloc(capture->width * capture->height, sizeof(capture_rgb_t));

  for (int x = 0; x < capture->width; x++) {
    for (int y = 0; y < capture->height; y++) {
      int index       = y * capture->width + x;
      unsigned long p = XGetPixel(image, x, y);

      capture->pixels[index].r = (p >> 16) & 0xFF;
      capture->pixels[index].g = (p >> 8) & 0xFF;
      capture->pixels[index].b = (p >> 0) & 0xFF;
    }
  }

  XDestroyImage(image);
  XCloseDisplay(display);

  return true;
}

#elif _WIN32

bool capture_screen_windows(capture_t* capture) {
  SetProcessDPIAware();
  HDC hScreenDC = GetDC(NULL);
  if (hScreenDC == NULL) return false;

  HDC hMemoryDC = CreateCompatibleDC(hScreenDC);
  if (hMemoryDC == NULL) {
    ReleaseDC(NULL, hScreenDC);
    return false;
  }

  int sx = GetSystemMetrics(SM_XVIRTUALSCREEN);
  int sy = GetSystemMetrics(SM_YVIRTUALSCREEN);
  int w  = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  int h  = GetSystemMetrics(SM_CYVIRTUALSCREEN);

  capture->width  = w;
  capture->height = h;
  capture->stride = w;

  BITMAPINFO bmi;
  memset(&bmi, 0, sizeof(bmi));
  bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
  bmi.bmiHeader.biWidth       = w;
  bmi.bmiHeader.biHeight      = -h;
  bmi.bmiHeader.biPlanes      = 1;
  bmi.bmiHeader.biBitCount    = 32;
  bmi.bmiHeader.biCompression = BI_RGB;

  void* bits      = NULL;
  HBITMAP hBitmap = CreateDIBSection(hScreenDC, &bmi, DIB_RGB_COLORS, &bits, NULL, 0);
  if (hBitmap == NULL || bits == NULL) {
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);
    if (hBitmap) DeleteObject(hBitmap);
    return false;
  }

  HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);

  if (!BitBlt(hMemoryDC, 0, 0, w, h, hScreenDC, sx, sy, SRCCOPY)) {
    SelectObject(hMemoryDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);
    return false;
  }

  capture->pixels = (capture_rgb_t*)calloc((size_t)w * (size_t)h, sizeof(capture_rgb_t));
  if (capture->pixels == NULL) {
    SelectObject(hMemoryDC, hOldBitmap);
    DeleteObject(hBitmap);
    DeleteDC(hMemoryDC);
    ReleaseDC(NULL, hScreenDC);
    return false;
  }

  uint8_t* src = (uint8_t*)bits;
  for (int y = 0; y < h; ++y) {
    uint8_t* row = src + (size_t)y * (size_t)w * 4;
    for (int x = 0; x < w; ++x) {
      uint8_t b                    = row[x * 4 + 0];
      uint8_t g                    = row[x * 4 + 1];
      uint8_t r                    = row[x * 4 + 2];
      capture->pixels[y * w + x].r = r;
      capture->pixels[y * w + x].g = g;
      capture->pixels[y * w + x].b = b;
    }
  }

  SelectObject(hMemoryDC, hOldBitmap);
  DeleteObject(hBitmap);
  DeleteDC(hMemoryDC);
  ReleaseDC(NULL, hScreenDC);

  return true;
}

#endif

void capture_free(capture_t* capture) {
  free(capture->pixels);
}

bool capture_in_bound(capture_t* capture, int x, int y) {
  if (x >= capture->width || x < 0) return false;
  if (y >= capture->height || y < 0) return false;
  return true;
}

bool capture_at(capture_t* capture, int x, int y, capture_rgb_t* rgb) {
  if (x >= capture->width || x < 0) return false;
  if (y >= capture->height || y < 0) return false;
  capture_rgb_t* tmp_rgb = capture->pixels + (y * capture->width + x);
  (*rgb).r               = tmp_rgb->r;
  (*rgb).g               = tmp_rgb->g;
  (*rgb).b               = tmp_rgb->b;
  return true;
}