#include "capture.h"

#include <stdlib.h>

#ifdef CAPPY_BUILD_WINDOWS
  #include <windows.h>
#elif CAPPY_BUILD_X11
  #include <X11/Xlib.h>
  #include <X11/Xutil.h>
#elif CAPPY_BUILD_WAYLAND
  #include "stb_image.h"
  #include <systemd/sd-bus.h>
#else
// nothing
#endif

#ifdef CAPPY_BUILD_WINDOWS
static bool capture_screen_windows(capture_t* capture);
#elif CAPPY_BUILD_X11
static bool capture_screen_x11(capture_t* capture);
#elif CAPPY_BUILD_WAYLAND
static int sdbus_on_screenshot(sd_bus_message* m, void* userdata, sd_bus_error* ret_error);
static char* sdbus_screenshot_get_uri();
static bool capture_screen_wayland(capture_t* capture);
#else
static bool capture_screen_stub(capture_t* capture);
#endif

bool capture_screen(capture_t* capture) {
#ifdef CAPPY_BUILD_WINDOWS
  return capture_screen_windows(capture);
#elif CAPPY_BUILD_X11
  return capture_screen_x11(capture);
#elif CAPPY_BUILD_WAYLAND
  return capture_screen_wayland(capture);
#else
  return capture_screen_stub(capture);
#endif
}

#ifdef CAPPY_BUILD_WINDOWS
static bool capture_screen_windows(capture_t* capture) {
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

#elif CAPPY_BUILD_X11

static bool capture_screen_x11(capture_t* capture) {
  Display* display = XOpenDisplay(NULL);
  if (display == NULL) return false;

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

#elif CAPPY_BUILD_WAYLAND

static bool capture_screen_wayland(capture_t* capture) {
  char* uri = sdbus_screenshot_get_uri();
  if (uri == NULL) return false;

  char* uri_ptr = uri;

  if (strncmp("file://", uri_ptr, 7) == 0) {
    uri_ptr += 7;
  } else if (strncmp("file:/", uri_ptr, 6) == 0) {
    uri_ptr += 6;
  }

  int width, height, n;
  unsigned char* data = stbi_load(uri_ptr, &width, &height, &n, 4);
  if (data == NULL) {
    remove(uri_ptr);
    free(uri);
    return false;
  }

  remove(uri_ptr);

  capture->width  = width;
  capture->height = height;
  capture->stride = width;
  capture->pixels = malloc(width * height * sizeof(capture_rgb_t));

  if (capture->pixels == NULL) {
    stbi_image_free(data);
    free(uri);
    return false;
  }

  for (int i = 0; i < width * height; ++i) {
    capture->pixels[i].r = data[i * 4 + 0];
    capture->pixels[i].g = data[i * 4 + 1];
    capture->pixels[i].b = data[i * 4 + 2];
  }

  stbi_image_free(data);
  free(uri);

  return true;
}

#else
static bool capture_screen_stub(capture_t* capture) {
  return false;
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

#ifdef CAPPY_BUILD_WINDOWS
// Windows: Nothing
#elif CAPPY_BUILD_X11
// X11: Nothing
#elif CAPPY_BUILD_WAYLAND
typedef struct capture_userdata {
  sd_bus* bus;
  char* uri;
  uint32_t response;
} capture_userdata;

static int sdbus_on_screenshot(sd_bus_message* m, void* userdata, sd_bus_error* ret_error) {
  capture_userdata* capture = (capture_userdata*)userdata;
  char* uri                 = NULL;
  sd_bus_message* dict      = NULL;
  const char* key           = NULL;

  int r = sd_bus_message_read(m, "u", &capture->response);
  if (r < 0) return 0;

  r = sd_bus_message_enter_container(m, 'a', "{sv}");
  if (r < 0) return 0;

  while ((r = sd_bus_message_enter_container(m, 'e', "sv")) > 0) {
    r = sd_bus_message_read(m, "s", &key);
    if (r < 0) break;

    if (strcmp(key, "uri") == 0) {
      r = sd_bus_message_read(m, "v", "s", &uri);
      if (r >= 0 && uri) {
        capture->uri = strdup(uri);
      }

    } else {
      r = sd_bus_message_skip(m, "v");
    }

    sd_bus_message_exit_container(m);
  }

  sd_bus_message_exit_container(m);

  sd_bus_close(capture->bus);
  return 0;
}

// https://flatpak.github.io/xdg-desktop-portal/docs/doc-org.freedesktop.portal.Screenshot.html
static char* sdbus_screenshot_get_uri() {
  sd_bus* bus           = NULL;
  sd_bus_message* reply = NULL;
  sd_bus_error error    = SD_BUS_ERROR_NULL;
  int r;

  capture_userdata userdata;
  userdata.response = 1; // 1: The user cancelled the interaction
  userdata.uri      = NULL;

  r = sd_bus_open_user(&bus);
  if (r < 0) goto finish;

  userdata.bus = bus;

  const char* method_destination = "org.freedesktop.portal.Desktop";
  const char* method_path        = "/org/freedesktop/portal/desktop";
  const char* method_interface   = "org.freedesktop.portal.Screenshot";
  const char* method_member      = "Screenshot";

  const char* types         = "sa{sv}";
  const char* parent_window = "";
  const int modal           = 0;
  const int interactive     = 0;

  r = sd_bus_call_method(
      bus,
      method_destination,
      method_path,
      method_interface,
      method_member,
      &error,
      &reply,
      types,
      parent_window,
      2,
      "modal", "b", modal,
      "interactive", "b", interactive);

  if (r < 0) goto finish;

  const char* signal_path;
  r = sd_bus_message_read(reply, "o", &signal_path);
  if (r < 0) goto finish;

  const char* signal_sender    = "org.freedesktop.portal.Desktop";
  const char* signal_interface = "org.freedesktop.portal.Request";
  const char* signal_member    = "Response";

  sd_bus_slot* slot = NULL;

  r = sd_bus_match_signal(
      bus,
      &slot,
      signal_sender,
      signal_path,
      signal_interface,
      signal_member,
      sdbus_on_screenshot,
      &userdata);

  if (r < 0) goto finish;

  for (;;) {
    r = sd_bus_process(bus, NULL);
    if (r < 0) break;
    if (r > 0) continue;
    if (sd_bus_wait(bus, (uint64_t)-1) < 0) break;
  }

finish:
  sd_bus_error_free(&error);
  sd_bus_message_unref(reply);
  sd_bus_unref(bus);

  if (r < 0 && userdata.response != 0) {
    free(userdata.uri);
    return NULL;
  }

  return userdata.uri;
}
#else
// Stub: Nothing
#endif
