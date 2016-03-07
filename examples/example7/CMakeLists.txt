cmake_minimum_required(VERSION 2.6)
project(example7)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Werror")
add_executable(example7 example7.cpp)
include_directories(${PROJECT_SOURCE_DIR}/../../include/kangaru)
