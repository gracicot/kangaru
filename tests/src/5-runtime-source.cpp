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

TEST_CASE("Runtime source will cache sources results", "[deducer]") {
	auto source = kangaru::function_source{[n = 0]() mutable { return n++; }};
	
	SECTION("Will cache the result of sources") {
		auto runtime_source = kangaru::runtime_source{kangaru::ref(source)};
		
		CHECK(kangaru::provide(kangaru::provide_tag_v<int&>, runtime_source) == 0);
		CHECK(kangaru::provide(kangaru::provide_tag_v<int&>, runtime_source) == 0);
		CHECK(kangaru::provide(kangaru::provide_tag_v<int>, source) == 1);
		CHECK(kangaru::provide(kangaru::provide_tag_v<int&>, runtime_source) == 0);
	}
	
	SECTION("Can polymorphically store cached types") {
		auto source = kangaru::concat(
			kangaru::object_source{Derived{3}},
			kangaru::object_source{Base{}}
		);
		auto runtime_source = kangaru::runtime_source<decltype(source), kangaru::polymorphic_map<std::unordered_map<std::size_t, void*>>>{source};
		
		CHECK(kangaru::provide(kangaru::provide_tag_v<Derived&>, runtime_source).get() == 3);
		CHECK(kangaru::provide(kangaru::provide_tag_v<Base&>, runtime_source).get() == 3);
	}
	
	SECTION("Already cached type has priority") {
		auto source = kangaru::concat(
			kangaru::object_source{Derived{3}},
			kangaru::object_source{Base{}}
		);
		auto runtime_source = kangaru::runtime_source<decltype(source), kangaru::polymorphic_map<std::unordered_map<std::size_t, void*>>>{source};
		
		CHECK(kangaru::provide(kangaru::provide_tag_v<Base&>, runtime_source).get() == 0);
		CHECK(kangaru::provide(kangaru::provide_tag_v<Derived&>, runtime_source).get() == 3);
		CHECK(kangaru::provide(kangaru::provide_tag_v<Base&>, runtime_source).get() == 0);
	}
}
