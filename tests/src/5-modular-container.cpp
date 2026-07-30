#include "container_test_helper.hpp"
#include <kangaru/kangaru.hpp>
#include <catch2/catch_test_macros.hpp>

template<typename... F>
concept is_valid_container = requires{
	typename kangaru::modular_container<kangaru::exhaustive_construction, F...>;
};

template<typename... F>
constexpr auto test_valid_container(F const&...) {
	return is_valid_container<F...>;
}

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
						return kangaru::object_source{unmapped_dependent_on<int>{value + 8}};
					},
				};
			}
		};
		
		static_assert(kangaru::source_of<decltype(container), unmapped_dependent_on<int>>);
		static_assert(kangaru::source_of<decltype(container), int>);
		static_assert(not kangaru::source_of<decltype(container), decltype(module0)>);
		static_assert(not kangaru::source_of<decltype(container), decltype(module0())>);
		static_assert(not kangaru::source_of<decltype(container), kangaru::module_dependencies<decltype(module0)>>);
		
		CHECK(kangaru::provide<int>(container) == 42);
		CHECK(kangaru::provide<unmapped_dependent_on<int>>(container).value == 50);
	}
	
	SECTION("constexpr container") {
		auto module0 = []{ return kangaru::object_source{42}; };
		constexpr auto container = kangaru::modular_container{
			kangaru::exhaustive_construction{},
			module0
		};
		
		static_assert(kangaru::source_of<decltype(container), unmapped_dependent_on<int>>);
		static_assert(kangaru::source_of<decltype(container), int>);
		static_assert(kangaru::provide<int>(container) == 42);
		static_assert(kangaru::provide<unmapped_dependent_on<int>>(container).value == 42);
	}
	
	SECTION("Contructed from container") {
		SECTION("Matching loosely") {
			auto module0 = [] {
				return kangaru::modular_source{
					[] {
						return kangaru::object_source{42};
					},
					[] {
						return kangaru::reference_source{52};
					},
				};
			};
			
			auto container = kangaru::modular_container{
				kangaru::exhaustive_construction{},
				module0,
			};
			
			static_assert(kangaru::source_of<decltype(container), unmapped_dependent_on<int>>);
			static_assert(kangaru::source_of<decltype(container), int>);
			static_assert(kangaru::source_of<decltype(container), int&>);
			static_assert(not kangaru::source_of<decltype(container), int const&>);
			static_assert(kangaru::source_of<decltype(container), unmapped_dependent_on<int const&>>);
			
			CHECK(kangaru::provide<int>(container) == 42);
			CHECK(kangaru::provide<int&>(container) == 52);
			CHECK(kangaru::provide<unmapped_dependent_on<int>>(container).value == 42);
			CHECK(kangaru::provide<unmapped_dependent_on<int&>>(container).value == 52);
			CHECK(kangaru::provide<unmapped_dependent_on<int const&>>(container).value == 52);
		}
		
		SECTION("Matching strictly") {
			auto module0 = [] {
				return kangaru::modular_source{
					[] {
						return kangaru::object_source{42};
					},
					[] {
						return kangaru::reference_source{52};
					},
				};
			};
			
			auto container = kangaru::modular_container{
				kangaru::exhaustive_strict_construction{},
				module0,
			};
			
			static_assert(kangaru::source_of<decltype(container), int>);
			static_assert(kangaru::source_of<decltype(container), int&>);
			static_assert(not kangaru::source_of<decltype(container), unmapped_dependent_on<int const&>>);
			
			CHECK(kangaru::provide<int>(container) == 42);
			CHECK(kangaru::provide<int&>(container) == 52);
			CHECK(kangaru::provide<unmapped_dependent_on<int>>(container).value == 42);
			CHECK(kangaru::provide<unmapped_dependent_on<int&>>(container).value == 52);
		}
		
		SECTION("No construction") {
			auto module0 = [] {
				return kangaru::modular_source{
					[] {
						return kangaru::object_source{42};
					},
					[] {
						return kangaru::reference_source{52};
					},
				};
			};
			
			auto container = kangaru::modular_container{
				kangaru::no_construction{},
				module0,
			};
			
			static_assert(not kangaru::source_of<decltype(container), unmapped_dependent_on<int>>);
			static_assert(not kangaru::source_of<decltype(container), unmapped_dependent_on<int&>>);
			static_assert(not kangaru::source_of<decltype(container), unmapped_dependent_on<int const&>>);
			
			CHECK(kangaru::provide<int>(container) == 42);
			CHECK(kangaru::provide<int&>(container) == 52);
		}
	}
	
	SECTION("None source") {
		auto module0 = [] {
			return kangaru::none_source{};
		};

		auto container = kangaru::modular_container{
			kangaru::exhaustive_construction{},
			module0,
		};
		
		static_assert(not kangaru::source_of<decltype(container), int>);
		static_assert(not kangaru::source_of<decltype(container), kangaru::none_source>);
		
		static_assert(test_valid_container(
			module0,
			[](kangaru::module_dependencies<decltype(module0)>) {
				return kangaru::none_source{};
			}
		));
	}
	
	SECTION("Module dependencies") {
		auto module0 = kangaru::object_source{12};
		
		auto source = kangaru::with_recursion{kangaru::make_source_with_exhaustive_construction(kangaru::object_source{kangaru::ref(module0)})};
		auto injector = kangaru::make_simple_injector(source);
		
		auto const result = injector([](kangaru::module_dependencies<auto(*)() -> decltype(module0)> s) {
			return kangaru::provide<int>(s);
		});
		
		CHECK(result == 12);
	}
	
	// Modules must spell all their dependencies manually using kangaru::module_dependencies
	static_assert(not test_valid_container(
		[] {
			return kangaru::object_source{42};
		},
		[](kangaru::source_of<int> auto all_of_them) {
			return kangaru::none_source{};
		}
	));

	SECTION("CTAD") {
		auto module0 = [] { return kangaru::object_source{42}; };
		auto container1 = kangaru::modular_container{module0};
		auto container2 = kangaru::modular_container{kangaru::exhaustive_construction{}, module0};
		auto container3 = kangaru::modular_container{kangaru::no_construction{}, module0};
		
		static_assert(std::same_as<kangaru::modular_container<kangaru::exhaustive_construction, decltype(module0)>, decltype(container1)>);
		static_assert(std::same_as<kangaru::modular_container<kangaru::exhaustive_construction, decltype(module0)>, decltype(container2)>);
		static_assert(std::same_as<kangaru::modular_container<kangaru::no_construction, decltype(module0)>, decltype(container3)>);
	}
	
	SECTION("Reference stability") {
		auto module0 = [] { return kangaru::object_source{42.f}; };
		auto module1 = [](kangaru::module_dependencies<decltype(module0)> dep) {
			return kangaru::make_modular_source<
				kangaru::derived_reference_source<unmapped_abstract, unmapped_concrete>,
				kangaru::object_source<unmapped_dependent_on<unmapped_abstract&>>
			>(dep);
		};
		auto module2 = [](kangaru::module_dependencies<decltype(module0), decltype(module1)> dep) {
			return kangaru::modular_source{
				dep,
				[](unmapped_abstract& abstract, float) {
					return kangaru::object_source{agg_unmapped_dependent_on<unmapped_abstract&>{abstract}};
				}
			};
		};
		
		auto container = kangaru::modular_container{
			module0, module1, module2
		};
		
		auto from_module1 = kangaru::provide<unmapped_dependent_on<unmapped_abstract&>>(container);
		auto from_module2 = kangaru::provide<agg_unmapped_dependent_on<unmapped_abstract&>>(container);
		
		CHECK(std::addressof(from_module1.value) == std::addressof(from_module2.value));
	}
}
