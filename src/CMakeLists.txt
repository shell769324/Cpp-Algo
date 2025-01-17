add_library(containers INTERFACE)
target_include_directories(containers INTERFACE ${CMAKE_SOURCE_DIR})
add_executable(cpp_algo_src main.cpp)
target_link_libraries(cpp_algo_src containers)
