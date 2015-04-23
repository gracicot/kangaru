cmake_minimum_required(VERSION 2.6)
project(cars-example)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror")
add_executable(cars-example cars-example.cpp)
include_directories(${PROJECT_SOURCE_DIR}/../../include/kangaru)

install(TARGETS cars-example RUNTIME DESTINATION bin)
