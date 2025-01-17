# Minimum version of CMake required to build this project
cmake_minimum_required(VERSION 3.30)

# gcc-14 breaks on mac for some unknown reason
if(CMAKE_SYSTEM_NAME MATCHES "Darwin")
    message("MacOS detected. Use clang g++")
    set(CMAKE_C_COMPILER    "/usr/bin/gcc")
    set(CMAKE_CXX_COMPILER  "/usr/bin/g++")
else()
    message("Linux detected. Use g++-14")
    set(CMAKE_C_COMPILER    "/usr/bin/gcc-14")
    set(CMAKE_CXX_COMPILER  "/usr/bin/g++-14")
endif()

# Name of the project
project(cpp_algo VERSION 1.0)

# Include -g for debugging
set(CMAKE_CXX_FLAGS "-Wall -Wextra")

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Release)

add_subdirectory(src)
add_subdirectory(tst)
add_subdirectory(benchmark)
