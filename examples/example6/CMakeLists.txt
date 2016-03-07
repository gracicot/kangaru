cmake_minimum_required(VERSION 2.6)
project(example6)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wall -Werror")
add_executable(example6 example6.cpp)
include_directories(${PROJECT_SOURCE_DIR}/../../include/kangaru)
