#include "container_test_helper.hpp"
#include <kangaru/kangaru.hpp>
#include <catch2/catch_test_macros.hpp>
#include <type_traits>

TEST_CASE("Incremental source", "[modular]") {
	SECTION("Construct sources from callbacks") {
		auto source = kangaru::incremental_source{
			[](kangaru::composed_source<> constructed) {
				return kangaru::object_source{42};
			},
		};
		
		static_assert(std::same_as<decltype(source.source), kangaru::object_source<int>>);
		static_assert(std::copyable<decltype(source)>);
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
		auto& second = kangaru::provide<unmapped_dependent_on<int>&>(source.next.source);
		auto third = kangaru::provide<agg_unmapped_dependent_on<unmapped_dependent_on<int>&>>(source.next.next.source);
		CHECK(std::addressof(second) == std::addressof(third.value));
	}
	
	SECTION("Can be empty") {
		auto source = kangaru::incremental_source{};
		static_assert(std::is_empty_v<decltype(source)>);
		static_assert(std::copyable<decltype(source)>);
	}
}
