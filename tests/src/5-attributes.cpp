#include <kangaru/kangaru.hpp>

#include <catch2/catch_test_macros.hpp>

struct default_value {};

// Allow runtime caching
static_assert(kangaru::allow_runtime_caching_v<default_value> == false);

struct attrf_allow_runtime_caching {
	friend auto attribute(kangaru::allow_runtime_caching<attrf_allow_runtime_caching>)
		-> std::true_type;
};

struct attrs_allow_runtime_caching {};

template<>
struct kangaru::allow_runtime_caching<attrs_allow_runtime_caching> : std::true_type {};

static_assert(kangaru::allow_runtime_caching_v<attrf_allow_runtime_caching>);
static_assert(kangaru::allow_runtime_caching_v<attrs_allow_runtime_caching>);

struct attrf_disable_runtime_caching {
	friend auto attribute(kangaru::allow_runtime_caching<attrf_disable_runtime_caching>)
		-> std::false_type;
};

struct attrs_disable_runtime_caching {};

template<>
struct kangaru::allow_runtime_caching<attrs_disable_runtime_caching> : std::false_type {};

static_assert(not kangaru::allow_runtime_caching_v<attrf_disable_runtime_caching>);
static_assert(not kangaru::allow_runtime_caching_v<attrs_disable_runtime_caching>);

// Allow empty injection
static_assert(kangaru::allow_empty_injection_v<default_value> == false);

struct attrf_allow_empty_injection {
	friend auto attribute(kangaru::allow_empty_injection<attrf_allow_empty_injection>)
		-> std::true_type;
};

struct attrs_allow_empty_injection {};

template<>
struct kangaru::allow_empty_injection<attrs_allow_empty_injection> : std::true_type {};

static_assert(kangaru::allow_empty_injection_v<attrf_allow_empty_injection>);
static_assert(kangaru::allow_empty_injection_v<attrs_allow_empty_injection>);

struct attrf_disable_empty_injection {
	friend auto attribute(kangaru::allow_empty_injection<attrf_disable_empty_injection>)
		-> std::false_type;
};

struct attrs_disable_empty_injection {};

template<>
struct kangaru::allow_empty_injection<attrs_disable_empty_injection> : std::false_type {};

static_assert(not kangaru::allow_empty_injection_v<attrf_disable_empty_injection>);
static_assert(not kangaru::allow_empty_injection_v<attrs_disable_empty_injection>);

// Assume runtime cached
static_assert(kangaru::assume_runtime_cached_v<default_value> == false);

struct attrf_assume_runtime_cached {
	friend auto attribute(kangaru::assume_runtime_cached<attrf_assume_runtime_cached>)
		-> std::true_type;
};

struct attrs_assume_runtime_cached {};

template<>
struct kangaru::assume_runtime_cached<attrs_assume_runtime_cached> : std::true_type {};

static_assert(kangaru::assume_runtime_cached_v<attrf_assume_runtime_cached>);
static_assert(kangaru::assume_runtime_cached_v<attrs_assume_runtime_cached>);

struct attrf_not_assume_runtime_cached {
	friend auto attribute(kangaru::assume_runtime_cached<attrf_not_assume_runtime_cached>)
		-> std::false_type;
};

struct attrs_not_assume_runtime_cached {};

template<>
struct kangaru::assume_runtime_cached<attrs_not_assume_runtime_cached> : std::false_type {};

static_assert(not kangaru::assume_runtime_cached_v<attrf_not_assume_runtime_cached>);
static_assert(not kangaru::assume_runtime_cached_v<attrs_not_assume_runtime_cached>);

// Allow injection using
static_assert(kangaru::allow_injection_using_v<default_value> == true);

struct attrf_not_allow_injection_using {
	friend auto attribute(kangaru::allow_injection_using<attrf_not_allow_injection_using>)
		-> std::false_type;
};

struct attrs_not_allow_injection_using {};

template<>
struct kangaru::allow_injection_using<attrs_not_allow_injection_using> : std::false_type {};

static_assert(not kangaru::allow_injection_using_v<attrf_not_allow_injection_using>);
static_assert(not kangaru::allow_injection_using_v<attrs_not_allow_injection_using>);

struct attrf_allow_injection_using {
	friend auto attribute(kangaru::allow_injection_using<attrf_allow_injection_using>)
		-> std::true_type;
};

struct attrs_allow_injection_using {};

template<>
struct kangaru::allow_injection_using<attrs_allow_injection_using> : std::true_type {};

static_assert(kangaru::allow_injection_using_v<attrf_allow_injection_using>);
static_assert(kangaru::allow_injection_using_v<attrs_allow_injection_using>);

// Second step init
static_assert(std::same_as<kangaru::noop_second_step, kangaru::second_step_init_t<default_value>>);

struct my_second_step {
	auto operator()(auto&, auto&&) -> void {}
};

struct attrf_second_step_init {
	friend auto attribute(kangaru::second_step_init<attrf_second_step_init>) -> my_second_step;
};

struct attrs_second_step_init {};

template<>
struct kangaru::second_step_init<attrs_second_step_init> {
	using type = my_second_step;
};

static_assert(std::same_as<my_second_step, kangaru::second_step_init_t<attrf_second_step_init>>);
static_assert(std::same_as<my_second_step, kangaru::second_step_init_t<attrs_second_step_init>>);

// Override types in cache
static_assert(std::same_as<std::tuple<>, kangaru::overrides_types_in_cache_t<default_value>>);

struct base_class {};

struct attrf_overrides_types_in_cache {
	friend auto attribute(kangaru::overrides_types_in_cache<attrf_overrides_types_in_cache>) -> std::tuple<base_class>;
};

struct attrs_overrides_types_in_cache {};

template<>
struct kangaru::overrides_types_in_cache<attrs_overrides_types_in_cache> {
	using type = std::tuple<base_class>;
};

static_assert(std::same_as<std::tuple<base_class>, kangaru::overrides_types_in_cache_t<attrf_overrides_types_in_cache>>);
static_assert(std::same_as<std::tuple<base_class>, kangaru::overrides_types_in_cache_t<attrs_overrides_types_in_cache>>);

TEST_CASE("allow_runtime_caching", "[attributes]") {
	// This test is successful and has at least one runtime assertion.
	REQUIRE(true);
}

