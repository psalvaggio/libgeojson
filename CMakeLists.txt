cmake_minimum_required(VERSION 3.27)

# If we are a single-config build (make, Ninja), set default to Release
get_property(multiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (NOT multiConfig AND NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build.")
endif()

project(libgeojson
  VERSION 0.2.0
  DESCRIPTION "GeoJSON for Modern C++"
  HOMEPAGE_URL "https://github.com/psalvaggio/libgeojson"
  LANGUAGES CXX
)

# -- Prevent in-source builds -------------------------------------------------

if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed")
endif()

# -- Project-wide settings ----------------------------------------------------

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# If we are using ninja or make, we still want colors
if (CMAKE_GENERATOR STREQUAL "Ninja" OR
    CMAKE_GENERATOR STREQUAL "Unix Makefiles")
  set(CMAKE_COLOR_DIAGNOSTICS TRUE)
endif()

# -- Options ------------------------------------------------------------------

option(LIBGEOJSON_BUILD_TESTS "Build libgeojson tests" ${PROJECT_IS_TOP_LEVEL})
option(LIBGEOJSON_ENABLE_CLANG_TIDY  "Enable clang-tidy analysis" OFF)
option(LIBGEOJSON_ENABLE_CPPCHECK "Enable cppcheck analysis" OFF)
option(LIBGEOJSON_ENABLE_IWYU "Enable include-what-you-use" OFF)


# -- Compilation database -----------------------------------------------------

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)  # For analysis tools (clang-tidy)
if (PROJECT_IS_TOP_LEVEL AND CMAKE_EXPORT_COMPILE_COMMANDS)
    set(COMPILE_DB "${PROJECT_BINARY_DIR}/compile_commands.json")
    set(COMPILE_DB_LINK "${PROJECT_SOURCE_DIR}/compile_commands.json")

    if(EXISTS "${COMPILE_DB}")
        execute_process(
            COMMAND ${CMAKE_COMMAND} -E create_symlink
                    "${COMPILE_DB}"
                    "${COMPILE_DB_LINK}"
            RESULT_VARIABLE _symlink_result
            ERROR_QUIET
        )
    endif()
endif()

# -- Static analysis ----------------------------------------------------------

include(cmake/StaticAnalysis.cmake)
 
# -- Dependencies -------------------------------------------------------------

include(cmake/Dependencies.cmake)

# -- Logging ------------------------------------------------------------------

message(STATUS "Build type is: ${CMAKE_BUILD_TYPE}")

# -- Library target -----------------------------------------------------------

add_library(libgeojson INTERFACE)
add_library(libgeojson::libgeojson ALIAS libgeojson)
 
include(GNUInstallDirs)
target_include_directories(libgeojson
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_compile_options(libgeojson
  INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>
)
 
target_link_libraries(libgeojson
  INTERFACE
    nlohmann_json::nlohmann_json
)
 
target_compile_features(libgeojson INTERFACE cxx_std_11)


# -- Tests --------------------------------------------------------------------

if (LIBGEOJSON_BUILD_TESTS)
  enable_testing()
  add_subdirectory(tests)
endif()

# -- Install / CMake package config -------------------------------------------

if (PROJECT_IS_TOP_LEVEL)
  include(cmake/Install.cmake)
endif()

