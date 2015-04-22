cmake_minimum_required(VERSION 2.6)
project(kangaru-example)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -Wall -Werror")
add_executable(kangaru-example example/kangaru-example.cpp)
include_directories(${PROJECT_SOURCE_DIR}/include/kangaru)

install(TARGETS kangaru-example RUNTIME DESTINATION bin)
