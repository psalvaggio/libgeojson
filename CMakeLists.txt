cmake_minimum_required(VERSION 3.27)

# If we are a single-config build (make, Ninja), set default to Release
get_property(multiConfig GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if (NOT multiConfig AND NOT DEFINED CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build.")
endif()

project(libgeojson
  VERSION
    0.1.0
  LANGUAGES
    CXX
)

# Set C++11 Standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

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

# Prevent in-source builds
if (${CMAKE_SOURCE_DIR} STREQUAL ${CMAKE_BINARY_DIR})
  message(FATAL_ERROR "In-source builds not allowed")
endif()

message(STATUS "Build type is: ${CMAKE_BUILD_TYPE}")

include(GNUInstallDirs)

find_package(nlohmann_json REQUIRED)

# If we are using ninja or make, we still want colors
if (CMAKE_GENERATOR STREQUAL "Ninja" OR
    CMAKE_GENERATOR STREQUAL "Unix Makefiles")
  set(CMAKE_COLOR_DIAGNOSTICS TRUE)
endif()

add_library(libgeojson INTERFACE)
add_library(${PROJECT_NAME}::libgeojson ALIAS libgeojson)
target_compile_features(libgeojson INTERFACE cxx_std_11)
target_compile_options(libgeojson
  INTERFACE
    $<$<CXX_COMPILER_ID:MSVC>:/Zc:__cplusplus>
)
target_link_libraries(libgeojson
  INTERFACE
    nlohmann_json::nlohmann_json
)
target_include_directories(libgeojson
  INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDE_DIR}>
)


include(CTest)
option(LIBGEOJSON_BUILD_TESTS "Whether to build unit tests" ${PROJECT_IS_TOP_LEVEL})
if (LIBGEOJSON_BUILD_TESTS AND BUILD_TESTING)
  include(FetchContent)
  FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/56efe3983185e3f37e43415d1afa97e3860f187f.zip
  )
  # For Windows: Prevent overriding the parent project's compiler/linker settings
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)

  enable_testing()

  add_subdirectory(tests)
endif()
