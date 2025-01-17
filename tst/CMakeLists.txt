set(BINARY ${CMAKE_PROJECT_NAME}_tst)

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false
    utility/*
    vector/*
    common_test.cpp
    map_test.h
    set_test.h
    stack/*
    deque/*
    tree/*
    range_query_tree/*
    allocator_aware_algorithms_test.cpp
    thread_pool_executor/thread_pool_executor_test.cpp
    main.cpp)


include(FetchContent)
FetchContent_Declare(
  googletest
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/googletest/archive/35d0c365609296fa4730d62057c487e3cfa030ff.zip
)
FetchContent_MakeAvailable(googletest)

add_executable(${BINARY} ${TEST_SOURCES})

add_test(NAME ${BINARY} COMMAND ${BINARY})

include_directories(${CMAKE_SOURCE_DIR})
target_link_libraries(${BINARY} PUBLIC containers gtest)
