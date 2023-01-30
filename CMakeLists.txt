# Minimum version of CMake required to build this project
cmake_minimum_required(VERSION 3.21)

# Name of the project
project(cpp_algo)

set(CMAKE_CXX_FLAGS "-Wall -Wextra")

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_BUILD_TYPE Debug)

include_directories(${PROJECT_NAME} PUBLIC src)

add_subdirectory(src)
add_subdirectory(tst)
add_subdirectory(lib/googletest)
