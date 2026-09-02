#include "kangaru/detail/utility.hpp"
#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
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

struct a {};
struct b {};

struct convertible_a {
	constexpr operator a() {
		return a{};
	}
};

struct convertible_b {
	constexpr operator b() {
		return b{};
	}
};

struct convertible_a_b {
	constexpr operator a() {
		which = 1;
		return a{};
	}
	
	constexpr operator b() {
		which = 2;
		return b{};
	}

	int which;
};

struct int_source {
	constexpr auto provide() const {
		return 1;
	}
};

template<kangaru::injectable T>
struct map1 {};

template<>
struct map1<int> {
	using type = int_source;
};

template<kangaru::injectable T>
using map1_map = typename map1<T>::type;

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
		SECTION("Simple conversion") {
			auto source = kangaru::make_source_with_cast_from<convertible_a>(kangaru::object_source{convertible_a{}});
			static_assert(kangaru::source_of<decltype(source), a>);
		}
		
		SECTION("Multi conversion") {
			auto source = kangaru::make_source_with_cast_from<convertible_a_b&>(kangaru::reference_source{convertible_a_b{}});
			static_assert(kangaru::source_of<decltype(source), a>);
			static_assert(kangaru::source_of<decltype(source), b>);
			(void) kangaru::provide<a>(source);
			REQUIRE(source.source.provide().which == 1);
			(void) kangaru::provide<b>(source);
			REQUIRE(source.source.provide().which == 2);
		}
		
		SECTION("Conversion from multiple types") {
			auto source = kangaru::make_source_with_cast_from<convertible_a, convertible_b&>(
				kangaru::compose(
					kangaru::object_source{convertible_a{}},
					kangaru::reference_source{convertible_b{}}
				)
			);
			static_assert(kangaru::source_of<decltype(source), a>);
			static_assert(kangaru::source_of<decltype(source), b>);
		}
		
		SECTION("Protection against materialized temporaries") {
			auto source = kangaru::make_source_with_cast_from<int&&>(
				kangaru::rvalue_source{42}
			);
			
			static_assert(kangaru::source_of<decltype(source), int&&>);
			static_assert(not kangaru::source_of<decltype(source), int&>);
			static_assert(kangaru::source_of<decltype(source), int const&>);
			static_assert(kangaru::source_of<decltype(source), float>);
			static_assert(not kangaru::source_of<decltype(source), float&&>);
			static_assert(not kangaru::source_of<decltype(source), float const&>);
			
			REQUIRE(kangaru::provide<int>(source) == 42);
			REQUIRE(kangaru::provide<int&&>(source) == 42);
			REQUIRE(kangaru::provide<float>(source) == 42.f);
			REQUIRE(kangaru::provide<int const&>(source) == 42);
		}
	}
	
	SECTION("with_source_wrapping") {
		auto source = kangaru::with_source_wrapping{
			kangaru::object_source<kangaru::object_source<int>>{
				kangaru::object_source{42}
			}
		};
		
		REQUIRE(kangaru::provide<int>(kangaru::provide<kangaru::basic_wrapping_source<kangaru::object_source<int>>>(source).source) == 42);
		static_assert(not kangaru::source_of<decltype(source), kangaru::sealed_source<kangaru::object_source<int>>>);
	}
	
	SECTION("with_provide_using_source") {
		auto source = kangaru::make_source_with_provide_using_source<map1_map>(
			kangaru::object_source{int_source{}}
		);
		
		// Provides using the int source given by the object source
		REQUIRE(kangaru::provide<int>(source) == 1);
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

struct source_of_int_value_cat_source {
	kangaru::object_source<int> source;
	
	constexpr auto provide() & -> kangaru::object_source<int> {
		return kangaru::object_source{1};
	}
	
	constexpr auto provide() && -> kangaru::object_source<int> {
		return kangaru::object_source{2};
	}
	
	constexpr auto provide() const& -> kangaru::object_source<int> {
		return kangaru::object_source{3};
	}
	
	constexpr auto provide() const&& -> kangaru::object_source<int> {
		return kangaru::object_source{4};
	}
};

template<kangaru::injectable T>
struct map2 {};

template<>
struct map2<int> {
	using type = kangaru::object_source<int>;
};

template<kangaru::injectable T>
using map2_map = typename map2<T>::type;

template<kangaru::injectable T>
struct map3 {};

template<>
struct map3<int> {
	using type = kangaru::basic_wrapping_source<kangaru::object_source<int>>;
};

template<kangaru::injectable T>
using map3_map = typename map3<T>::type;

struct test_with_alternative {
	static constexpr auto make_source() {
		return kangaru::with_alternative{
			value_category_source{},
			kangaru::object_source{injected1{}},
		};
	}
};

struct test_with_alternative_alternative {
	static constexpr auto make_source() {
		return kangaru::with_alternative{
			kangaru::object_source{injected1{}},
			value_category_source{},
		};
	}
};

struct test_with_filter {
	static constexpr auto make_source() {
		return kangaru::with_filter{value_category_source{}};
	}
};

struct test_with_filter_if {
	static constexpr auto make_source() {
		return kangaru::with_filter_if{
			value_category_source{},
			[]<kangaru::injectable>(){ return true; }
		};
	}
};

struct test_with_passthrough {
	static constexpr auto make_source() {
		return kangaru::make_source_with_passthrough<2>(
			value_category_source{}
		);
	}
};

struct test_with_passthrough_passthrough {
	static constexpr auto make_source() {
		return kangaru::make_source_with_passthrough<1>(
			kangaru::make_source_with_filter_if(
				value_category_source{},
				[]<kangaru::injectable>() { return false; }
			)
		);
	}
};

template<kangaru::source Source>
struct with_heap_storage_mutable {
	Source source;
	
	template<kangaru::pointer T, kangaru::forwarded<with_heap_storage_mutable> Self> requires kangaru::wrapping_source_of<Self, std::remove_pointer_t<T>>
	constexpr friend auto provide(Self&& source) -> T {
		return source.storage.template emplace<std::remove_pointer_t<T>>(
			kangaru::in_place_construct{[&source] {
				return kangaru::provide<std::remove_pointer_t<T>>(std::forward<decltype(source)>(source).source);
			}}
		);
	}
	
	mutable kangaru::default_heap_storage storage;
};

struct test_with_dereference_with_cast_from {
	static auto make_source() {
		return kangaru::make_source_with_cast_from<int&>(
			kangaru::with_dereference{
				with_heap_storage_mutable{
					value_category_source{},
				},
			}
		);
	}
};

struct test_with_cast_from_passthrough {
	static constexpr auto make_source() {
		return kangaru::make_source_with_cast_from<float>(
			value_category_source{}
		);
	}
};

struct value_cat_object_source_of_int {
	value_category_source source;

	template<kangaru::forwarded<value_cat_object_source_of_int> Self>
	friend constexpr auto provide(Self&& source) -> kangaru::object_source<int> {
		return kangaru::object_source{kangaru::provide<int>(std::forward<decltype(source)>(source).source)};
	}
};

struct test_with_source_wrapping {
	static constexpr auto make_source() {
		return kangaru::make_source_with_provide_using_source<map3_map>(
			kangaru::with_source_wrapping{
				source_of_int_value_cat_source{}
			}
		);
	}
};

struct test_with_provide_using_source {
	static constexpr auto make_source() {
		return kangaru::make_source_with_provide_using_source<map2_map>(
			value_cat_object_source_of_int{}
		);
	}
};

struct test_with_transformed_source {
	static constexpr auto make_source() {
		return kangaru::with_transformed_source{
			kangaru::none_source{},
			[](kangaru::forwarded_source auto&& ss) -> auto&& {
				static value_category_source s{};
				if constexpr (std::is_lvalue_reference_v<decltype(ss)>) {
					if constexpr (std::is_const_v<std::remove_reference_t<decltype(ss)>>) {
						return std::as_const(s);
					} else {
						return s;
					}
				} else {
					if constexpr (std::is_const_v<std::remove_reference_t<decltype(ss)>>) {
						return std::move(std::as_const(s));
					} else {
						return std::move(s);
					}
				}
			}
		};
	}
};

struct test_sealed_source {
	static constexpr auto make_source() {
		return kangaru::sealed_source{
			value_category_source{}
		};
	}
};

struct test_basic_wrapping_source {
	static constexpr auto make_source() {
		return kangaru::basic_wrapping_source{
			value_category_source{}
		};
	}
};

TEMPLATE_TEST_CASE("Forwards the wrapped source properly", "[source]",
	(test_with_alternative),
	(test_with_alternative_alternative),
	(test_with_filter),
	(test_with_filter_if),
	(test_with_passthrough),
	(test_with_passthrough_passthrough),
	(test_with_dereference_with_cast_from),
	(test_with_cast_from_passthrough),
	(test_with_source_wrapping),
	(test_with_provide_using_source),
	(test_with_transformed_source),
	(test_sealed_source),
	(test_basic_wrapping_source)
) {
	auto source = TestType::make_source();
	
	CHECK(kangaru::provide<int>(source) == 1);
	CHECK(kangaru::provide<int>(std::move(source)) == 2);
	CHECK(kangaru::provide<int>(std::as_const(source)) == 3);
	CHECK(kangaru::provide<int>(std::move(std::as_const(source))) == 4);
}
