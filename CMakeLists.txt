cmake_minimum_required(VERSION 2.6)
project(kangaru-example)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
add_executable(kangaru-example example/kangaru-example.cpp)

install(TARGETS kangaru-example RUNTIME DESTINATION bin)
