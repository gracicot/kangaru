find_package(benchmark REQUIRED)

add_executable(kangaru_benchmarks src/benchmarks.cpp)
target_link_libraries(kangaru_benchmarks PRIVATE kangaru::kangaru)
target_compile_features(kangaru_benchmarks PRIVATE cxx_std_11)
target_link_libraries(kangaru_benchmarks PRIVATE benchmark)
