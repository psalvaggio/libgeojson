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
