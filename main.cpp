#include <iostream>

#include "capture.h"

int main() {
  Capture capture;
  if (!capture.capture()) {
    std::cerr << "Failed to capture screen!\n";
    return 1;
  }

  capture.write_pnm("output.pnm");

  return 0;
}
