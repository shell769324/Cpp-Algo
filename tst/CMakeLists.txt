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
    tree/*
    range_query_tree/*
    allocator_aware_algorithms_test.cpp
    thread_pool_executor/thread_pool_executor_test.cpp
    main.cpp)

set(SOURCES ${TEST_SOURCES})
message(STATUS "names of binary files: ${TEST_SOURCES}")

add_executable(${BINARY} ${TEST_SOURCES})

add_test(NAME ${BINARY} COMMAND ${BINARY})

include_directories(${CMAKE_SOURCE_DIR})
target_link_libraries(${BINARY} PUBLIC containers gtest)
