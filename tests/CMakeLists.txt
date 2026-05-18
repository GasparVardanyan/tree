cmake_minimum_required (VERSION 4.1)
project (vix_tree_example)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

set (CMAKE_CXX_STANDARD 20)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_CXX_EXTENSIONS OFF)

add_compile_options (-Wall -Wextra -Werror -pedantic-errors -Werror=pedantic)

find_package(GTest REQUIRED)
enable_testing ()



add_executable (binary_tree_node_test binary_tree_node_test.cpp)
target_include_directories (binary_tree_node_test PUBLIC ../include/)
target_link_libraries (binary_tree_node_test GTest::gtest_main)

add_executable (binary_search_tree_test binary_search_tree_test.cpp)
target_include_directories (binary_search_tree_test PUBLIC ../include/)
target_link_libraries (binary_search_tree_test GTest::gtest_main)

add_executable (avl_tree_test avl_tree_test.cpp)
target_include_directories (avl_tree_test PUBLIC ../include/)
target_link_libraries (avl_tree_test GTest::gtest_main)

add_executable (splay_tree_test splay_tree_test.cpp)
target_include_directories (splay_tree_test PUBLIC ../include/)
target_link_libraries (splay_tree_test GTest::gtest_main)


include (GoogleTest)

gtest_discover_tests (binary_search_tree_test)
gtest_discover_tests (binary_tree_node_test)
gtest_discover_tests (avl_tree_test)
gtest_discover_tests (splay_tree_test)
