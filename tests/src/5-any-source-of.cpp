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

struct many_source {
	template<std::integral T>
	auto provide() -> T {
		return static_cast<T>(value);
	}
	
	std::int64_t value;
};

struct immovable_source {
	immovable_source() = default;
	immovable_source(immovable_source&&) = delete;
	auto operator=(immovable_source&&) -> immovable_source& = delete;
	immovable_source(immovable_source const&) = delete;
	auto operator=(immovable_source const&) -> immovable_source& = delete;
};

TEST_CASE("any_source_of type erase sources", "[any_source_of]") {
	SECTION("Wraps none") {
		auto source1 = kangaru::any_source_of<>{kangaru::none_source{}};
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
			REQUIRE(kangaru::provide<int>(source) == 9);
			REQUIRE(kangaru::provide<float>(source) == 9.5f);
		}
	}
	
	SECTION("Can be initialized using a lambda") {
		auto source = kangaru::any_source_of<int>{[]{ return int_source{9}; }};
		REQUIRE(kangaru::provide<int>(source) == 9);

		SECTION("Can be used to initialize immovable sources") {
			// TODO: Fix ambiguity
			// auto source = kangaru::any_source_of<>{[]{ return immovable_source{}; }};
		}
	}
	
	SECTION("Can wrap many") {
		auto source = kangaru::any_source_of<
			std::uint8_t,
			std::int8_t,
			std::uint16_t,
			std::int16_t,
			std::uint32_t,
			std::int32_t,
			std::uint64_t,
			std::int64_t
		>{many_source{32}};
		
		static_assert(sizeof(source) == sizeof(void*) * 2);
	}
}
