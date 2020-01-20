cmake_minimum_required(VERSION 3.8)

# If we are a single-config build (make, Ninja), set default to Release
get_property(multiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (NOT multiConfig AND NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build.")
endif()

project(libgeojson
  VERSION
    0.0.0
  LANGUAGES
    CXX
)

# Set C++11 Standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # For analysis tools (clang-tidy)

# Prevent in-source builds
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed")
endif()

message(STATUS "Build type is: ${CMAKE_BUILD_TYPE}")

option(BUILD_TESTS "Whether to build unit tests" ON)

include(GnuInstallDirs)

find_package(nlohmann_json REQUIRED)

# Enable all compiler warnings and treat as errors
if (MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

# If we are using ninja or make, we still want colors
if (NOT WIN32)
  if (CMAKE_GENERATOR STREQUAL "Ninja" OR
      CMAKE_GENERATOR STREQUAL "Unix Makefiles")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fdiagnostics-color=always")
  endif()
endif()

add_library(libgeojson INTERFACE)
add_library(${PROJECT_NAME}::libgeojson ALIAS libgeojson)
target_link_libraries(libgeojson
  INTERFACE
    nlohmann_json::nlohmann_json
)
target_include_directories(libgeojson
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDE_DIR}>
)

if (BUILD_TESTS)
  enable_testing()

  find_package(GTest 1.8.0 REQUIRED NO_MODULE)

  add_subdirectory(tests)
endif()
