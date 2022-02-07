set(BINARY ${CMAKE_PROJECT_NAME}_run)

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true common.h vector.h stack.h map.h set.h
     tree/* main.cpp)

set(SOURCES ${SOURCES})

add_executable(${BINARY} ${SOURCES})

add_library(containers ${SOURCES})

include_directories(${CMAKE_SOURCE_DIR})
