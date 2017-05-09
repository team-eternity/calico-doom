cmake_minimum_required(VERSION 3.0)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

project("Calico Doom" C CXX)

find_package(SDL2 2.0.0)
find_package(SDL2_mixer 2.0.0)
find_package(OpenGL)

add_subdirectory(src)
