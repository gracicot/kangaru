#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

#include <fmt/core.h>

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
};

TEST_CASE("Runtime source will cache sources results", "[deducer]") {
	auto source = increment_source{};
	SECTION("Will cache the result of sources") {
		auto runtime_source = kangaru::with_cache<kangaru::with_heap_storage<decltype(kangaru::ref(source))>>{kangaru::with_heap_storage{kangaru::ref(source)}};
		
		CHECK(*kangaru::provide<int*>(runtime_source) == 0);
		CHECK(*kangaru::provide<int*>(runtime_source) == 0);
		CHECK(kangaru::provide<int>(source) == 1);
		CHECK(*kangaru::provide<int*>(runtime_source) == 0);
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
		
		CHECK(kangaru::provide<Derived*>(runtime_source)->get() == 3);
		CHECK(kangaru::provide<Base*>(runtime_source)->get() == 3);
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
		
		CHECK(kangaru::provide<Base*>(runtime_source)->get() == 0);
		CHECK(kangaru::provide<Derived*>(runtime_source)->get() == 3);
		CHECK(kangaru::provide<Base*>(runtime_source)->get() == 0);
	}
}
