project(Squish)

cmake_minimum_required(VERSION 3.21)

# Delayed Coding Version
add_subdirectory(lib)
include_directories(lib)
add_executable(Squish ${PROJECT_SOURCE_DIR}/sample.cpp)

target_link_libraries(Squish PUBLIC squish_lib)