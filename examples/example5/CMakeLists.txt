cmake_minimum_required(VERSION 2.6)
project(example5)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror")
add_executable(example5 example5.cpp)
include_directories(${PROJECT_SOURCE_DIR}/../../include/kangaru)
