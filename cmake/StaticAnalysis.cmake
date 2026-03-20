# cmake/StaticAnalysis.cmake
#
# Opt-in static analysis tools. Each tool is activated by the corresponding
# LIBGEOJSON_ENABLE_* option defined in the root CMakeLists.txt.
#
# Tools are wired up via CMake's built-in <LANG>_<TOOL> target properties so
# that every target that links libgeojson::libgeojson is automatically checked
# during the normal build — no separate step required.
#
# Usage:
#   cmake -B build \
#         -DLIBGEOJSON_ENABLE_CLANG_TIDY=ON \
#         -DLIBGEOJSON_ENABLE_CPPCHECK=ON \
#         -DLIBGEOJSON_ENABLE_IWYU=ON


# -- clang-tidy ----------------------------------------------------------------

if(LIBGEOJSON_ENABLE_CLANG_TIDY)
  find_program(CLANG_TIDY_EXE
    NAMES
        clang-tidy
        clang-tidy-22
        clang-tidy-21
        clang-tidy-20
        clang-tidy-19
        clang-tidy-18
        clang-tidy-17
        clang-tidy-16
    DOC   "Path to clang-tidy executable"
  )

  if(CLANG_TIDY_EXE)
    message(STATUS "[libgeojson] clang-tidy enabled: ${CLANG_TIDY_EXE}")

    # .clang-tidy at the repo root is the authoritative config; fall back to
    # an inline set of checks if that file is absent.
    if(EXISTS "${CMAKE_SOURCE_DIR}/.clang-tidy")
      set(_CLANG_TIDY_CMD "${CLANG_TIDY_EXE}"
        "--use-color"
      )
    else()
      set(_CLANG_TIDY_CMD "${CLANG_TIDY_EXE}"
        "--checks=bugprone-*,clang-analyzer-*,cppcoreguidelines-*,misc-*,modernize-*,performance-*,portability-*,readability-*,-modernize-use-trailing-return-type,-readability-braces-around-statements"
        "--use-color"
        "--warnings-as-errors="  # empty: warnings only, not errors
      )
    endif()

    set(CMAKE_CXX_CLANG_TIDY "${_CLANG_TIDY_CMD}" CACHE STRING "" FORCE)
  else()
    message(WARNING "[libgeojson] LIBGEOJSON_ENABLE_CLANG_TIDY=ON but clang-tidy not found - skipping")
  endif()
endif()

# -- cppcheck ------------------------------------------------------------------

if(LIBGEOJSON_ENABLE_CPPCHECK)
  find_program(CPPCHECK_EXE
    NAMES cppcheck
    DOC   "Path to cppcheck executable"
  )

  if(CPPCHECK_EXE)
    message(STATUS "[libgeojson] cppcheck enabled: ${CPPCHECK_EXE}")

    execute_process(
      COMMAND "${CPPCHECK_EXE}" --version
      OUTPUT_VARIABLE _CPPCHECK_VERSION_OUT
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    message(STATUS "[libgeojson] cppcheck version: ${_CPPCHECK_VERSION_OUT}")

    set(CMAKE_CXX_CPPCHECK
      "${CPPCHECK_EXE}"
      "--enable=warning,style,performance,portability"
      "--suppress=missingIncludeSystem"
      "--inline-suppr"            # respect // cppcheck-suppress comments
      "--std=c++17"
      "--error-exitcode=1"        # fail the build on cppcheck errors
      CACHE STRING "" FORCE
    )
  else()
    message(WARNING "[libgeojson] LIBGEOJSON_ENABLE_CPPCHECK=ON but cppcheck not found - skipping")
  endif()
endif()

# -- include-what-you-use ------------------------------------------------------

if(LIBGEOJSON_ENABLE_IWYU)
  find_program(IWYU_EXE
    NAMES include-what-you-use iwyu
    DOC   "Path to include-what-you-use executable"
  )

  if(IWYU_EXE)
    message(STATUS "[libgeojson] include-what-you-use enabled: ${IWYU_EXE}")
    set(CMAKE_CXX_INCLUDE_WHAT_YOU_USE
      "${IWYU_EXE}"
      "-Xiwyu;--cxx17ns"
      "-Xiwyu;--quoted_includes_first"
      "-Xiwyu;--mapping_file=${CMAKE_SOURCE_DIR}/.iwyu.imp"  # optional
      CACHE STRING "" FORCE
    )
  else()
    message(WARNING "[libgeojson] LIBGEOJSON_ENABLE_IWYU=ON but include-what-you-use not found - skipping")
  endif()
endif()

# -- Link-what-you-use (built-in linker check, GCC/Clang) ----------------------

# Enabled automatically in Debug builds when the linker supports it; no extra
# tool installation required.
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
  set(CMAKE_LINK_WHAT_YOU_USE ON)
endif()
