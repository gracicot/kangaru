#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

struct injected1 { int value; };
struct injected2 { int value; };
struct injected3 { int value; };

struct source_of_1_and_2 {
	template<kangaru::injectable T>
		requires(std::same_as<injected1, T> or std::same_as<injected2, T>)
	constexpr auto provide() const {
		return T{value};
	}
	
	int value;
};

struct injected2_poison {
	template<kangaru::injectable T>
	constexpr auto provide() const {
		static_assert(not std::same_as<injected2, T>);
		return T{value};
	}
	
	int value;
};

struct source1 {
	constexpr auto provide() const {
		return injected1{value};
	}
	
	int value;
};

struct source2 {
	constexpr auto provide() const {
		return injected2{value};
	}
	
	int value;
};

TEST_CASE("Source wrappers", "[source]") {
	SECTION("with_alternative") {
		auto source = kangaru::with_alternative{
			source1{1},
			source2{2},
		};
		
		CHECK(kangaru::provide<injected1>(source).value == 1);
		CHECK(kangaru::provide<injected2>(source).value == 2);
		
		SECTION("Short circuiting behaviour") {
			auto source = kangaru::with_alternative{
				source_of_1_and_2{3},
				injected2_poison{4},
			};
			
			CHECK(kangaru::provide<injected1>(source).value == 3);
			CHECK(kangaru::provide<injected2>(source).value == 3);
			CHECK(kangaru::provide<injected3>(source).value == 4);
		}
	}
	
	SECTION("with_filter") {
		auto source = kangaru::make_source_with_filter<injected2>(injected2_poison{1});
		CHECK(kangaru::provide<injected1>(source).value == 1);
		static_assert(not kangaru::source_of<decltype(source), injected2>);
		CHECK(kangaru::provide<injected3>(source).value == 1);
	}
	
	SECTION("with_filter_if") {
		auto source = kangaru::make_source_with_filter_if(
			injected2_poison{1},
			[]<kangaru::injectable T>() { return not std::same_as<injected2, T>; }
		);
		
		CHECK(kangaru::provide<injected1>(source).value == 1);
		static_assert(not kangaru::source_of<decltype(source), injected2>);
		CHECK(kangaru::provide<injected3>(source).value == 1);
	}
	
	SECTION("with_passthrough") {
		auto source = kangaru::make_source_with_source_wrapping(
			kangaru::make_source_with_source_wrapping(
				kangaru::make_source_with_filter_if(
					source2{23},
					[]<kangaru::injectable T>() { return not std::same_as<injected2, T>; }
				)
			)
		);
		
		static_assert(not kangaru::source_of<decltype(source), injected2>);
		static_assert(not kangaru::source_of<kangaru::with_passthrough<0, decltype(source)>, injected2>);
		static_assert(not kangaru::source_of<kangaru::with_passthrough<1, decltype(source)>, injected2>);
		static_assert(not kangaru::source_of<kangaru::with_passthrough<2, decltype(source)>, injected2>);
		static_assert(kangaru::source_of<kangaru::with_passthrough<3, decltype(source)>, injected2>);
		static_assert(not kangaru::source_of<kangaru::with_passthrough<4, decltype(source)>, injected2>);
		
		CHECK(kangaru::provide<injected2>(kangaru::make_source_with_passthrough<3>(source)).value == 23);
	}
	
	SECTION("with_dereference") {
		auto source = kangaru::with_dereference{
			kangaru::compose(
				kangaru::pointer_source{injected1{12}},
				kangaru::reference_source{injected2{13}}
			),
		};
		
		static_assert(not kangaru::source_of<decltype(source), injected2&>);
		CHECK(kangaru::provide<injected1&>(source).value == 12);
	}
	
	SECTION("with_cast_from") {
		
	}
	
	SECTION("with_source_wrapping") {
		
	}
	
	SECTION("with_provide_using_source") {
		
	}
	
	SECTION("with_transformed_source") {
		auto source = kangaru::with_transformed_source{
			injected2_poison{9},
			[](kangaru::forwarded_source auto&& source) {
				static_assert(std::same_as<injected2_poison&, decltype(source)>);
				return source_of_1_and_2{42};
			}
		};
		
		CHECK(kangaru::provide<injected2>(source).value == 42);
		
		SECTION("Forwards value category") {
			auto source = kangaru::with_transformed_source{
				source_of_1_and_2{1},
				[](kangaru::forwarded_source auto&& source) {
					static_assert(std::same_as<source_of_1_and_2&&, decltype(source)>);
					return source_of_1_and_2{4};
				}
			};
			
			CHECK(kangaru::provide<injected2>(std::move(source)).value == 4);
		}
	}
}
