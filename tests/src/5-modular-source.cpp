#include "container_test_helper.hpp"
#include <kangaru/kangaru.hpp>
#include <catch2/catch_test_macros.hpp>

TEST_CASE("Modular source can provide a type", "[modular]") {
	SECTION("Simple source from lambda") {
		auto source = kangaru::modular_source{
			[] {
				return kangaru::object_source{42};
			},
		};
		
		static_assert(kangaru::source_of<decltype(source), int>);
		static_assert(not kangaru::source_of<decltype(source), float>);
		static_assert(not kangaru::source_of<decltype(source), int&>);
		static_assert(not kangaru::source_of<decltype(source), int const&>);
		CHECK(kangaru::provide<int>(source) == 42);
	}
	
	SECTION("Simple source from constructor function") {
		auto source = kangaru::modular_source{
			kangaru::constructor_function<kangaru::object_source<int>>{},
		};
		
		static_assert(kangaru::source_of<decltype(source), int>);
		static_assert(not kangaru::source_of<decltype(source), float>);
		static_assert(not kangaru::source_of<decltype(source), float>);
		static_assert(not kangaru::source_of<decltype(source), int&>);
		static_assert(not kangaru::source_of<decltype(source), int const&>);
		CHECK(kangaru::provide<int>(source) == 0);
	}
	
	SECTION("With simple dependency chain") {
		auto source = kangaru::modular_source{
			[] {
				return kangaru::object_source{42};
			},
			kangaru::constructor_function<kangaru::object_source<agg_unmapped_dependent_on<int>>>{},
		};
		
		static_assert(kangaru::source_of<decltype(source), int>);
		static_assert(kangaru::source_of<decltype(source), agg_unmapped_dependent_on<int>>);
		// modular source does not provide with construction like it does in the callbacks
		// That part is handeled by the modular container
		static_assert(not kangaru::source_of<decltype(source), agg_unmapped_dependent_on<agg_unmapped_dependent_on<int>>>);
		CHECK(kangaru::provide<agg_unmapped_dependent_on<int>>(source).value == 42);
	}
	
	SECTION("With dependency chain constructed") {
		auto source = kangaru::modular_source{
			[] {
				return kangaru::object_source{42};
			},
			[](agg_unmapped_dependent_on<int> dep) {
				return kangaru::object_source<agg_unmapped_dependent_on<agg_unmapped_dependent_on<int>>>{dep};
			}
		};
		
		static_assert(kangaru::source_of<decltype(source), int>);
		static_assert(kangaru::source_of<decltype(source), agg_unmapped_dependent_on<agg_unmapped_dependent_on<int>>>);
		CHECK(kangaru::provide<agg_unmapped_dependent_on<agg_unmapped_dependent_on<int>>>(source).value.value == 42);
	}
	
	SECTION("With dependency chain constructed empty injectable") {
		auto source = kangaru::modular_source{
			[] {
				return kangaru::object_source{42};
			},
			[](int dep, empty_injectable) {
				return kangaru::object_source<agg_unmapped_dependent_on<int>>{dep};
			}
		};
		
		static_assert(kangaru::source_of<decltype(source), int>);
		static_assert(kangaru::source_of<decltype(source), agg_unmapped_dependent_on<int>>);
		CHECK(kangaru::provide<agg_unmapped_dependent_on<int>>(source).value == 42);
	}
	
	SECTION("With reference dependency") {
		auto source = kangaru::modular_source{
			[] {
				return kangaru::reference_source{42};
			},
			[](int& dep) {
				return kangaru::object_source<agg_unmapped_dependent_on<int>>{dep};
			}
		};
		
		static_assert(kangaru::source_of<decltype(source), int&>);
		static_assert(kangaru::source_of<decltype(source), agg_unmapped_dependent_on<int>>);
		static_assert(not kangaru::source_of<decltype(source), int>);
		CHECK(kangaru::provide<agg_unmapped_dependent_on<int>>(source).value == 42);
	}
	
	SECTION("With shared pointer polymorphic dependency") {
		auto source = kangaru::modular_source{
			[] {
				return kangaru::derived_reference_source<unmapped_abstract, unmapped_concrete>{42};
			},
			[](unmapped_abstract& dep) {
				return kangaru::object_source<agg_unmapped_dependent_on<int>>{dep.value};
			}
		};
		
		static_assert(kangaru::source_of<decltype(source), unmapped_abstract&>);
		static_assert(not kangaru::source_of<decltype(source), unmapped_concrete&>);
		static_assert(kangaru::source_of<decltype(source), agg_unmapped_dependent_on<int>>);
		static_assert(not kangaru::source_of<decltype(source), int>);
		CHECK(kangaru::provide<agg_unmapped_dependent_on<int>>(source).value == 42);
	}
	
	SECTION("With shared pointer dependency") {
		auto source = kangaru::modular_source{
			[] {
				return kangaru::shared_pointer_source{42};
			},
			[](std::shared_ptr<int> dep) {
				return kangaru::object_source<agg_unmapped_dependent_on<std::shared_ptr<int>>>{dep};
			}
		};
		
		static_assert(kangaru::source_of<decltype(source), std::shared_ptr<int>>);
		static_assert(kangaru::source_of<decltype(source), agg_unmapped_dependent_on<std::shared_ptr<int>>>);
		static_assert(not kangaru::source_of<decltype(source), int>);
		CHECK(*kangaru::provide<agg_unmapped_dependent_on<std::shared_ptr<int>>>(source).value == 42);
	}
	
	SECTION("With shared pointer polymorphic dependency") {
		auto source = kangaru::modular_source{
			[] {
				return kangaru::derived_shared_pointer_source<unmapped_abstract, unmapped_concrete>{42};
			},
			[](std::shared_ptr<unmapped_abstract> dep) {
				return kangaru::object_source<agg_unmapped_dependent_on<int>>{dep->value};
			}
		};
		
		static_assert(kangaru::source_of<decltype(source), std::shared_ptr<unmapped_abstract>>);
		static_assert(not kangaru::source_of<decltype(source), std::shared_ptr<unmapped_concrete>>);
		static_assert(kangaru::source_of<decltype(source), agg_unmapped_dependent_on<int>>);
		static_assert(not kangaru::source_of<decltype(source), int>);
		CHECK(kangaru::provide<agg_unmapped_dependent_on<int>>(source).value == 42);
	}
	
	SECTION("With base source") {
		auto source = kangaru::modular_source{
			kangaru::object_source{42},
			kangaru::constructor_function<kangaru::object_source<unmapped_dependent_on<int>>>{}
		};
		
		static_assert(not kangaru::source_of<decltype(source), int>);
		static_assert(kangaru::source_of<decltype(source), unmapped_dependent_on<int>>);
		CHECK(kangaru::provide<unmapped_dependent_on<int>>(source).value == 42);
	}
	
	SECTION("With base source and construction") {
		auto source = kangaru::modular_source{
			kangaru::exhaustive_strict_construction{},
			kangaru::reference_source{42},
			kangaru::constructor_function<kangaru::object_source<unmapped_dependent_on<unmapped_dependent_on<int&>>>>{}
		};
		
		static_assert(not kangaru::source_of<decltype(source), int>);
		static_assert(kangaru::source_of<decltype(source), unmapped_dependent_on<unmapped_dependent_on<int&>>>);
		CHECK(kangaru::provide<unmapped_dependent_on<unmapped_dependent_on<int&>>>(source).value.value == 42);
	}
	
	SECTION("Using factory function empty") {
		auto source = kangaru::make_modular_source();
		static_assert(std::same_as<kangaru::modular_source<>, decltype(source)>);
	}
	
	SECTION("Using factory function source only") {
		auto source = kangaru::make_modular_source(kangaru::object_source{42});
		static_assert(
			std::same_as<
				kangaru::modular_source<kangaru::exhaustive_construction, kangaru::object_source<int>>,
				decltype(source)
			>
		);
		static_assert(not kangaru::source_of<decltype(source), int>);
	}
	
	SECTION("Using factory function lambdas") {
		auto source = kangaru::make_modular_source(
			[] {
				return kangaru::object_source{42};
			},
			kangaru::constructor_function<kangaru::object_source<agg_unmapped_dependent_on<int>>>{}
		);
		
		static_assert(kangaru::source_of<decltype(source), int>);
		static_assert(kangaru::source_of<decltype(source), agg_unmapped_dependent_on<int>>);
		CHECK(kangaru::provide<agg_unmapped_dependent_on<int>>(source).value == 42);
	}
	
	SECTION("Using factory source and lambdas") {
		auto source = kangaru::make_modular_source(
			kangaru::object_source{42},
			kangaru::constructor_function<kangaru::object_source<unmapped_dependent_on<int>>>{}
		);
		
		static_assert(not kangaru::source_of<decltype(source), int>);
		static_assert(kangaru::source_of<decltype(source), unmapped_dependent_on<int>>);
		CHECK(kangaru::provide<unmapped_dependent_on<int>>(source).value == 42);
	}
	
	SECTION("Using factory construction, source and lambdas") {
		auto source = kangaru::make_modular_source(
			kangaru::exhaustive_strict_construction{},
			kangaru::reference_source{42},
			kangaru::constructor_function<kangaru::object_source<unmapped_dependent_on<unmapped_dependent_on<int&>>>>{}
		);
		
		static_assert(not kangaru::source_of<decltype(source), int>);
		static_assert(kangaru::source_of<decltype(source), unmapped_dependent_on<unmapped_dependent_on<int&>>>);
		CHECK(kangaru::provide<unmapped_dependent_on<unmapped_dependent_on<int&>>>(source).value.value == 42);
	}
	
	SECTION("Using factory list of types") {
		struct initialized_int_source {
			constexpr auto provide() {
				return 42;
			}
		};
		
		auto source = kangaru::make_modular_source<
			initialized_int_source,
			kangaru::object_source<unmapped_dependent_on<int>>
		>();
		
		static_assert(kangaru::source_of<decltype(source), int>);
		static_assert(kangaru::source_of<decltype(source), unmapped_dependent_on<int>>);
		CHECK(kangaru::provide<unmapped_dependent_on<int>>(source).value == 42);
	}
	
	SECTION("Using factory list of types and a source") {
		auto source = kangaru::make_modular_source<
			kangaru::object_source<unmapped_dependent_on<int>>,
			kangaru::reference_source<unmapped_dependent_on<unmapped_dependent_on<int>>>
		>(kangaru::object_source{42});
		
		static_assert(not kangaru::source_of<decltype(source), int>);
		static_assert(kangaru::source_of<decltype(source), unmapped_dependent_on<int>>);
		static_assert(kangaru::source_of<decltype(source), unmapped_dependent_on<unmapped_dependent_on<int>>&>);
		CHECK(kangaru::provide<unmapped_dependent_on<unmapped_dependent_on<int>>&>(source).value.value == 42);
	}
	
	SECTION("Using factory list of types a construction and a source") {
		auto source = kangaru::make_modular_source<
			kangaru::reference_source<unmapped_dependent_on<int>>,
			kangaru::object_source<unmapped_dependent_on<unmapped_dependent_on<int>&>>
		>(kangaru::exhaustive_strict_construction{}, kangaru::object_source{42});
		
		static_assert(not kangaru::source_of<decltype(source), int>);
		static_assert(kangaru::source_of<decltype(source), unmapped_dependent_on<int>&>);
		static_assert(kangaru::source_of<decltype(source), unmapped_dependent_on<unmapped_dependent_on<int>&>>);
		CHECK(kangaru::provide<unmapped_dependent_on<unmapped_dependent_on<int>&>>(source).value.value == 42);
	}
	
	SECTION("Using in-place factory function empty") {
		auto in_place_source = kangaru::make_modular_source_in_place();
		auto source = kangaru::in_place_construct_result_t<decltype(in_place_source)>{std::move(in_place_source)};
		static_assert(std::same_as<kangaru::modular_source<>, decltype(source)>);
	}
	
	SECTION("Using in-place factory function source only") {
		auto in_place_source = kangaru::make_modular_source_in_place(kangaru::object_source{42});
		auto source = kangaru::in_place_construct_result_t<decltype(in_place_source)>{std::move(in_place_source)};
		static_assert(
			std::same_as<
				kangaru::modular_source<kangaru::exhaustive_construction, kangaru::object_source<int>>,
				decltype(source)
			>
		);
		static_assert(not kangaru::source_of<decltype(source), int>);
	}
	
	SECTION("Using in-place factory function lambdas") {
		auto in_place_source = kangaru::make_modular_source_in_place(
			[] {
				return kangaru::object_source{42};
			},
			kangaru::constructor_function<kangaru::object_source<agg_unmapped_dependent_on<int>>>{}
		);
		auto source = kangaru::in_place_construct_result_t<decltype(in_place_source)>{std::move(in_place_source)};
		
		static_assert(kangaru::source_of<decltype(source), int>);
		static_assert(kangaru::source_of<decltype(source), agg_unmapped_dependent_on<int>>);
		CHECK(kangaru::provide<agg_unmapped_dependent_on<int>>(source).value == 42);
	}
	
	SECTION("Using in-place factory source and lambdas") {
		auto in_place_source = kangaru::make_modular_source_in_place(
			kangaru::object_source{42},
			kangaru::constructor_function<kangaru::object_source<unmapped_dependent_on<int>>>{}
		);
		auto source = kangaru::in_place_construct_result_t<decltype(in_place_source)>{std::move(in_place_source)};
		
		static_assert(not kangaru::source_of<decltype(source), int>);
		static_assert(kangaru::source_of<decltype(source), unmapped_dependent_on<int>>);
		CHECK(kangaru::provide<unmapped_dependent_on<int>>(source).value == 42);
	}
	
	SECTION("Using in-place factory construction, source and lambdas") {
		auto in_place_source = kangaru::make_modular_source_in_place(
			kangaru::exhaustive_strict_construction{},
			kangaru::reference_source{42},
			kangaru::constructor_function<kangaru::object_source<unmapped_dependent_on<unmapped_dependent_on<int&>>>>{}
		);
		auto source = kangaru::in_place_construct_result_t<decltype(in_place_source)>{std::move(in_place_source)};
		
		static_assert(not kangaru::source_of<decltype(source), int>);
		static_assert(kangaru::source_of<decltype(source), unmapped_dependent_on<unmapped_dependent_on<int&>>>);
		CHECK(kangaru::provide<unmapped_dependent_on<unmapped_dependent_on<int&>>>(source).value.value == 42);
	}
	
	SECTION("Using in-place factory list of types") {
		struct initialized_int_source {
			constexpr auto provide() {
				return 42;
			}
		};
		
		auto in_place_source = kangaru::make_modular_source_in_place<
			initialized_int_source,
			kangaru::object_source<unmapped_dependent_on<int>>
		>();
		auto source = kangaru::in_place_construct_result_t<decltype(in_place_source)>{std::move(in_place_source)};
		
		static_assert(kangaru::source_of<decltype(source), int>);
		static_assert(kangaru::source_of<decltype(source), unmapped_dependent_on<int>>);
		CHECK(kangaru::provide<unmapped_dependent_on<int>>(source).value == 42);
	}
	
	SECTION("Using in-place factory list of types and a source") {
		auto in_place_source = kangaru::make_modular_source_in_place<
			kangaru::object_source<unmapped_dependent_on<int>>,
			kangaru::reference_source<unmapped_dependent_on<unmapped_dependent_on<int>>>
		>(kangaru::object_source{42});
		auto source = kangaru::in_place_construct_result_t<decltype(in_place_source)>{std::move(in_place_source)};
		
		static_assert(not kangaru::source_of<decltype(source), int>);
		static_assert(kangaru::source_of<decltype(source), unmapped_dependent_on<int>>);
		static_assert(kangaru::source_of<decltype(source), unmapped_dependent_on<unmapped_dependent_on<int>>&>);
		CHECK(kangaru::provide<unmapped_dependent_on<unmapped_dependent_on<int>>&>(source).value.value == 42);
	}
	
	SECTION("Using in-place factory list of types a construction and a source") {
		auto in_place_source = kangaru::make_modular_source_in_place<
			kangaru::reference_source<unmapped_dependent_on<int>>,
			kangaru::object_source<unmapped_dependent_on<unmapped_dependent_on<int>&>>
		>(kangaru::exhaustive_strict_construction{}, kangaru::object_source{42});
		auto source = kangaru::in_place_construct_result_t<decltype(in_place_source)>{std::move(in_place_source)};
		
		static_assert(not kangaru::source_of<decltype(source), int>);
		static_assert(kangaru::source_of<decltype(source), unmapped_dependent_on<int>&>);
		static_assert(kangaru::source_of<decltype(source), unmapped_dependent_on<unmapped_dependent_on<int>&>>);
		CHECK(kangaru::provide<unmapped_dependent_on<unmapped_dependent_on<int>&>>(source).value.value == 42);
	}
}

