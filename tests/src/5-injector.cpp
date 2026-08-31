#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

struct injected1 {};
struct injected2 {};

struct needs_int {
	int a;
};

TEST_CASE("Injectors can compose", "[injector]") {
	auto injected1_value = kangaru::object_source{injected1{}};
	auto injected2_value = kangaru::object_source{injected2{}};
	
	SECTION("Simple injector") {
		auto injector = kangaru::make_simple_injector(injected1_value);
		
		int result = injector([](injected1) {
			return 42;
		});
		
		REQUIRE(result == 42);
		
		auto const function1 = [](injected1) {};
		auto const function2 = [](injected2) {};
		auto const function3 = [](injected1, injected1) {};
		auto const function4 = []() {};
		
		REQUIRE(not std::invocable<decltype(injector), decltype(function4)>);
		REQUIRE(std::invocable<decltype(injector), decltype(function1)>);
		REQUIRE(not std::invocable<decltype(injector), decltype(function2)>);
		REQUIRE(not std::invocable<decltype(injector), decltype(function3)>);
	}
	
	SECTION("Spread injector chooses function it can call") {
		struct function_t {
			auto operator()(injected1) { return 1; }
			auto operator()(injected2, injected2) { return 2; }
		};
		
		auto injector = kangaru::make_spread_injector(injected1_value);
		static_assert(not kangaru::callable<decltype(injector), function_t>);
	}
	
	SECTION("Spread injector chooses function it can call with empty fallback") {
		struct function_t {
			auto operator()() { return 1; }
			auto operator()(injected2, injected2) { return 2; }
		};
		
		auto injector = kangaru::make_spread_injector(injected1_value);
		static_assert(not kangaru::callable<decltype(injector), function_t>);
	}
	
	SECTION("Spread injector chooses one constructor function no matter the source") {
		auto injector1 = kangaru::make_spread_injector(kangaru::object_source{42});
		auto injector2 = kangaru::make_spread_injector(kangaru::none_source{});
		static_assert(kangaru::callable<decltype(injector1), kangaru::constructor_function<needs_int>>);
		static_assert(not kangaru::callable<decltype(injector2), kangaru::constructor_function<needs_int>>);
	}
	
	SECTION("Spread injector") {
		auto injector = kangaru::make_spread_injector(kangaru::tie(injected1_value, injected2_value));
		
		REQUIRE(42 == injector([](injected1) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](injected2) {
			return 42;
		}));
		
		REQUIRE(42 == injector([]() {
			return 42;
		}));
		
		REQUIRE(42 == injector([](injected1, injected1) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](injected1, injected2) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](injected2, injected1) {
			return 42;
		}));
	}
	
	SECTION("Composed simple then spread injector") {
		auto injector = kangaru::concat(
			kangaru::make_simple_injector(injected1_value),
			kangaru::make_composable_spread_injector(injected2_value)
		);
		
		REQUIRE(42 == injector([](injected1) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](injected1, injected2) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](injected1, injected2, injected2) {
			return 42;
		}));
		
		auto const function1 = [](injected2) {};
		auto const function2 = [](injected1, injected1) {};
		auto const function3 = [](injected1, injected2, injected1) {};
		
		REQUIRE(not std::invocable<decltype(injector), decltype(function1)>);
		REQUIRE(not std::invocable<decltype(injector), decltype(function2)>);
		REQUIRE(not std::invocable<decltype(injector), decltype(function3)>);
	}
	
	SECTION("Composed spread then simple injector") {
		auto injector = kangaru::concat(
			kangaru::make_composable_spread_injector(injected2_value),
			kangaru::make_simple_injector(injected1_value)
		);
		
		REQUIRE(42 == injector([](injected1) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](injected2, injected1) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](injected2, injected2, injected1) {
			return 42;
		}));
		
		auto const function1 = [](injected1, injected2) {};
		
		REQUIRE(not std::invocable<decltype(injector), decltype(function1)>);
	}
	
	SECTION("Composed spread then spread injector") {
		auto injector = kangaru::concat(
			kangaru::make_composable_spread_injector(injected2_value),
			kangaru::make_composable_spread_injector(injected1_value)
		);
		
		REQUIRE(42 == injector([](injected1) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](injected1, injected1) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](injected1, injected1) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](injected1, injected1, injected1) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](injected2) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](injected2, injected1) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](injected2, injected1, injected1) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](injected2, injected2, injected1, injected1) {
			return 42;
		}));
		
		REQUIRE(42 == injector([](injected2, injected2, injected2, injected1, injected1) {
			return 42;
		}));
		
		auto const function1 = [](injected1, injected2) {};
		
		REQUIRE(not std::invocable<decltype(injector), decltype(function1)>);
	}
}
