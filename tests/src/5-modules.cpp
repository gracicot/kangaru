#include <fmt/core.h>

import kangaru;

auto main() -> int {
	auto source = kangaru::object_source{8};
	auto inject = kangaru::spread_injector{source};
	
	inject([](int n) {
		fmt::println("value: {}", n);
	});
}
