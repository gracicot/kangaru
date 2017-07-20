cmake_minimum_required(VERSION 2.6)
project(kangaru-examples)

set(CMAKE_CXX_STANDARD 11)

if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror")
elseif (Intel)
  # using Intel C++
elseif (MSVC)
  # using Visual Studio C++
endif()

add_subdirectory(example1)
add_subdirectory(example2)
add_subdirectory(example3)
add_subdirectory(example4)
add_subdirectory(example5)
add_subdirectory(example6)
add_subdirectory(example7)
add_subdirectory(example8)
add_subdirectory(cars)
