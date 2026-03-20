"""conanfile.py - Conan 2 recipe for libgeojson.

libgeojson is a header-only library.  Its only runtime dependency is
nlohmann_json.  When ``build_testing`` is enabled (the default in CI) the
tests are built and run using Google Test.
 
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
from conan.tools.build import check_min_cppstd
from conan.tools.cmake import CMake, CMakeDeps, CMakeToolchain, cmake_layout
 
 
class LibgeojsonConan(ConanFile):
    name = "libgeojson"
    version = "0.2.0"
    description = "GeoJSON for Modern C++"
    license = "MIT"
    url = "https://github.com/psalvaggio/libgeojson"
    homepage = "https://github.com/psalvaggio/libgeojson"
    topics = ("geojson", "json", "gis", "header-only")

    package_type = "header-library"
    settings = "os", "compiler", "build_type", "arch"

    options = {"build_testing": [True, False]}
    default_options ={"build_testing": False}

    exports_sources = (
        "CMakeLists.txt",
        "cmake/*",
        "include/*",
        "tests/*",
        "LICENSE",
    )

    # Source tree is not modified during build — safe to skip the copy.
    no_copy_source = True
 
    # ------------------------------------------------------------------
    # Dependencies
    # ------------------------------------------------------------------

    def requirements(self):
        self.requires("nlohmann_json/3.12.0")
 
    def build_requirements(self):
        self.test_requires("gtest/1.17.0")

    # ------------------------------------------------------------------
    # Validation
    # ------------------------------------------------------------------
 
    def validate(self):
        check_min_cppstd(self, "17")

    # ------------------------------------------------------------------
    # Layout / generate / build / package
    # ------------------------------------------------------------------
 
    def layout(self):
        cmake_layout(self)
 
    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["BUILD_TESTING"] = bool(self.options.build_testing)
        tc.generate()

        deps = CMakeDeps(self)
        deps.generate()

    def build(self):
        # Nothing to compile for the library itself; only invoke CMake
        # when tests are requested so that `conan install` stays fast.
        if self.options.build_testing:
            cmake = CMake(self)
            cmake.configure()
            cmake.build()
            cmake.test()

    def package(self):
        self.copy("LICENSE", dst="licenses")
        self.copy("include/*", dst="", keep_path=True)
 
    def package_info(self):
        self.cpp_info.bindirs = []
        self.cpp_info.libdirs = []
        self.cpp_info.set_property("cmake_file_name", "libgeojson")
        self.cpp_info.set_property("cmake_target_name", "libgeojson::libgeojson")
