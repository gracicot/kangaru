#include "container_test_helper.hpp"
#include <kangaru/kangaru.hpp>
#include <catch2/catch_test_macros.hpp>
#include <type_traits>

TEST_CASE("Modular container", "[modular]") {
	SECTION("Create modular container from modular sources") {
		auto module0 = [] {
			return kangaru::modular_source{
				[] {
					return kangaru::object_source{42};
				},
			};
		};
		
		auto container = kangaru::modular_container{
			module0,
			[](kangaru::module_dependencies<decltype(module0)> dep) {
				return kangaru::modular_source{
					dep,
					[](int value) {
						return kangaru::object_source{unmapped_dependent_on<int>{value}};
					},
				};
			}
		};
		
		CHECK(kangaru::provide<unmapped_dependent_on<int>>(container).value == 42);
		static_assert(not kangaru::source_of<decltype(container), decltype(module0)>);
		static_assert(not kangaru::source_of<decltype(container), decltype(module0())>);
		static_assert(not kangaru::source_of<decltype(container), kangaru::module_dependencies<decltype(module0)>>);
	}
}
