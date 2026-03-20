cmake_minimum_required(VERSION 3.27)
project(geojson_export_consumer LANGUAGES CXX)
 
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
 
message(STATUS "CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}")
find_package(libgeojson REQUIRED)
 
add_executable(geojson_hello main.cpp)
target_link_libraries(geojson_hello PRIVATE libgeojson::libgeojson)
