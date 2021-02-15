cmake_minimum_required(VERSION 3.17)
project(LFStack)

set(CMAKE_CXX_STANDARD 20)
set(GCC_COVERAGE_COMPILE_FLAGS "-pthread")
set(CMAKE_CXX_FLAGS "${GCC_COVERAGE_COMPILE_FLAGS}" )
add_executable(LFStack lock_free_stack.hpp tests.cpp)