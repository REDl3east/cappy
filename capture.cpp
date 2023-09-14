#include "capture.h"

#include <bitset>
#include <iomanip>
#include <sstream>

#if __linux__
  #include <X11/Xlib.h>
  #include <X11/Xutil.h>
#elif _WIN32
  #include <windows.h>
#endif

Capture::~Capture() {
  if (captured) delete pixels;
}

bool Capture::capture() {
  if (captured) return false;

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
#elif _WIN32
  SetProcessDPIAware();
  HDC hScreenDC      = GetDC(nullptr); // CreateDC("DISPLAY",nullptr,nullptr,nullptr);
  HDC hMemoryDC      = CreateCompatibleDC(hScreenDC);
  width              = GetDeviceCaps(hScreenDC, HORZRES);
  stride             = width;
  height             = GetDeviceCaps(hScreenDC, VERTRES);
  HBITMAP hBitmap    = CreateCompatibleBitmap(hScreenDC, width, height);
  HBITMAP hOldBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDC, hBitmap));
  BitBlt(hMemoryDC, 0, 0, width, height, hScreenDC, 0, 0, SRCCOPY);
  hBitmap = static_cast<HBITMAP>(SelectObject(hMemoryDC, hOldBitmap));

  BITMAPINFO MyBMInfo       = {0};
  MyBMInfo.bmiHeader.biSize = sizeof(MyBMInfo.bmiHeader);

  if (!GetDIBits(hMemoryDC, hBitmap, 0, 0, NULL, &MyBMInfo, DIB_RGB_COLORS)) {
    DeleteDC(hMemoryDC);
    DeleteDC(hScreenDC);
    return false;
  }

  BYTE* pixel_bytes = new BYTE[MyBMInfo.bmiHeader.biSizeImage];

  MyBMInfo.bmiHeader.biBitCount    = 32;
  MyBMInfo.bmiHeader.biCompression = BI_RGB;
  MyBMInfo.bmiHeader.biHeight      = std::abs(MyBMInfo.bmiHeader.biHeight);

  if (!GetDIBits(hMemoryDC, hBitmap, 0, MyBMInfo.bmiHeader.biHeight, pixel_bytes, &MyBMInfo, DIB_RGB_COLORS)) {
    DeleteDC(hMemoryDC);
    DeleteDC(hScreenDC);
    delete[] pixel_bytes;
    return false;
  }

  pixels = new RGB[width * height];

  int i = 0;

  for (int y = height - 1; y >= 0; --y) {
    for (int x = 0; x < width; ++x) {
      RGB& rgb = pixels[i];
      rgb.b    = pixel_bytes[(y * width + x) * 4];
      rgb.g    = pixel_bytes[(y * width + x) * 4 + 1];
      rgb.r    = pixel_bytes[(y * width + x) * 4 + 2];
      i++;
    }
  }

  DeleteDC(hMemoryDC);
  DeleteDC(hScreenDC);
  delete[] pixel_bytes;

  captured = true;
  return true;
#endif

  return false;
}

std::string toDecimalString(const RGB& color) {
  int x = (color.r << 16) | (color.g << 8) | color.b;
  std::stringstream stream;
  stream << x;
  return stream.str();
}

std::string toDecimalSepString(const RGB& color) {
  std::stringstream stream;
  stream << static_cast<int>(color.r) << ", " << static_cast<int>(color.g) << ", " << static_cast<int>(color.b);
  return stream.str();
}

std::string toHexString(const RGB& color) {
  std::stringstream stream;
  stream << "0x" << std::setfill('0') << std::setw(2) << std::hex;
  stream << static_cast<int>(color.r) << std::setw(2) << static_cast<int>(color.g) << std::setw(2) << static_cast<int>(color.b);
  return stream.str();
}

std::string toHexSepString(const RGB& color) {
  std::stringstream stream;
  stream << "0x" << std::setfill('0') << std::setw(2) << std::hex;
  stream << static_cast<int>(color.r) << std::setw(2) << ", ";
  stream << "0x" << std::setfill('0') << std::setw(2) << std::hex;
  stream << static_cast<int>(color.g) << std::setw(2) << ", ";
  stream << "0x" << std::setfill('0') << std::setw(2) << std::hex;
  stream << static_cast<int>(color.b) << std::setw(2);
  return stream.str();
}

std::string toBinaryString(const RGB& color) {
  return "0b" + std::bitset<8>(color.r).to_string() + std::bitset<8>(color.g).to_string() + std::bitset<8>(color.b).to_string();
}

std::string toBinarySepString(const RGB& color) {
  return "0b" + std::bitset<8>(color.r).to_string() + ", 0b" + std::bitset<8>(color.g).to_string() + ", 0b" + std::bitset<8>(color.b).to_string();
}