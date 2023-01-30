set(BINARY ${CMAKE_PROJECT_NAME}_run)

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true common.h thread_pool_executor/* vector.h stack.h deque.h tree/* main.cpp)

set(SOURCES ${SOURCES})

add_executable(${BINARY} ${SOURCES})

add_library(containers ${SOURCES})

include_directories(${CMAKE_SOURCE_DIR})
