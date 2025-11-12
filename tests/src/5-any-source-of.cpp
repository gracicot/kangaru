#include <kangaru/kangaru.hpp>

#include <catch2/catch_test_macros.hpp>

struct int_source {
	auto provide() -> int {
		return value;
	}
	
	int value;
};

struct int_float_source {
	template<typename T> requires (std::same_as<int, T> or std::same_as<float, T>)
	auto provide() -> T {
		return static_cast<T>(value);
	}
	
	float value;
};

struct int_float_source_type2 {
	template<typename T> requires (std::same_as<int, T> or std::same_as<float, T>)
	auto provide() -> T {
		return static_cast<T>(value);
	}
	
	float value;
};

TEST_CASE("any_source_of type erase sources", "[any_source_of]") {
	SECTION("Wraps none") {
		auto source1 = kangaru::any_source_of<>{kangaru::none_source{}};
		auto source2 = kangaru::any_source_of{kangaru::none_source{}};
	}
	
	SECTION("Can wrap one") {
		auto source = kangaru::any_source_of<int>{int_source{9}};
		REQUIRE(kangaru::provide<int>(source) == 9);
	}
	
	SECTION("Can wrap two") {
		auto source = kangaru::any_source_of<int, float>{int_float_source{9.5f}};
		REQUIRE(kangaru::provide<int>(source) == 9);
		REQUIRE(kangaru::provide<float>(source) == 9.5f);

		SECTION("Can reassign to another type") {
			source = int_float_source_type2{6.5f};
			REQUIRE(kangaru::provide<int>(source) == 6);
			REQUIRE(kangaru::provide<float>(source) == 6.5f);
		}

		SECTION("Can reassign to another any_source_of") {
			auto source2 = kangaru::any_source_of<int, float>{int_float_source_type2{9.5f}};
			source = std::move(source2);
			REQUIRE(kangaru::provide<int>(source) == 6);
			REQUIRE(kangaru::provide<float>(source) == 6.5f);
		}
	}
}
