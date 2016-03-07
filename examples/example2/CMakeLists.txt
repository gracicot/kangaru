cmake_minimum_required(VERSION 2.6)
project(example2)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Werror")
add_executable(example2 example2.cpp)
include_directories(${PROJECT_SOURCE_DIR}/../../include/kangaru)
