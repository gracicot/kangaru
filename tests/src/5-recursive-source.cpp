#include "kangaru/detail/recursive_source.hpp"
#include "kangaru/detail/source_types.hpp"
#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

#include <fmt/core.h>

struct increment_source {
	int n = 0;
	
	constexpr auto provide() -> int {
		return n++;
	}
};

struct service_a {
	explicit service_a(int a) : a{a} {}
	int a = 0;
	friend auto attribute(kangaru::allow_runtime_caching<service_a&>) -> std::true_type;
};

struct service_b {
	service_a& a;
	
	friend auto attribute(kangaru::allow_runtime_caching<service_b&>) -> std::true_type;
};

struct service_aggregate {
	service_a& sa;
	service_b& sb;
};

struct service_c {
	explicit service_c(service_aggregate services) noexcept : services{services} {}
	service_aggregate services;
	
	friend auto attribute(kangaru::allow_runtime_caching<service_c&>) -> std::true_type;
};

struct needs_int_ref {
	explicit needs_int_ref(int& ref) : ref{ref} {}
	int& ref;
};

template<typename T>
struct remove_unique_ptr {
	using type = T;
};

template<typename T>
struct remove_unique_ptr<std::unique_ptr<T>> {
	using type = T;
};

template<kangaru::source Source>
struct with_add_pointer {
	constexpr explicit with_add_pointer(Source source) noexcept : source{std::move(source)} {}
	
	Source source;
	
	template<
		typename Ptr,
		kangaru::forwarded<with_add_pointer> Self,
		typename T = typename remove_unique_ptr<Ptr>::type
	>
		requires kangaru::wrapping_source_of<Self, T>
	friend constexpr auto provide(Self&& source) -> std::unique_ptr<T> {
		return std::make_unique<T>(kangaru::provide<T>(std::forward<Self>(source).source));
	}
};

TEST_CASE("Recursive source", "[recursive]") {
	SECTION("Can call source recursively") {
		auto source = kangaru::with_recursion{
			with_add_pointer{
				increment_source{9}
			}
		};
		
		static_assert(kangaru::source_of<decltype((source)), std::unique_ptr<int>>);
		CHECK(*kangaru::provide<std::unique_ptr<int>>(source) == 9);
		CHECK(**kangaru::provide<std::unique_ptr<std::unique_ptr<int>>>(source) == 10);
		CHECK(***kangaru::provide<std::unique_ptr<std::unique_ptr<std::unique_ptr<int>>>>(source) == 11);
		CHECK(****kangaru::provide<std::unique_ptr<std::unique_ptr<std::unique_ptr<std::unique_ptr<int>>>>>(source) == 12);
		CHECK(*****kangaru::provide<std::unique_ptr<std::unique_ptr<std::unique_ptr<std::unique_ptr<std::unique_ptr<int>>>>>>(source) == 13);
	}
	
	SECTION("Recursion with heap and cache") {
		static_assert(kangaru::stateful_rebindable_wrapping_source<kangaru::with_heap_storage<increment_source>>);
		static_assert(kangaru::stateful_rebindable_wrapping_source<kangaru::with_cache<kangaru::with_heap_storage<increment_source>>>);
		
		auto basic_recursion_test = kangaru::with_recursion{
			kangaru::make_source_with_cache(kangaru::make_source_with_heap_storage(increment_source{}))
		};
		
		CHECK(kangaru::provide<int*>(basic_recursion_test));
	}
	
	SECTION("Recursion with heap and cache and construction") {
		auto source = kangaru::with_recursion{
			kangaru::make_source_with_dereference(
				kangaru::make_source_with_cache(
					kangaru::make_source_with_heap_storage(
						kangaru::make_source_with_non_empty_construction(
							increment_source{}
						)
					),
					std::unordered_map<std::size_t, void*>{}
				)
			)
		};
		
		CHECK(kangaru::provide<needs_int_ref&>(source).ref == 0);
	}
	
	SECTION("Support the service idiom and cache and construction alternative") {
		auto source = kangaru::with_recursion{
			kangaru::make_source_with_cache_using_source<kangaru::cached_reference_to_reference_source>(
				kangaru::make_source_with_dereference(
					kangaru::make_source_with_cache(
						kangaru::make_source_with_heap_storage(
							kangaru::make_source_with_exhaustive_construction(
								increment_source{.n = 3} // just a source of int
							)
						),
						std::unordered_map<std::size_t, void*>{}
					)
				)
			)
		};
		
		CHECK(kangaru::provide<service_a&>(source).a == 3);
		 
		service_a& ra = kangaru::provide<service_a&>(source);
		service_b& rb = kangaru::provide<service_b&>(source);
		CHECK(std::addressof(ra) == std::addressof(rb.a));
	}
	
	SECTION("Support the service idiom and cache and construction, with recursive construction on top") {
		auto source = kangaru::make_source_with_recursion(
			kangaru::make_source_with_passthrough(
				kangaru::make_source_with_exhaustive_construction(
					kangaru::make_source_with_recursion(
						kangaru::make_source_with_cache_using_source<kangaru::cached_reference_to_reference_source>(
							kangaru::make_source_with_dereference(
								kangaru::make_source_with_cache(
									kangaru::make_source_with_heap_storage(
										kangaru::make_source_with_exhaustive_construction(
											increment_source{.n = 3} // just a source of int
										)
									),
									std::unordered_map<std::size_t, void*>{}
								)
							)
						)
					)
				)
			)
		);
		
		CHECK(kangaru::provide<service_a&>(source).a == 3);
		
		service_a& ra = kangaru::provide<service_a&>(source);
		service_b& rb = kangaru::provide<service_b&>(source);
		CHECK(std::addressof(ra) == std::addressof(rb.a));
		
		auto c = kangaru::provide<service_aggregate>(source);
		CHECK(std::addressof(ra) == std::addressof(c.sa));
		CHECK(std::addressof(rb) == std::addressof(c.sb));
		
		service_c& rc = kangaru::provide<service_c&>(source);
		CHECK(std::addressof(ra) == std::addressof(rc.services.sa));
		CHECK(std::addressof(rb) == std::addressof(rc.services.sb));
	}
	
	SECTION("Properly forward value category of the source") {
		struct forward_sensitive_source {
			auto provide() & -> int {
				return 1;
			}
			
			auto provide() const& -> int {
				return 2;
			}
			
			auto provide() && -> int {
				return 3;
			}
			
			auto provide() const&& -> int {
				return 4;
			}
		};
		
		struct needs_int {
			int value;
		};
		
		struct needs_needs_int {
			needs_int i;
		};
		
		auto source = kangaru::with_recursion{
			kangaru::make_source_with_non_empty_construction(
				forward_sensitive_source{}
			)
		};
		
		CHECK(kangaru::provide<needs_int>(source).value == 1);
		CHECK(kangaru::provide<needs_int>(std::as_const(source)).value == 2);
		CHECK(kangaru::provide<needs_int>(std::move(source)).value == 3);
		CHECK(kangaru::provide<needs_int>(std::move(std::as_const(source))).value == 4);
		
		CHECK(kangaru::provide<needs_needs_int>(source).i.value == 1);
		CHECK(kangaru::provide<needs_needs_int>(std::as_const(source)).i.value == 2);
		CHECK(kangaru::provide<needs_needs_int>(std::move(source)).i.value == 3);
		CHECK(kangaru::provide<needs_needs_int>(std::move(std::as_const(source))).i.value == 4);
	}
	
	SECTION("Can use lambdas instead of constructors") {
		struct type1 { int id; };
		struct type2 { int id; };
		struct aggregate { type1 t1; type2& t2; };
		struct type3 { aggregate agg; };
		
		auto source = kangaru::with_recursion{
			kangaru::with_function_call{
				kangaru::compose(
					kangaru::object_source{type1{.id = 2}},
					kangaru::reference_source{type2{.id = 3}}
				),
				kangaru::overload{
					kangaru::function{
						[](type1 t1, type2& t2) {
							return aggregate{.t1 = t1, .t2 = t2};
						},
						kangaru::make_spread_injector_function{},
					},
					kangaru::function{
						[](aggregate agg) {
							return type3{.agg = agg};
						},
						kangaru::make_spread_injector_function{},
					},
				},
			},
		};
		
		auto t3 = kangaru::provide<type3>(source);
		
		CHECK(t3.agg.t1.id == 2);
		CHECK(t3.agg.t2.id == 3);
	}
	
	SECTION("Can use lambdas to return references") {
		struct type1 { int id; };
		struct type2 { int id; };
		struct aggregate { type1 t1; type2& t2; };
		struct type3 { aggregate agg; };
		
		auto obj = type2{.id = 8};
		auto source = kangaru::with_recursion{
			kangaru::with_function_call{
				kangaru::object_source{type1{.id = 2}},
				kangaru::overload{
					kangaru::function{
						[](type1 t1, type2& t2) {
							return aggregate{.t1 = t1, .t2 = t2};
						},
						kangaru::make_spread_injector_function{},
					},
					kangaru::function{
						[](aggregate agg) {
							return type3{.agg = agg};
						},
						kangaru::make_spread_injector_function{},
					},
					kangaru::function{
						[&obj]() -> type2& {
							return obj;
						},
						kangaru::make_spread_injector_function{},
					},
				},
			},
		};
		
		auto t3 = kangaru::provide<type3>(source);
		
		CHECK(t3.agg.t1.id == 2);
		CHECK(t3.agg.t2.id == 8);
	}
}
