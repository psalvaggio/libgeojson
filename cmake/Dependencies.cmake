# -- nlohmann_json ------------------------------------------------------------

set(_NLOHMANN_MIN_VERSION "3.0.0")

# Prefer any pre-found target, then try find_package()
if (TARGET nlohmann_json::nlohmann_json)
  message(STATUS "[libgeojson] nlohmann_json target already defined - skipping dependency resolution")
  # TODO: Check the version
else()
  find_package(nlohmann_json ${_NLOHMANN_MIN_VERSION} REQUIRED)
  message(STATUS "[libgeojson] Found nlohmann_json ${nlohmann_json_VERSION} via find_package")
endif()
