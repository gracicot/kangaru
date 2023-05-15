#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

struct sleepy {};
struct grumpy {};

TEST_CASE("Injectors can compose", "[injector]") {
	auto sleepy_value = kangaru::object_source{sleepy{}};
	auto grumpy_value = kangaru::object_source{grumpy{}};
	
	SECTION("Simple injector") {
		auto injector = kangaru::simple_injector{sleepy_value};
		
		int result = injector([](sleepy) {
			return 42;
		});
		
		REQUIRE(result == 42);
		
		auto const function1 = [](sleepy) {};
		auto const function2 = [](grumpy) {};
		auto const function3 = [](sleepy, sleepy) {};
		auto const function4 = []() {};
		
		REQUIRE(not std::invocable<decltype(injector), decltype(function4)>);
		REQUIRE(std::invocable<decltype(injector), decltype(function1)>);
		REQUIRE(not std::invocable<decltype(injector), decltype(function2)>);
		REQUIRE(not std::invocable<decltype(injector), decltype(function3)>);
	}
	
	SECTION("Spread injector") {
		auto injector = kangaru::spread_injector<decltype(kangaru::tie(sleepy_value, grumpy_value))>{kangaru::tie(sleepy_value, grumpy_value)};
		
		REQUIRE(42 == injector([](sleepy) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](grumpy) {
			return 42;
		}));
		
		REQUIRE(42 == injector([]() {
			return 42;
		}));
		
		REQUIRE(42 == injector([](sleepy, sleepy) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](sleepy, grumpy) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](grumpy, sleepy) {
			return 42;
		}));
	}
	
	SECTION("Composed simple then spread injector") {
		auto injector = kangaru::compose(
			kangaru::simple_injector{sleepy_value},
			kangaru::spread_injector<decltype(grumpy_value)>{grumpy_value}
		);
		
		REQUIRE(42 == injector([](sleepy) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](sleepy, grumpy) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](sleepy, grumpy, grumpy) {
			return 42;
		}));
		
		auto const function1 = [](grumpy) {};
		auto const function2 = [](sleepy, sleepy) {};
		auto const function3 = [](sleepy, grumpy, sleepy) {};
		
		REQUIRE(not std::invocable<decltype(injector), decltype(function1)>);
		REQUIRE(not std::invocable<decltype(injector), decltype(function2)>);
		REQUIRE(not std::invocable<decltype(injector), decltype(function3)>);
	}
	
	SECTION("Composed spread then simple injector") {
		auto injector = kangaru::compose(
			kangaru::spread_injector<decltype(grumpy_value)>{grumpy_value},
			kangaru::simple_injector{sleepy_value}
		);
		
		REQUIRE(42 == injector([](sleepy) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](grumpy, sleepy) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](grumpy, grumpy, sleepy) {
			return 42;
		}));
		
		auto const function1 = [](sleepy, grumpy) {};
		
		REQUIRE(not std::invocable<decltype(injector), decltype(function1)>);
	}
	
	SECTION("Composed spread then spread injector") {
		auto injector = kangaru::compose(
			kangaru::spread_injector<decltype(grumpy_value)>{grumpy_value},
			kangaru::spread_injector<decltype(sleepy_value)>{sleepy_value}
		);
		
		REQUIRE(42 == injector([](sleepy) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](sleepy, sleepy) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](sleepy, sleepy) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](sleepy, sleepy, sleepy) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](grumpy) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](grumpy, sleepy) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](grumpy, sleepy, sleepy) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](grumpy, grumpy, sleepy, sleepy) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](grumpy, grumpy, grumpy, sleepy, sleepy) {
			return 42;
		}));
		
		auto const function1 = [](sleepy, grumpy) {};
		
		REQUIRE(not std::invocable<decltype(injector), decltype(function1)>);
	}
}
