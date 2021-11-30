# Minimum version of CMake required to build this project
cmake_minimum_required(VERSION 3.21)

# Name of the project
project(star_anise)

set(CMAKE_CXX_STANDARD 20)

include_directories(${PROJECT_NAME} PUBLIC src)

add_subdirectory(src)
add_subdirectory(tst)
add_subdirectory(lib/googletest)
