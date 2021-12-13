set(BINARY ${CMAKE_PROJECT_NAME}_tst)

# file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false utility/* main.cpp stack_test.cpp)

file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.h *.cpp)


set(SOURCES ${TEST_SOURCES})
message(STATUS "names of binary files: ${TEST_SOURCES}")

add_executable(${BINARY} ${TEST_SOURCES})

add_test(NAME ${BINARY} COMMAND ${BINARY})


target_link_libraries(${BINARY} PUBLIC containers gtest)
