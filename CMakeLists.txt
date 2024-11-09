# Minimum version of CMake required to build this project
cmake_minimum_required(VERSION 3.30)


set(CMAKE_C_COMPILER    "/usr/local/bin/gcc-13")
set(CMAKE_CXX_COMPILER  "/usr/local/bin/g++-13")

# Name of the project
project(cpp_algo)

set(CMAKE_CXX_FLAGS "-Wall -Wextra -ld_classic")

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_BUILD_TYPE Debug)

include_directories(${PROJECT_NAME} PUBLIC src)

add_subdirectory(src)
add_subdirectory(tst)
add_subdirectory(lib/googletest)
