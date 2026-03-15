# cmake/Install.cmake
#
# Installs libgeojson and generates a relocatable CMake package config so that
# downstream projects can consume the library via:
#
#   find_package(libgeojson REQUIRED)
#   target_link_libraries(my_target PRIVATE libgeojson::libgeojson)

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# -- Install the headers -------------------------------------------------------

install(
  DIRECTORY   "${CMAKE_CURRENT_SOURCE_DIR}/include/"
  DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
  FILES_MATCHING PATTERN "*.h" PATTERN "*.hpp"
)

# -- Install the INTERFACE target and export it --------------------------------

install(TARGETS ${PROJECT_NAME} EXPORT ${PROJECT_NAME}Targets)

install(
  EXPORT      ${PROJECT_NAME}Targets
  FILE        ${PROJECT_NAME}Targets.cmake
  NAMESPACE   ${PROJECT_NAME}::
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

# -- Generate the installed CMake config file ----------------------------------

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/${PROJECT_NAME}Config.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)

# -- Generate the installed version file ---------------------------------------

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  VERSION       "${PROJECT_VERSION}"
  COMPATIBILITY AnyNewerVersion
)

install(
  FILES
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}"
)
