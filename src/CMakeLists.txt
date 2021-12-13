set(BINARY ${CMAKE_PROJECT_NAME}_run)

file(GLOB_RECURSE SOURCES LIST_DIRECTORIES true vector.h stack.h tree_common.h avl_tree.h main.cpp stub.h int_stub.h common.h)

set(SOURCES ${SOURCES})

add_executable(${BINARY} ${SOURCES})

add_library(containers ${SOURCES})
