#include <kangaru/kangaru.hpp>

#include <catch2/catch_test_macros.hpp>

// Allow runtime caching
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
	friend auto attribute(kangaru::allow_empty_injection<attrf_disable_runtime_caching>)
		-> std::false_type;
};

struct attrs_disable_empty_injection {};

template<>
struct kangaru::allow_empty_injection<attrs_disable_runtime_caching> : std::false_type {};

static_assert(not kangaru::allow_empty_injection_v<attrf_disable_empty_injection>);
static_assert(not kangaru::allow_empty_injection_v<attrs_disable_empty_injection>);

// Assume runtime cached
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
	friend auto attribute(kangaru::assume_runtime_cached<attrf_disable_runtime_caching>)
		-> std::false_type;
};

struct attrs_not_assume_runtime_cached {};

template<>
struct kangaru::assume_runtime_cached<attrs_disable_runtime_caching> : std::false_type {};

static_assert(not kangaru::assume_runtime_cached_v<attrf_not_assume_runtime_cached>);
static_assert(not kangaru::assume_runtime_cached_v<attrs_not_assume_runtime_cached>);

TEST_CASE("allow_runtime_caching", "[attributes]") {
	REQUIRE(true);
}

