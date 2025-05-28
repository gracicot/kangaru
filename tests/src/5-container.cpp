#include "kangaru/detail/source_types.hpp"
#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

#include <fmt/core.h>

struct service_a {
	int i;
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

TEST_CASE("Container act a bit like kangaru 4", "[container]") {
	auto container = kangaru::container{kangaru::none_source{}};
	
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

struct service_aa : service_a {
	// TODO: Better syntax for tags
	friend auto tag(kangaru::tag_for<service_aa&>) -> kangaru::tags<kangaru::cached, kangaru::overrides<service_a&>>;
};

TEST_CASE("Container act a bit like kangaru 4 with polymorphic services", "[container]") {
	auto container = kangaru::polymorphic_container{kangaru::none_source{}};
	
	auto& aa = container.provide<service_aa&>();
	aa.i = 9;
	
	auto& a = container.provide<service_a&>();
	CHECK(a.i == 9);
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

struct int_source {
	auto provide() const& -> int {
		return 3;
	}
	
	auto provide() && -> int {
		return 4;
	}
};

TEST_CASE("Container can have a base source", "[container]") {
	auto container = kangaru::polymorphic_container{int_source{}};
	
	SECTION("lvalues") {
		auto& a = container.provide<service_a&>();
		CHECK(a.i == 3);
	}
	
	SECTION("rvalues") {
		auto& a = std::move(container).provide<service_a&>();
		CHECK(a.i == 4);
	}
	
	SECTION("lvalues deep") {
		auto& c = container.provide<service_c&>();
		CHECK(c.services.sa.i == 3);
	}
	
	SECTION("rvalues deep") {
		auto& c = std::move(container).provide<service_c&>();
		CHECK(c.services.sa.i == 4);
	}
}

TEST_CASE("Can use any_copiable_source_of as type erased source") {
	SECTION("const") {
		auto a = service_a{};
		auto source = kangaru::any_copiable_source_of<service_a&>{kangaru::external_reference_source<service_a>{a}};
		
		CHECK(std::addressof(kangaru::provide<service_a&>(source)) == std::addressof(a));
	}
	
	SECTION("non const") {
		auto source = kangaru::any_copiable_source_of<service_a&>{kangaru::reference_source<service_a>{}};
		CHECK(kangaru::source_of<decltype(source), service_a&>);
	}
}

