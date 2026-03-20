# Builds the libgeojson test suite

if (TARGET GTest::gtest_main)
  message(STATUS "[libgeojson] - GTest target already defined - skipping dependency resolution")
  # TODO: Check the version
else()
  find_package(GTest QUIET)
  if (TARGET GTest::gtest_main)
    message(STATUS "[libgeojson] Found GTest via find_package")
  else()
    include(FetchContent)
    FetchContent_Declare(
      googletest
      URL https://github.com/google/googletest/archive/56efe3983185e3f37e43415d1afa97e3860f187f.zip
    )

    # For Windows: Prevent overriding the parent project's compiler/linker
    # settings
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
  endif()
endif()

include(GoogleTest)

# Add a GTest-based unit test
#
# Arguments:
#  TEST_NAME  Name of the test executable
#  SOURCES    List of source files to compile into the test
function(libgeojson_add_gtest TEST_NAME)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs SOURCES)
  cmake_parse_arguments(args "${options}" "${oneValueArgs}"
                             "${multiValueArgs}" ${ARGN})

  add_executable(${TEST_NAME} ${args_SOURCES})
  set_property(TARGET ${TEST_NAME} PROPERTY COMPILE_WARNING_AS_ERROR ON)
  target_link_libraries(${TEST_NAME}
      GTest::gtest_main
      ${PROJECT_NAME}::libgeojson
  )

  # Enable all compiler warnings and treat as errors
  target_compile_options(${TEST_NAME}
    PUBLIC
      $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -pedantic>
      $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  )

  gtest_discover_tests(${TEST_NAME})
endfunction()

libgeojson_add_gtest(UnitTests
  SOURCES
    Predicates.h
    unit_tests.cpp
)

# -- CMake export/import smoke test ------------------------------------------
#
# Installs libgeojson to a temporary prefix, then configures and builds a
# standalone consumer project against it using find_package(). This validates
# the full install + CMake export mechanism end-to-end.
 
set(_export_test_install_prefix "${CMAKE_CURRENT_BINARY_DIR}/export_test_install")
set(_export_test_src_dir        "${CMAKE_CURRENT_SOURCE_DIR}/cmake_export_test")
set(_export_test_build_dir      "${CMAKE_CURRENT_BINARY_DIR}/cmake_export_test_build")
 
# Step 1: install libgeojson into a temp prefix as a test fixture.
add_test(
  NAME    CmakeExport_Install
  COMMAND "${CMAKE_COMMAND}"
            --install "${PROJECT_BINARY_DIR}"
            --prefix  "${_export_test_install_prefix}"
            --config  $<CONFIG>
)
set_tests_properties(CmakeExport_Install PROPERTIES
  FIXTURES_SETUP CmakeExportFixture
)
 
# Forward the Conan-generated toolchain from the parent build into the consumer
# so that the same compiler, stdlib, and Conan package paths are used.
# CMAKE_TOOLCHAIN_FILE is already set by the preset that configured this build
# (e.g. build/Release/generators/conan_toolchain.cmake), so we just propagate it.
set(_export_test_toolchain_args "")
if (CMAKE_TOOLCHAIN_FILE)
  cmake_path(IS_RELATIVE CMAKE_TOOLCHAIN_FILE TOOLCHAIN_IS_RELATIVE)
  if (TOOLCHAIN_IS_RELATIVE)
    list(APPEND _export_test_toolchain_args
      "-DCMAKE_TOOLCHAIN_FILE=${PROJECT_BINARY_DIR}/${CMAKE_TOOLCHAIN_FILE}"
    )
  else()
    list(APPEND _export_test_toolchain_args
        "-DCMAKE_TOOLCHAIN_FILE=${CMAKE_TOOLCHAIN_FILE}"
    )
  endif()
endif()
 
# Step 2: configure the consumer project against the temp prefix.
add_test(
  NAME    CmakeExport_Configure
  COMMAND "${CMAKE_COMMAND}"
            -S "${_export_test_src_dir}"
            -B "${_export_test_build_dir}"
            -DCMAKE_BUILD_TYPE=$<CONFIG>
            -DCMAKE_PREFIX_PATH=${_export_test_install_prefix}
            ${_export_test_toolchain_args}
)
set_tests_properties(CmakeExport_Configure PROPERTIES
  FIXTURES_REQUIRED CmakeExportFixture
  DEPENDS           CmakeExport_Install
)
 
# Step 3: build the consumer project.
add_test(
  NAME    CmakeExport_Build
  COMMAND "${CMAKE_COMMAND}"
            --build "${_export_test_build_dir}"
            --config $<CONFIG>
            --parallel
)
set_tests_properties(CmakeExport_Build PROPERTIES
  FIXTURES_REQUIRED CmakeExportFixture
  DEPENDS           CmakeExport_Configure
)
 
# Step 4: run the resulting binary to confirm it links and executes.
add_test(
  NAME    CmakeExport_Run
  COMMAND "${_export_test_build_dir}/geojson_hello"
)
set_tests_properties(CmakeExport_Run PROPERTIES
  FIXTURES_REQUIRED CmakeExportFixture
  DEPENDS           CmakeExport_Build
)
