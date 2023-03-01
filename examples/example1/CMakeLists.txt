add_executable(example1 example1.cpp)

find_package(fmt REQUIRED)

target_link_libraries(example1 PUBLIC kangaru-example fmt::fmt)
