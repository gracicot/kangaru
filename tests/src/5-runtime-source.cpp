#include "kangaru/detail/recursive_source.hpp"
#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

#include <iostream>

struct Base {
	virtual auto get() -> int {
		return 0;
	}
	virtual ~Base() = default;
};

struct Derived final : Base {
	explicit Derived(int id) noexcept : id{id} {}
	int id;
	auto get() -> int override {
		return id;
	}
	
	friend auto tag(kangaru::tag_for<Derived>) -> kangaru::overrides<Base>;
};

struct increment_source {
	int n = 0;
	
	constexpr auto provide() -> int {
		return n++;
	}
} source{};

struct needs_int_pointer {
	int* ptr;
};

TEST_CASE("Runtime source will cache sources results", "[deducer]") {
	SECTION("Will cache the result of sources") {
		auto runtime_source = kangaru::with_cache<kangaru::with_heap_storage<decltype(kangaru::ref(source))>>{kangaru::with_heap_storage{kangaru::ref(source)}};
		
		CHECK(*kangaru::provide(kangaru::provide_tag_v<int*>, runtime_source) == 0);
		CHECK(*kangaru::provide(kangaru::provide_tag_v<int*>, runtime_source) == 0);
		CHECK(kangaru::provide(kangaru::provide_tag_v<int>, source) == 1);
		CHECK(*kangaru::provide(kangaru::provide_tag_v<int*>, runtime_source) == 0);
	}
	
	SECTION("Can polymorphically store cached types") {
		auto source = kangaru::concat(
			kangaru::object_source{Derived{3}},
			kangaru::object_source{Base{}}
		);
		auto runtime_source = kangaru::make_source_with_cache(
			kangaru::make_source_with_heap_storage(source),
			kangaru::polymorphic_map<std::unordered_map<std::size_t, void*>>{}
		);
		
		CHECK(kangaru::provide(kangaru::provide_tag_v<Derived*>, runtime_source)->get() == 3);
		CHECK(kangaru::provide(kangaru::provide_tag_v<Base*>, runtime_source)->get() == 3);
	}
	
	SECTION("Already cached type has priority") {
		auto source = kangaru::concat(
			kangaru::object_source{Derived{3}},
			kangaru::object_source{Base{}}
		);
		auto runtime_source = kangaru::make_source_with_cache(
			kangaru::make_source_with_heap_storage(source),
			kangaru::polymorphic_map<std::unordered_map<std::size_t, void*>>{}
		);
		
		CHECK(kangaru::provide(kangaru::provide_tag_v<Base*>, runtime_source)->get() == 0);
		CHECK(kangaru::provide(kangaru::provide_tag_v<Derived*>, runtime_source)->get() == 3);
		CHECK(kangaru::provide(kangaru::provide_tag_v<Base*>, runtime_source)->get() == 0);
	}
	
	SECTION("Recursion with heap and cache") {
		static_assert(kangaru::stateful_rebindable_wrapping_source<kangaru::with_heap_storage<increment_source>>);
		static_assert(kangaru::stateful_rebindable_wrapping_source<kangaru::with_cache<kangaru::with_heap_storage<increment_source>>>);
		
		auto basic_recursion_test = kangaru::with_tree_recursion<kangaru::with_cache<kangaru::with_heap_storage<increment_source>>>{
			kangaru::with_cache<kangaru::with_heap_storage<increment_source>>{kangaru::with_heap_storage<increment_source>{increment_source{}}}
		};
		
		CHECK(kangaru::provide(kangaru::provide_tag_v<int*>, basic_recursion_test));
	}
	
	SECTION("Recursion with heap and cache and construction") {
		auto source = kangaru::with_tree_recursion{
			kangaru::make_source_with_cache(kangaru::make_source_with_heap_storage(kangaru::make_source_with_construction(increment_source{}, kangaru::non_empty_construction{})))
		};

		CHECK(kangaru::provide(kangaru::provide_tag_v<needs_int_pointer*>, source));
	}
}
