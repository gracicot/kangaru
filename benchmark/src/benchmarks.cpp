#include <benchmark/benchmark.h>
#include <kangaru/kangaru.hpp>

template<std::size_t nth, std::size_t size>
struct Service1 {
	std::array<char, size> data;
};

template<std::size_t nth, std::size_t size>
struct Definition1 : kgr::single_service<Service1<nth, size>> {};

template<std::size_t size, std::size_t... S>
static void service_bench(kgr::detail::seq<S...>, benchmark::State& state) {
	using unpack = int[];
	for (auto _ : state) {
		kgr::container container;
		(void) unpack{(
			container.service<Definition1<S, size>>()
		, 0)...};
	}
}

template<std::size_t amount, std::size_t size>
static void service_bench(benchmark::State& state) {
	service_bench<size>(typename kgr::detail::seq_gen<amount>::type{}, state);
}

BENCHMARK_TEMPLATE(service_bench, 8, 1);
BENCHMARK_TEMPLATE(service_bench, 16, 1);
BENCHMARK_TEMPLATE(service_bench, 32, 1);
BENCHMARK_TEMPLATE(service_bench, 64, 1);
BENCHMARK_TEMPLATE(service_bench, 128, 1);

BENCHMARK_TEMPLATE(service_bench, 8, 8);
BENCHMARK_TEMPLATE(service_bench, 16, 8);
BENCHMARK_TEMPLATE(service_bench, 32, 8);
BENCHMARK_TEMPLATE(service_bench, 64, 8);
BENCHMARK_TEMPLATE(service_bench, 128, 8);

BENCHMARK_TEMPLATE(service_bench, 8, 16);
BENCHMARK_TEMPLATE(service_bench, 16, 16);
BENCHMARK_TEMPLATE(service_bench, 32, 16);
BENCHMARK_TEMPLATE(service_bench, 64, 16);
BENCHMARK_TEMPLATE(service_bench, 128, 16);

template<std::size_t size, std::size_t... S>
static void emplace_bench(kgr::detail::seq<S...>, benchmark::State& state) {
	using unpack = int[];
	for (auto _ : state) {
		kgr::container container;
		(void) unpack{(
			container.emplace<Definition1<S, size>>()
		, 0)...};
	}
}

template<std::size_t amount, std::size_t size>
static void emplace_bench(benchmark::State& state) {
	emplace_bench<size>(typename kgr::detail::seq_gen<amount>::type{}, state);
}

BENCHMARK_TEMPLATE(emplace_bench, 8, 1);
BENCHMARK_TEMPLATE(emplace_bench, 16, 1);
BENCHMARK_TEMPLATE(emplace_bench, 32, 1);
BENCHMARK_TEMPLATE(emplace_bench, 64, 1);
BENCHMARK_TEMPLATE(emplace_bench, 128, 1);

BENCHMARK_TEMPLATE(emplace_bench, 8, 8);
BENCHMARK_TEMPLATE(emplace_bench, 16, 8);
BENCHMARK_TEMPLATE(emplace_bench, 32, 8);
BENCHMARK_TEMPLATE(emplace_bench, 64, 8);
BENCHMARK_TEMPLATE(emplace_bench, 128, 8);

BENCHMARK_TEMPLATE(emplace_bench, 8, 16);
BENCHMARK_TEMPLATE(emplace_bench, 16, 16);
BENCHMARK_TEMPLATE(emplace_bench, 32, 16);
BENCHMARK_TEMPLATE(emplace_bench, 64, 16);
BENCHMARK_TEMPLATE(emplace_bench, 128, 16);

BENCHMARK_MAIN();
