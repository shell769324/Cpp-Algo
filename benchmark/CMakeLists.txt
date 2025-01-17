set(BINARY ${CMAKE_PROJECT_NAME}_benchmark)

file(GLOB BENCHMARK_SOURCES LIST_DIDRECTORIES false
    utility/*
    vector_benchmark.cpp
    deque_benchmark.cpp
    set_benchmark.cpp
    map_benchmark.cpp
    range_query_benchmark.cpp
    main.cpp
)

include(FetchContent)
FetchContent_Declare(
  benchmark
  # Specify the commit you depend on and update it regularly.
  URL https://github.com/google/benchmark/archive/b2b0aab464b1d5be3cf1728d36bb03f8b84f246f.zip
)
FetchContent_MakeAvailable(benchmark)

add_executable(${BINARY} ${BENCHMARK_SOURCES})

add_test(NAME ${BINARY} COMMAND ${BINARY})

include_directories(${CMAKE_SOURCE_DIR})

target_link_libraries(${BINARY} PUBLIC containers benchmark::benchmark)
