"""conanfile.py - Conan 2 recipe for libgeojson.
 
Usage (from the repo root):
  # For a release build:
  conan install . --build=missing
  cmake --preset conan-release
  cmake --build --preset conan-release
  cmake --install build/Release [--prefix <prefix>]

  # For a debug build:
  conan install . --build=missing -s build_type=Debug
  cmake --preset conan-debug
  cmake --build --preset conan-debug
  cmake --install build/Debug [--prefix <prefix>]
"""
 
from conan import ConanFile
from conan.tools.cmake import CMakeToolchain, CMakeDeps, cmake_layout
 
 
class LibgeojsonConan(ConanFile):
    name = "libgeojson"
    version = "1.0.0"
    description = "GeoJSON for Modern C++"
    url = "https://github.com/psalvaggio/libgeojson"
    license = "MIT"
    package_type = "header-library"
    settings = "os", "compiler", "build_type", "arch"
    exports_sources = ("CMakeLists.txt", "cmake/*", "include/*")
 
    def requirements(self):
        self.requires("nlohmann_json/3.12.0")
 
    def build_requirements(self):
        self.test_requires("gtest/1.17.0")
 
    def layout(self):
        cmake_layout(self)
 
    def generate(self):
        deps = CMakeDeps(self)
        deps.generate()
        tc = CMakeToolchain(self)
        tc.generate()
