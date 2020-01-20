# Add a Googletest-based unit test
#
# Arguments:
#  TEST_NAME  Name of the test executable
#  SOURCES    List of source files to compile into the test
#  ARGUMENTS  Any additional flags that need to be added to the command
function(libgeojson_add_gtest TEST_NAME)
  set(options)
  set(oneValueArgs)
  set(multiValueArgs SOURCES ARGUMENTS)
  cmake_parse_arguments(libgeojson_add_gtest "${options}" "${oneValueArgs}"
                                             "${multiValueArgs}" ${ARGN})

  add_executable(${TEST_NAME} ${libgeojson_add_gtest_SOURCES})
  target_link_libraries(${TEST_NAME}
    GTest::gtest
    #GTest::gtest_main
  )
  if (NOT WIN32)
    target_link_libraries(${TEST_NAME}
      Threads::Threads
    )
  endif()

  set(cmd_args)
  if (libgeojson_add_gtest_ARGUMENTS)
    set(cmd_args ${cmd_args} ${libgeojson_add_gtest_ARGUMENTS})
  endif()
  add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME} ${cmd_args})
endfunction()

libgeojson_add_gtest(UnitTests SOURCES unit_tests.cpp)
target_link_libraries(UnitTests ${PROJECT_NAME}::libgeojson)
