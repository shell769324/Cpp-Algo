set(BINARY ${CMAKE_PROJECT_NAME}_tst)

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.h *.cpp main.cpp)

# file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false utility/* tree/tree_test_util.* tree/avl_tree_test.cpp main.cpp)

set(SOURCES ${TEST_SOURCES})
message(STATUS "names of binary files: ${TEST_SOURCES}")

add_executable(${BINARY} ${TEST_SOURCES})

add_test(NAME ${BINARY} COMMAND ${BINARY})

include_directories(${CMAKE_SOURCE_DIR})
target_link_libraries(${BINARY} PUBLIC containers gtest)
