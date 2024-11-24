set(BINARY ${CMAKE_PROJECT_NAME}_tst)

# file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.h *.cpp main.cpp)

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false
    utility/*
    vector_test.cpp
    common_test.cpp
    map_test.h
    set_test.h
    stack_test/*
    deque_test/*
    tree/tree_test_util.*
    tree/binary_tree_node_util.*
    tree/binary_tree_iterator_test.cpp
    tree/binary_tree_node_test.cpp
    tree/tree_common_test.h
    tree/tree_bulk_operationr_complexity_test.h
    tree/tree_parallel_comparison_test.h
    tree/avl_node_test.cpp
    tree/avl_tree_test.cpp
    tree/avl_tree_map_test.cpp
    tree/avl_tree_set_test.cpp
    tree/red_black_tree_test.cpp
    tree/red_black_tree_set_test.cpp
    tree/red_black_tree_map_test.cpp
    range_query_tree/*
    thread_pool_executor/thread_pool_executor_test.cpp
    main.cpp)

set(SOURCES ${TEST_SOURCES})
message(STATUS "names of binary files: ${TEST_SOURCES}")

add_executable(${BINARY} ${TEST_SOURCES})

add_test(NAME ${BINARY} COMMAND ${BINARY})

include_directories(${CMAKE_SOURCE_DIR})
target_link_libraries(${BINARY} PUBLIC containers gtest)
