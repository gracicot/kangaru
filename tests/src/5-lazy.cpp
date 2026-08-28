#include <kangaru/kangaru.hpp>

#include <catch2/catch_test_macros.hpp>

struct noisy {
	int value;
};

struct noisy_source {
	constexpr auto provide() {
		provided = true;
		return noisy{42};
	}
	
	bool provided = false;
};

TEST_CASE("Lazy will lazily initialize the result of the source", "[lazy]") {
	auto source = noisy_source{};
	auto lazy = kangaru::make_lazy<noisy>(kangaru::ref(source));
	
	REQUIRE(not source.provided);
	
	SECTION("Star operator") {
		CHECK((*lazy).value == 42);
	}
	
	SECTION("Arrow operator") {
		CHECK(lazy->value == 42);
	}
	
	SECTION("Object or") {
		CHECK(lazy.object_or(noisy{2}).value == 2);
		(void) *lazy;
		CHECK(lazy.object_or(noisy{2}).value == 42);
	}
	
	REQUIRE(source.provided);
}
