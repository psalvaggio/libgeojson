/** Minimal consumer exercising the installed libgeojson CMake export.
 *
 * \file main.cpp
 * \date 2026-03-20
 */

#include <libgeojson/libgeojson.h>

#include <iostream>

int main() {
  auto pt = geojson::Point(-73.98, 40.75);
  std::cout << "Hello from libgeojson! point=" << pt << '\n';
  return 0;
}
