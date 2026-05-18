cmake_minimum_required (VERSION 4.1)
project (vix_tree_example)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

add_compile_options (-Wall -Wextra -Werror -pedantic-errors -Werror=pedantic)

add_executable (set_and_map set_and_map/main.cpp set_and_map/set.hpp)
target_include_directories (set_and_map PUBLIC ../include/)
target_include_directories (set_and_map PUBLIC set_and_map/)

add_executable (basics basics/main.cpp)
target_include_directories (basics PUBLIC ../include/)

add_executable (expression_tree expression_tree/main.cpp)
target_include_directories (expression_tree PUBLIC ../include/)
target_include_directories (expression_tree PUBLIC expression_tree/)
target_compile_options (expression_tree PRIVATE -Wno-c99-designator)
