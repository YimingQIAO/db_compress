project(Squish)

cmake_minimum_required(VERSION 3.23)

# Delayed Coding Version
add_subdirectory(lib)
add_executable(Squish ${PROJECT_SOURCE_DIR}/sample.cpp)

target_link_libraries(Squish PUBLIC squish_lib)