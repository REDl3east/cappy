#include "capture.h"

#if __linux__
  #include <X11/Xlib.h>
  #include <X11/Xutil.h>
#elif _WIN32
  #include <windows.h>
#endif

#include <stdlib.h>

bool capture_screen(capture_t* capture) {
#if __linux__
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
#elif _WIN32
  SetProcessDPIAware();
  HDC hScreenDC = GetDC(NULL);
  HDC hMemoryDC = CreateCompatibleDC(hScreenDC);

  capture->width     = GetSystemMetrics(SM_CXVIRTUALSCREEN);
  capture->height    = GetSystemMetrics(SM_CYVIRTUALSCREEN);
  capture->stride    = capture->width;
  HBITMAP hBitmap    = CreateCompatibleBitmap(hScreenDC, capture->width, capture->height);
  HBITMAP hOldBitmap = (HBITMAP)SelectObject(hMemoryDC, hBitmap);
  BitBlt(hMemoryDC, GetSystemMetrics(SM_XVIRTUALSCREEN), GetSystemMetrics(SM_YVIRTUALSCREEN), capture->width, capture->height, hScreenDC, 0, 0, SRCCOPY);
  hBitmap = (HBITMAP)SelectObject(hMemoryDC, hOldBitmap);

  BITMAPINFO MyBMInfo       = {0};
  MyBMInfo.bmiHeader.biSize = sizeof(MyBMInfo.bmiHeader);

  if (!GetDIBits(hMemoryDC, hBitmap, 0, 0, NULL, &MyBMInfo, DIB_RGB_COLORS)) {
    DeleteDC(hMemoryDC);
    DeleteDC(hScreenDC);
    return false;
  }

  BYTE* pixel_bytes = (BYTE*)calloc(MyBMInfo.bmiHeader.biSizeImage, sizeof(BYTE));

  MyBMInfo.bmiHeader.biBitCount    = 32;
  MyBMInfo.bmiHeader.biCompression = BI_RGB;
  MyBMInfo.bmiHeader.biHeight      = abs(MyBMInfo.bmiHeader.biHeight);

  if (!GetDIBits(hMemoryDC, hBitmap, 0, MyBMInfo.bmiHeader.biHeight, pixel_bytes, &MyBMInfo, DIB_RGB_COLORS)) {
    DeleteDC(hMemoryDC);
    DeleteDC(hScreenDC);
    free(pixel_bytes);
    return false;
  }

  capture->pixels = (capture_rgb_t*)calloc(capture->width * capture->height, sizeof(capture_rgb_t));

  int i = 0;
  for (int y = capture->height - 1; y >= 0; --y) {
    for (int x = 0; x < capture->width; ++x) {
      capture_rgb_t* rgb = capture->pixels + i;
      rgb->b             = pixel_bytes[(y * capture->width + x) * 4 + 0];
      rgb->g             = pixel_bytes[(y * capture->width + x) * 4 + 1];
      rgb->r             = pixel_bytes[(y * capture->width + x) * 4 + 2];
      i++;
    }
  }

  DeleteDC(hMemoryDC);
  DeleteDC(hScreenDC);
  free(pixel_bytes);

  return true;
#endif

  return false;
}

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