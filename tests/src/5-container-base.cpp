#include "container_test_helper.hpp"

#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

#include <fmt/core.h>

struct dynamic_provided_abstract {
	explicit constexpr dynamic_provided_abstract(std::int32_t value) : value{value} {}
	virtual ~dynamic_provided_abstract() = 0;
	
	std::int32_t value;
	
	friend auto attribute(kangaru::assume_runtime_cached<std::shared_ptr<dynamic_provided_abstract>>) -> std::true_type;
};

dynamic_provided_abstract::~dynamic_provided_abstract() = default;

struct dynamic_provided_concrete : dynamic_provided_abstract {
	explicit constexpr dynamic_provided_concrete(std::int32_t value) : dynamic_provided_abstract{value} {}
	
	friend auto attribute(kangaru::assume_runtime_cached<std::shared_ptr<dynamic_provided_concrete>>) -> std::true_type;
	friend auto attribute(kangaru::overrides_types_in_cache<std::shared_ptr<dynamic_provided_concrete>>)
		-> std::tuple<std::shared_ptr<dynamic_provided_abstract>>;
};

auto self_contained_base(auto&& base) {
	return kangaru::with_recursion{
		kangaru::make_source_with_passthrough<1>(
			kangaru::make_source_with_provide_using_source<std::decay_t<decltype(base)>::template source_for>(
				kangaru::with_construction{
					kangaru::seal_source(kangaru::ref(base.source)),
					base.construction,
				}
			)
		),
	};
}

TEST_CASE("Container base source", "[container]") {
	SECTION("Default assume cached") {
		auto base = kangaru::make_container_base(
			kangaru::allow_assume_cached
		);
		
		CHECK_THROWS_AS(kangaru::provide<kangaru::shared_pointer_source<dynamic_provided_abstract>>(base.source), kangaru::not_found_exception);
		
		static_assert(not kangaru::source_of<decltype(base.source), std::shared_ptr<dynamic_provided_abstract>>);
		static_assert(kangaru::source_of<decltype(base.source), kangaru::shared_pointer_source<dynamic_provided_abstract>>);
		static_assert(not kangaru::source_of<decltype(base.source), int>);
	}
	
	SECTION("Assume cached with specific not found source") {
		auto base = kangaru::make_container_base(
			kangaru::allow_assume_cached,
			kangaru::throw_if_not_found{}
		);
		
		CHECK_THROWS_AS(kangaru::provide<kangaru::shared_pointer_source<dynamic_provided_abstract>>(base.source), kangaru::not_found_exception);
		
		static_assert(not kangaru::source_of<decltype(base.source), std::shared_ptr<dynamic_provided_abstract>>);
		static_assert(kangaru::source_of<decltype(base.source), kangaru::shared_pointer_source<dynamic_provided_abstract>>);
		static_assert(not kangaru::source_of<decltype(base.source), int>);
	}
	
	SECTION("Assume cached with lambdas") {
		auto base = kangaru::make_container_base(
			kangaru::allow_assume_cached,
			kangaru::throw_if_not_found{},
			[](std::shared_ptr<dynamic_provided_abstract> ptr) {
				return kangaru::object_source<mapped_value_dependent_on<std::shared_ptr<dynamic_provided_abstract>>>{ptr};
			}
		);
		
		CHECK_THROWS_AS(
			kangaru::provide<kangaru::object_source<mapped_value_dependent_on<std::shared_ptr<dynamic_provided_abstract>>>>(
				self_contained_base(base)
			),
			kangaru::not_found_exception
		);
	}
	
	SECTION("Assume cached with lambdas") {
		auto base = kangaru::make_container_base(
			[]() -> std::shared_ptr<dynamic_provided_abstract> {
				return std::make_shared<dynamic_provided_concrete>(32);
			},
			[](std::shared_ptr<dynamic_provided_abstract> ptr) {
				return kangaru::object_source<mapped_value_dependent_on<std::shared_ptr<dynamic_provided_abstract>>>{ptr};
			}
		);
		
		CHECK(
			kangaru::provide<mapped_value_dependent_on<std::shared_ptr<dynamic_provided_abstract>>>(
				kangaru::provide<kangaru::object_source<mapped_value_dependent_on<std::shared_ptr<dynamic_provided_abstract>>>>(
					self_contained_base(base)
				)
			).value->value == 32
		);
	}
	
	SECTION("Assume cached with lambdas and a source") {
		auto base = kangaru::make_container_base(
			kangaru::none_source{},
			[]() -> std::shared_ptr<dynamic_provided_abstract> {
				return std::make_shared<dynamic_provided_concrete>(32);
			},
			[](std::shared_ptr<dynamic_provided_abstract> ptr) {
				return kangaru::object_source<mapped_value_dependent_on<std::shared_ptr<dynamic_provided_abstract>>>{ptr};
			}
		);
		
		CHECK(
			kangaru::provide<mapped_value_dependent_on<std::shared_ptr<dynamic_provided_abstract>>>(
				kangaru::provide<kangaru::object_source<mapped_value_dependent_on<std::shared_ptr<dynamic_provided_abstract>>>>(
					self_contained_base(base)
				)
			).value->value == 32
		);
	}
	
	SECTION("Custom injector") {
		auto base1 = kangaru::make_container_base(
			kangaru::make_strict_spread_injector_function{},
			kangaru::none_source{},
			[] {
				return 1;
			},
			[](int&&) {
				return std::string{};
			}
		);
		
		auto base2 = kangaru::make_container_base(
			kangaru::make_strict_spread_injector_function{},
			kangaru::none_source{},
			[] {
				return 1;
			},
			[](int) {
				return std::string{};
			}
		);
		
		auto base3 = kangaru::make_container_base(
			kangaru::make_spread_injector_function{},
			kangaru::none_source{},
			[] {
				return 1;
			},
			[](int&&) {
				return std::string{};
			}
		);
		
		auto base4 = kangaru::make_container_base(
			kangaru::make_strict_spread_injector_function{},
			kangaru::allow_assume_cached,
			kangaru::throw_if_not_found{},
			kangaru::none_source{},
			[](std::shared_ptr<dynamic_provided_abstract>) {
				return 1;
			},
			[](int) {
				return std::string{};
			}
		);
		
		auto base5 = kangaru::make_container_base(
			kangaru::make_strict_spread_injector_function{},
			kangaru::allow_assume_cached,
			kangaru::throw_if_not_found{},
			kangaru::none_source{},
			[](std::shared_ptr<dynamic_provided_abstract> const&) {
				return 1;
			}
		);
		
		static_assert(kangaru::source_of<decltype(self_contained_base(base1)), int>);
		static_assert(not kangaru::source_of<decltype(self_contained_base(base1)), std::string>);
		static_assert(kangaru::source_of<decltype(self_contained_base(base2)), int>);
		static_assert(kangaru::source_of<decltype(self_contained_base(base2)), std::string>);
		static_assert(kangaru::source_of<decltype(self_contained_base(base3)), int>);
		static_assert(kangaru::source_of<decltype(self_contained_base(base3)), std::string>);
		static_assert(kangaru::source_of<decltype(self_contained_base(base4)), int>);
		static_assert(kangaru::source_of<decltype(self_contained_base(base4)), std::string>);
		static_assert(not kangaru::source_of<decltype(self_contained_base(base5)), int>);
	}
	
	SECTION("Default assume abort") {
		auto base = kangaru::make_container_base(
			kangaru::allow_assume_cached,
			kangaru::abort_if_not_found{}
		);
		
		static_assert(not kangaru::source_of<decltype(base.source), std::shared_ptr<dynamic_provided_abstract>>);
		static_assert(kangaru::source_of<decltype(base.source), kangaru::shared_pointer_source<dynamic_provided_abstract>>);
		static_assert(not kangaru::source_of<decltype(base.source), int>);
	}
	
	SECTION("Default assume terminate") {
		auto base = kangaru::make_container_base(
			kangaru::allow_assume_cached,
			kangaru::terminate_if_not_found{}
		);
		
		static_assert(not kangaru::source_of<decltype(base.source), std::shared_ptr<dynamic_provided_abstract>>);
		static_assert(kangaru::source_of<decltype(base.source), kangaru::shared_pointer_source<dynamic_provided_abstract>>);
		static_assert(not kangaru::source_of<decltype(base.source), int>);
	}
}
