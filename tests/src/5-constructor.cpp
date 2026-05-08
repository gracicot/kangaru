#include <kangaru/kangaru.hpp>
#include <catch2/catch_test_macros.hpp>

struct agg {
	int value;
};

struct needs_int {
	explicit constexpr needs_int(int value) : value{value} {}
	explicit constexpr needs_int(int value, needs_int&&) : value{value} {}
	int value;
};

struct int_or_swallow {
	explicit constexpr int_or_swallow(int value) : value{value} {}
	explicit constexpr int_or_swallow(auto) { value = 3; }
	
	int value;
};

struct strict_a {
	explicit constexpr strict_a(int value) : value{value} {}
	
	int value;
};

struct converts_to_a {
	constexpr operator int_or_swallow() {
		return int_or_swallow{2};
	}
	
	constexpr operator agg() {
		return agg{2};
	}
	
	constexpr operator int() {
		return 4;
	}
};

struct explicitly_converts_to_a {
	explicit constexpr operator int_or_swallow() {
		return int_or_swallow{1};
	}
	
	explicit constexpr operator strict_a() {
		return strict_a{1};
	}
	
	constexpr operator agg() {
		return agg{2};
	}
};

struct with_args {
	explicit with_args() = default;
	constexpr with_args(int, float, double) {}
	constexpr with_args(short, with_args, double) {}
	constexpr with_args(short, float, double, double) {}
};

template<typename T>
struct source_of_t {
	auto provide() const -> T;
};

template<typename T>
using deducer_of = kangaru::basic_deducer<source_of_t<T>&>;

TEST_CASE("constructor", "[constructor]") {
	auto s1 = kangaru::reference_source{1};
	auto s2 = kangaru::reference_source{needs_int{1}};
	static_assert(not kangaru::callable<kangaru::constructor_function<needs_int>, kangaru::basic_deducer<decltype(s2)&>>);
	
	SECTION("Conversion constructor") {
		static_assert(kangaru::constructor_callable<int_or_swallow, converts_to_a>);
		static_assert(kangaru::constructor_callable<int_or_swallow, explicitly_converts_to_a>);
		
		// The convertion operator does not take priority
		CHECK(kangaru::constructor<int_or_swallow>(converts_to_a{}).value == 3);
		CHECK(kangaru::constructor<int_or_swallow>(explicitly_converts_to_a{}).value == 3);
		
		// Conversion operator agg called
		CHECK(kangaru::constructor<agg>(converts_to_a{}).value == 2);
	}
	
	SECTION("With deducer as first parameter") {
		auto n1 = kangaru::constructor<needs_int>(kangaru::basic_deducer<decltype(s1)&>{s1});
		REQUIRE(n1.value == 1);
		
		static_assert(kangaru::constructor_callable<with_args, deducer_of<int>, float, double>);
		static_assert(not kangaru::constructor_callable<with_args, deducer_of<int>, with_args, double>);
		static_assert(kangaru::constructor_callable<with_args, deducer_of<short>, with_args, double>);
		static_assert(kangaru::constructor_callable<with_args, deducer_of<short&>, with_args, double>);
		static_assert(kangaru::constructor_callable<with_args, deducer_of<short&&>, with_args, double>);
		static_assert(kangaru::constructor_callable<with_args, deducer_of<short const&>, with_args, double>);
		static_assert(kangaru::constructor_callable<with_args, deducer_of<short const&&>, with_args, double>);
		static_assert(kangaru::constructor_callable<with_args, deducer_of<short>, float, double, double>);
	}
	
	SECTION("With value as first parameter") {
		auto n1 = kangaru::constructor<needs_int>(2, kangaru::basic_deducer<decltype(s2)&>{s2});
		REQUIRE(n1.value == 2);
		auto n2 = kangaru::constructor<needs_int>(4);
		REQUIRE(n2.value == 4);
		
		static_assert(kangaru::constructor_callable<with_args, int, float, double>);
		static_assert(kangaru::constructor_callable<with_args, int, with_args, double>);
		static_assert(kangaru::constructor_callable<with_args, short, with_args, double>);
		static_assert(kangaru::constructor_callable<with_args, short, float, double, double>);
		
		static_assert(kangaru::constructor_callable<with_args, int, deducer_of<float>, double>);
		static_assert(not kangaru::constructor_callable<with_args, int, deducer_of<double>, double>);
		static_assert(kangaru::constructor_callable<with_args, int, deducer_of<with_args>, double>);
		static_assert(kangaru::constructor_callable<with_args, int, deducer_of<with_args&>, double>);
		static_assert(kangaru::constructor_callable<with_args, int, deducer_of<with_args&&>, double>);
		static_assert(kangaru::constructor_callable<with_args, int, deducer_of<with_args const&>, double>);
		static_assert(kangaru::constructor_callable<with_args, int, deducer_of<with_args const&&>, double>);
		static_assert(kangaru::constructor_callable<with_args, short, with_args, double>);
		static_assert(kangaru::constructor_callable<with_args, short, float, double, double>);
	}
	
	SECTION("Default constructor") {
		static_assert(kangaru::constructor_callable<agg>);
		static_assert(kangaru::constructor_callable<with_args>);
		static_assert(kangaru::constructor_callable<int>);
		static_assert(not kangaru::constructor_callable<strict_a>);
		REQUIRE(kangaru::constructor<int>() == 0);
		REQUIRE(kangaru::constructor<agg>().value == 0);
	}
}


TEST_CASE("non_default_constructor", "[constructor]") {
	SECTION("With deducer as first parameter") {
		auto s1 = kangaru::reference_source{1};
		auto n1 = kangaru::non_default_constructor<needs_int>(kangaru::basic_deducer<decltype(s1)&>{s1});
		REQUIRE(n1.value == 1);
		static_assert(kangaru::non_default_constructor_callable<with_args, deducer_of<int>, float, double>);
	}
	
	SECTION("With value as first parameter") {
		auto n1 = kangaru::non_default_constructor<needs_int>(3);
		REQUIRE(n1.value == 3);
		static_assert(kangaru::non_default_constructor_callable<with_args, int, float, double>);
	}
	
	SECTION("Default constructor") {
		static_assert(not kangaru::non_default_constructor_callable<agg>);
		static_assert(not kangaru::non_default_constructor_callable<with_args>);
		static_assert(not kangaru::non_default_constructor_callable<int>);
		static_assert(not kangaru::non_default_constructor_callable<strict_a>);
	}
}

struct deducing_from_in_place {
	template<kangaru::callable F> requires(kangaru::not_self<std::invoke_result_t<F>, deducing_from_in_place>)
	deducing_from_in_place(kangaru::in_place_construct<F>) {}
};

struct deducing_from_in_place_deleted {
	template<kangaru::callable F>
	deducing_from_in_place_deleted(kangaru::in_place_construct<F>) = delete;
};

template<typename T>
struct simple {
	explicit simple(T value) : value{value} {}
	T value;
};

TEST_CASE("In place construction", "[constructor]") {
	static_assert(kangaru::in_place_constructible<deducing_from_in_place>);
	static_assert(not kangaru::in_place_constructible<deducing_from_in_place_deleted>);
	static_assert(kangaru::in_place_constructible<int_or_swallow>);
	static_assert(kangaru::in_place_constructible<needs_int>);
	
	SECTION("CTAD constructor") {
		auto s = simple<int>(kangaru::construct_in_place<simple>(1));
		REQUIRE(s.value == 1);
	}
	
	SECTION("Type constructor") {
		auto s = simple<int>(kangaru::construct_in_place<simple<int>>(2));
		REQUIRE(s.value == 2);
	}
}

