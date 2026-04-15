#include <kangaru/kangaru.hpp>

#include <catch2/catch_test_macros.hpp>

struct int_source {
	auto provide() -> int {
		return value;
	}
	
	int value;
};

struct value_category_source {
	auto provide() & -> int {
		return 1;
	}

	auto provide() && -> int {
		return 2;
	}
	auto provide() const& -> int {
		return 3;
	}

	auto provide() const&& -> int {
		return 4;
	}
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
		
		SECTION("Move construct and assignment") {
			auto source2 = std::move(source);
			REQUIRE(kangaru::provide<int>(source2) == 9);
			source2 = std::move(source2);
			REQUIRE(kangaru::provide<int>(source2) == 9);
			source = std::move(source2);
			REQUIRE(kangaru::provide<int>(source) == 9);
			
			// Assignment to moved-from
			source2 = int_source{5};
			REQUIRE(kangaru::provide<int>(source2) == 5);
		}
		
		source = value_category_source{};
		CHECK(kangaru::provide<int>(source) == 1);
		
		// any_source_of has value semantics, but does not do forwarded to the wrapped object.
		CHECK(kangaru::provide<int>(std::move(source)) == 1);
	}
	
	SECTION("Properly destroy") {
		static int count = 0;
		
		struct counting_source {
			counting_source() = default;
			auto operator=(counting_source&&) -> counting_source& = default;
			auto operator=(counting_source const&) -> counting_source& = default;
			counting_source(counting_source&&) = default;
			counting_source(counting_source const&) = default;
			
			~counting_source() {
				++count;
			}
		};
		
		{
			auto source = kangaru::any_source_of<>(counting_source{});
		}
		
		CHECK(count == 2);
		
		{
			auto source = kangaru::any_source_of<>(counting_source{});
		}
		
		CHECK(count == 4);
	}
	
	SECTION("Can wrap one rvalue and lvalue construction") {
		auto s = int_source{9};
		SECTION("non const lvalue") {
			auto source = kangaru::any_source_of<int>{s};
			REQUIRE(kangaru::provide<int>(source) == 9);
		}
		
		SECTION("const lvalue") {
			auto source = kangaru::any_source_of<int>{std::as_const(s)};
			REQUIRE(kangaru::provide<int>(source) == 9);
		}
		
		SECTION("non const rvalue") {
			auto source = kangaru::any_source_of<int>{std::move(s)};
			REQUIRE(kangaru::provide<int>(source) == 9);
		}
		
		SECTION("const rvalue") {
			auto source = kangaru::any_source_of<int>{std::move(std::as_const(s))};
			REQUIRE(kangaru::provide<int>(source) == 9);
		}
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
	
	SECTION("Can be initialized using in_place_construct") {
		auto source = kangaru::any_source_of<int>{kangaru::in_place_construct{[]{ return int_source{9}; }}};
		REQUIRE(kangaru::provide<int>(source) == 9);
		
		SECTION("Can be used to initialize immovable sources") {
			auto source = kangaru::any_source_of<>{kangaru::in_place_construct{[]{ return immovable_source{}; }}};
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
		
		CHECK(kangaru::provide<std::uint8_t>(source) == 32);
		CHECK(kangaru::provide<std::int8_t>(source) == 32);
		CHECK(kangaru::provide<std::uint16_t>(source) == 32);
		CHECK(kangaru::provide<std::int16_t>(source) == 32);
		CHECK(kangaru::provide<std::uint32_t>(source) == 32);
		CHECK(kangaru::provide<std::int32_t>(source) == 32);
		CHECK(kangaru::provide<std::uint64_t>(source) == 32);
		CHECK(kangaru::provide<std::int64_t>(source) == 32);
		
		static_assert(sizeof(source) == sizeof(void*) * 2);
	}
}

TEST_CASE("any_source_of_ref type erase non owned sources", "[any_source_of]") {
	SECTION("Wraps none") {
		auto s = kangaru::none_source{};
		auto source1 = kangaru::any_source_of_ref<>{s};
	}
	
	SECTION("Can wrap one") {
		auto s = int_source{9};
		auto source = kangaru::any_source_of_ref<int>{s};
		REQUIRE(kangaru::provide<int>(source) == 9);
		
		SECTION("Can wrap reference wrappers") {
			auto s = int_source{5};
			auto source = kangaru::any_source_of_ref<int>{kangaru::ref(s)};
			REQUIRE(kangaru::provide<int>(source) == 5);
		}
		
		SECTION("Move construct and assignment") {
			auto source2 = std::move(source);
			REQUIRE(kangaru::provide<int>(source2) == 9);
			source2 = std::move(source2);
			s = int_source{4};
			REQUIRE(kangaru::provide<int>(source2) == 4);
			source = std::move(source2);
			REQUIRE(kangaru::provide<int>(source) == 4);
			
			// Assignment to moved-from
			auto s2 = int_source{5};
			source2 = s2;
			REQUIRE(kangaru::provide<int>(source2) == 5);
		}
		
		auto s2 = value_category_source{};
		source = s2;
		CHECK(kangaru::provide<int>(source) == 1);
		
		// any_source_of_ref has reference semantics, the value category of the wrapper don't change the evaluation
		CHECK(kangaru::provide<int>(std::move(source)) == 1);
	}
	
	SECTION("Can wrap two") {
		auto s = int_float_source{9.5f};
		auto source = kangaru::any_source_of_ref<int, float>{s};
		REQUIRE(kangaru::provide<int>(source) == 9);
		REQUIRE(kangaru::provide<float>(source) == 9.5f);
		
		SECTION("Can reassign to another type") {
			auto s = int_float_source_type2{6.5f};
			source = s;
			REQUIRE(kangaru::provide<int>(source) == 6);
			REQUIRE(kangaru::provide<float>(source) == 6.5f);
		}
		
		SECTION("Can reassign to another any_source_of") {
			auto s = int_float_source_type2{9.5f};
			auto source2 = kangaru::any_source_of_ref<int, float>{s};
			source = source2;
			REQUIRE(kangaru::provide<int>(source) == 9);
			REQUIRE(kangaru::provide<float>(source) == 9.5f);
		}
	}
	
	SECTION("Can wrap many") {
		auto s = many_source{32};
		auto source = kangaru::any_source_of_ref<
			std::uint8_t,
			std::int8_t,
			std::uint16_t,
			std::int16_t,
			std::uint32_t,
			std::int32_t,
			std::uint64_t,
			std::int64_t
		>{s};
		
		CHECK(kangaru::provide<std::uint8_t>(source) == 32);
		CHECK(kangaru::provide<std::int8_t>(source) == 32);
		CHECK(kangaru::provide<std::uint16_t>(source) == 32);
		CHECK(kangaru::provide<std::int16_t>(source) == 32);
		CHECK(kangaru::provide<std::uint32_t>(source) == 32);
		CHECK(kangaru::provide<std::int32_t>(source) == 32);
		CHECK(kangaru::provide<std::uint64_t>(source) == 32);
		CHECK(kangaru::provide<std::int64_t>(source) == 32);
		
		static_assert(sizeof(source) == sizeof(void*) * 2);
	}
}
