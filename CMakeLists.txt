CMAKE_MINIMUM_REQUIRED(VERSION 3.11)
project(caches)

set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# add_compile_options("-fsanitize=address")

add_executable(run_cache src/main.cpp)
target_include_directories(run_cache PRIVATE include/)

find_library(GTEST_LIBRARY NAMES gtest gtest_main)

add_executable(run_test src/test.cpp)
target_include_directories(run_test PRIVATE include/)
target_link_libraries(run_test ${GTEST_LIBRARY})
