#include "container_test_helper.hpp"
#include <kangaru/kangaru.hpp>
#include <catch2/catch_test_macros.hpp>
#include <type_traits>

template<typename T>
concept has_next = requires(T source) {
	{ source.next };
};

template<typename... F>
concept is_valid_incremental_source = requires{
	typename kangaru::incremental_source<F...>;
};

template<typename... F>
constexpr auto test_valid_incremental_source(F const&...) {
	return is_valid_incremental_source<F...>;
}

TEST_CASE("Incremental source", "[modular]") {
	SECTION("Construct sources from callbacks") {
		auto source = kangaru::incremental_source{
			[](kangaru::composed_source<> constructed) {
				return kangaru::object_source{42};
			},
		};
		
		static_assert(std::same_as<decltype(source.source), kangaru::object_source<int>>);
		static_assert(std::copyable<decltype(source)>);
		static_assert(std::assignable_from<decltype(source)&, decltype(source) const&>);
		static_assert(not has_next<decltype(source)>);
		static_assert(kangaru::wrapping_source<decltype(source)>);
	}
	
	SECTION("Construct sources from two") {
		auto source = kangaru::incremental_source{
			[](kangaru::composed_source<> constructed) {
				return kangaru::object_source{42};
			},
			[](kangaru::composed_source<kangaru::source_reference_wrapper<kangaru::object_source<int>>> constructed) {
				return kangaru::reference_source<unmapped_dependent_on<int>>{kangaru::provide<int>(constructed)};
			},
		};
		
		static_assert(std::same_as<decltype(source.source), kangaru::object_source<int>>);
		static_assert(std::same_as<decltype(source.next.source), kangaru::reference_source<unmapped_dependent_on<int>>>);
		static_assert(not std::copyable<decltype(source)>);
		static_assert(not std::movable<decltype(source)>);
		static_assert(std::copyable<decltype(source.next)>);
		static_assert(std::movable<decltype(source.next)>);
		static_assert(std::assignable_from<decltype(source)&, decltype(source) const&>);
		static_assert(not kangaru::source_of<decltype(source), kangaru::object_source<int>>);
		static_assert(not kangaru::source_of<decltype(source), kangaru::reference_source<unmapped_dependent_on<int>>>);
		static_assert(has_next<decltype(source)>);
		static_assert(not has_next<decltype(source.next)>);
		static_assert(kangaru::wrapping_source<decltype(source.next)>);
		
		CHECK(kangaru::provide<int>(source) == 42);
		CHECK(kangaru::provide<unmapped_dependent_on<int>&>(source).value == 42);
	}
	
	SECTION("Construct using lambda auto") {
		auto source = kangaru::incremental_source{
			[](kangaru::source auto constructed) {
				return kangaru::object_source{42};
			},
			[](kangaru::source auto constructed) {
				return kangaru::reference_source<unmapped_dependent_on<int>>{kangaru::provide<int>(constructed)};
			},
		};
		
		// Identical to previous section
		static_assert(std::same_as<decltype(source.source), kangaru::object_source<int>>);
		static_assert(std::same_as<decltype(source.next.source), kangaru::reference_source<unmapped_dependent_on<int>>>);
		static_assert(not std::copyable<decltype(source)>);
		static_assert(not std::movable<decltype(source)>);
		static_assert(std::copyable<decltype(source.next)>);
		static_assert(std::movable<decltype(source.next)>);
		static_assert(std::assignable_from<decltype(source)&, decltype(source) const&>);
		static_assert(not kangaru::source_of<decltype(source), kangaru::object_source<int>>);
		static_assert(not kangaru::source_of<decltype(source), kangaru::reference_source<unmapped_dependent_on<int>>>);
		static_assert(has_next<decltype(source)>);
		static_assert(not has_next<decltype(source.next)>);
		static_assert(kangaru::wrapping_source<decltype(source.next)>);
		
		CHECK(kangaru::provide<int>(source) == 42);
		CHECK(kangaru::provide<unmapped_dependent_on<int>&>(source).value == 42);
	}
	
	SECTION("Construct sources from three") {
		auto source = kangaru::incremental_source{
			[](kangaru::composed_source<> constructed) {
				return kangaru::object_source{42};
			},
			[](kangaru::composed_source<kangaru::source_reference_wrapper<kangaru::object_source<int>>> constructed) {
				return kangaru::reference_source<unmapped_dependent_on<int>>{kangaru::provide<int>(constructed)};
			},
			[](kangaru::composed_source<
				kangaru::source_reference_wrapper<kangaru::object_source<int>>,
				kangaru::source_reference_wrapper<kangaru::reference_source<unmapped_dependent_on<int>>>
			> constructed) {
				return kangaru::object_source<agg_unmapped_dependent_on<unmapped_dependent_on<int>&>>{
					kangaru::provide<unmapped_dependent_on<int>&>(constructed)
				};
			},
		};
		
		static_assert(std::same_as<decltype(source.source), kangaru::object_source<int>>);
		static_assert(std::same_as<decltype(source.next.source), kangaru::reference_source<unmapped_dependent_on<int>>>);
		static_assert(std::same_as<decltype(source.next.next.source), kangaru::object_source<agg_unmapped_dependent_on<unmapped_dependent_on<int>&>>>);
		static_assert(not std::copyable<decltype(source)>);
		static_assert(not std::movable<decltype(source)>);
		static_assert(not std::copyable<decltype(source.next)>);
		static_assert(not std::movable<decltype(source.next)>);
		static_assert(has_next<decltype(source)>);
		static_assert(has_next<decltype(source.next)>);
		static_assert(not has_next<decltype(source.next.next)>);
		static_assert(kangaru::wrapping_source<decltype(source.next.next)>);
		
		// Not assignable since it contains a reference
		static_assert(not std::assignable_from<decltype(source)&, decltype(source) const&>);
		
		static_assert(kangaru::source_of<decltype(source), unmapped_dependent_on<int>&>);
		static_assert(not kangaru::source_of<decltype(source), unmapped_dependent_on<int>>);
		
		auto& second = kangaru::provide<unmapped_dependent_on<int>&>(source.next.source);
		auto third = kangaru::provide<agg_unmapped_dependent_on<unmapped_dependent_on<int>&>>(source.next.next.source);
		CHECK(std::addressof(second) == std::addressof(third.value));
	}
	
	SECTION("Duplicate are not providable") {
		struct ref_to_unmapped_dep_on_int {
			explicit ref_to_unmapped_dep_on_int(int val) : obj{val} {}
			
			constexpr auto provide() -> auto& {
				return obj;
			}
			
			unmapped_dependent_on<int> obj;
		};
		
		auto source = kangaru::incremental_source{
			[](kangaru::composed_source<> constructed) {
				return kangaru::object_source{42};
			},
			[](kangaru::composed_source<kangaru::source_reference_wrapper<kangaru::object_source<int>>> constructed) {
				return kangaru::reference_source<unmapped_dependent_on<int>>{kangaru::provide<int>(constructed)};
			},
			[](auto constructed) {
				return ref_to_unmapped_dep_on_int{kangaru::provide<int>(constructed)};
			},
		};
		
		static_assert(std::same_as<decltype(source.source), kangaru::object_source<int>>);
		static_assert(std::same_as<decltype(source.next.source), kangaru::reference_source<unmapped_dependent_on<int>>>);
		static_assert(std::same_as<decltype(source.next.next.source), ref_to_unmapped_dep_on_int>);
		static_assert(not kangaru::source_of<decltype(source), unmapped_dependent_on<int>&>);
		static_assert(kangaru::source_of<decltype(source), int>);
		static_assert(kangaru::source_of<decltype(source), int>);
		static_assert(kangaru::source_of<decltype(source.next.source), unmapped_dependent_on<int>&>);
		static_assert(kangaru::source_of<decltype(source.next.next.source), unmapped_dependent_on<int>&>);
		static_assert(has_next<decltype(source)>);
		static_assert(has_next<decltype(source.next)>);
		static_assert(not has_next<decltype(source.next.next)>);
		static_assert(kangaru::wrapping_source<decltype(source.next.next)>);
		
		auto& second = kangaru::provide<unmapped_dependent_on<int>&>(source.next.source);
		auto& third = kangaru::provide<unmapped_dependent_on<int>&>(source.next.next.source);
		CHECK(std::addressof(second) != std::addressof(third));
		CHECK(second.value == third.value);
	}
	
	SECTION("Forwards value category") {
		struct value_cat_source {
			constexpr auto provide() & {
				return 1;
			}
			
			constexpr auto provide() const& {
				return 2;
			}
			
			constexpr auto provide() && {
				return 3;
			}
			
			constexpr auto provide() const&& {
				return 4;
			}
		};
		
		auto source = kangaru::incremental_source{
			[](kangaru::source auto none) {
				return value_cat_source{};
			}
		};
		
		CHECK(kangaru::provide<int>(source) == 1);
		CHECK(kangaru::provide<int>(std::as_const(source)) == 2);
		CHECK(kangaru::provide<int>(std::move(source)) == 3);
		CHECK(kangaru::provide<int>(std::move(std::as_const(source))) == 4);
		static_assert(kangaru::wrapping_source<decltype(source)>);
		static_assert(not has_next<decltype(source)>);
	}

	SECTION("Provide different types depending on the source constructed") {
		auto source = kangaru::incremental_source{
			[](auto none) {
				return kangaru::derived_shared_pointer_source<unmapped_abstract, unmapped_concrete>{42};
			},
			[](auto constructed) {
				return kangaru::reference_source<unmapped_dependent_on<std::shared_ptr<unmapped_abstract>>>{
					kangaru::provide<std::shared_ptr<unmapped_abstract>>(constructed)
				};
			},
			[](auto constructed) {
				struct unique_ptr_source {
					decltype(constructed) source;
					
					constexpr auto provide() {
						return std::make_unique<
							unmapped_dependent_on<unmapped_dependent_on<std::shared_ptr<unmapped_abstract>>&>
						>(
							kangaru::provide<unmapped_dependent_on<std::shared_ptr<unmapped_abstract>>&>(source)
						);
					}
				};
				
				return unique_ptr_source{constructed};
			}
		};
		
		CHECK(kangaru::provide<std::shared_ptr<unmapped_abstract>>(source)->value == 42);
		CHECK(kangaru::provide<unmapped_dependent_on<std::shared_ptr<unmapped_abstract>>&>(source).value->value == 42);
		CHECK(kangaru::provide<std::unique_ptr<unmapped_dependent_on<unmapped_dependent_on<std::shared_ptr<unmapped_abstract>>&>>>(source)->value.value->value == 42);
	}
	
	SECTION("Can be empty") {
		auto source = kangaru::incremental_source{};
		static_assert(std::is_empty_v<decltype(source)>);
		static_assert(std::copyable<decltype(source)>);
		static_assert(std::assignable_from<decltype(source)&, decltype(source) const&>);
		static_assert(not kangaru::source_of<decltype(source), empty>);
		static_assert(not kangaru::source_of<decltype(source), int>);
		static_assert(not kangaru::wrapping_source<decltype(source)>);
		static_assert(not has_next<decltype(source)>);
	}
	
	static_assert(not test_valid_incremental_source(
		[]{}
	));
	
	static_assert(not test_valid_incremental_source(
		[](auto){}
	));
	
	static_assert(not test_valid_incremental_source(
		[](auto){ return kangaru::none_source{}; },
		[](){}
	));
	
	static_assert(not test_valid_incremental_source(
		[](auto){ return kangaru::none_source{}; },
		[](auto){}
	));
	
	static_assert(test_valid_incremental_source(
		[](auto){ return kangaru::none_source{}; },
		[](auto){ return kangaru::none_source{}; }
	));
	
	static_assert(not test_valid_incremental_source(
		[](auto){ return kangaru::none_source{}; },
		[](kangaru::source_of<int> auto) { return kangaru::none_source{}; }
	));
	
	static_assert(test_valid_incremental_source(
		[](auto){ return kangaru::object_source{21}; },
		[](kangaru::source_of<int> auto) { return kangaru::none_source{}; }
	));
	
	static_assert(test_valid_incremental_source(
		[](kangaru::source auto){ return kangaru::object_source{21}; },
		[](kangaru::source_of<int> auto) { return kangaru::object_source{14.f}; },
		[](kangaru::source_of_all<int, float> auto){ return kangaru::none_source{}; }
	));
}
