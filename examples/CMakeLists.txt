cmake_minimum_required(VERSION 2.6)
project(kangaru-examples)

if (Clang)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Werror -std=c++11")
elseif (GNU)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wsuggest-override -Wmain -Werror -std=c++11")
elseif (Intel)
  # using Intel C++
elseif (MSVC)
  # using Visual Studio C++
endif()

include_directories(${PROJECT_SOURCE_DIR}/../../include)

add_subdirectory(example1)
add_subdirectory(example2)
add_subdirectory(example3)
add_subdirectory(example4)
add_subdirectory(example5)
add_subdirectory(example6)
add_subdirectory(example7)
add_subdirectory(example8)
add_subdirectory(cars)
