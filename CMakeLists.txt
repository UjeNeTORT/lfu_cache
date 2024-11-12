CMAKE_MINIMUM_REQUIRED(VERSION 3.11)
project(caches)

set(CMAKE_CXX_STANDARD_REQUIRED 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options("-fsanitize=address")
add_link_options("-fsanitize=address")

add_executable(testgen src/testgen.cpp)

add_executable(cache src/main.cpp)
target_include_directories(cache PRIVATE include/)

find_library(GTEST_LIBRARY NAMES gtest gtest_main)

add_executable(test src/test.cpp)
target_include_directories(test PRIVATE include/)
target_link_libraries(test ${GTEST_LIBRARY})
