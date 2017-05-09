cmake_minimum_required(VERSION 3.0)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

project("Calico Doom" C CXX)

add_definitions("-Wall -Wextra")

# Once CMake 3.1 is required, you can replace this with CMAKE_CXX_STANDARD
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_CXX_FLAGS "--std=gnu++11 ${CMAKE_CXX_FLAGS}")
endif()

find_package(SDL2 2.0.0)
find_package(SDL2_mixer 2.0.0)
find_package(OpenGL)

add_subdirectory(src)
