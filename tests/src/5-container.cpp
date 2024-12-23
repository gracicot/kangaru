#include "kangaru/detail/tag.hpp"
#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

#include <fmt/core.h>

struct service_a {
	friend auto tag(kangaru::tag_for<service_a&>) -> kangaru::tags<kangaru::cached>;
};

struct service_b {
	service_a& a;
	
	friend auto tag(kangaru::tag_for<service_b&>) -> kangaru::tags<kangaru::cached>;
};

struct service_aggregate {
	service_a& sa;
	service_b& sb;
};

struct service_c {
	explicit service_c(service_aggregate services) noexcept : services{services} {}
	service_aggregate services;
	
	friend auto tag(kangaru::tag_for<service_c&>) -> kangaru::tags<kangaru::cached>;
};

static_assert(kangaru::is_cachable_v<service_a&>);
static_assert(kangaru::is_cachable_v<service_b&>);
static_assert(kangaru::is_cachable_v<service_c&>);

TEST_CASE("Container act a bit like kangaru 4", "[container]") {
	auto container = kangaru::dynamic_container{kangaru::none_source{}};
	
	auto& a = container.provide<service_a&>();
	auto& b = container.provide<service_b&>();
	CHECK(std::addressof(a) == std::addressof(b.a));
	
	auto& c = container.provide<service_c&>();
	
	CHECK(std::addressof(c.services.sa) == std::addressof(a));
	
	auto injector = kangaru::make_spread_injector(kangaru::ref(container));
	
	injector([](service_a& a, service_c& c) {
		CHECK(std::addressof(c.services.sa) == std::addressof(a));
		CHECK(std::addressof(c.services.sb.a) == std::addressof(a));
	});
}
