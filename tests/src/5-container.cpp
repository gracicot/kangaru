#include "kangaru/detail/source_types.hpp"
#include "kangaru/detail/tag.hpp"
#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

#include <fmt/core.h>

struct service_a {
	int i;
	friend auto config(kangaru::allow_runtime_caching<service_a&>) -> std::true_type;
};

struct service_b {
	service_a& a;
	
	friend auto config(kangaru::allow_runtime_caching<service_b&>) -> std::true_type;
};

struct service_aggregate {
	service_a& sa;
	service_b& sb;
};

struct service_c {
	explicit service_c(service_aggregate services) noexcept : services{services} {}
	service_aggregate services;
	
	friend auto config(kangaru::allow_runtime_caching<service_c&>) -> std::true_type;
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
	friend auto config(kangaru::allow_runtime_caching<service_aa&>) -> std::true_type;
	friend auto config(kangaru::overrides_types_in_cache<service_aa&>) -> std::tuple<service_a&>;
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
	SECTION("Normal") {
		auto container = kangaru::container{int_source{}};
		
		auto const i = container.provide<int>();
		CHECK(i == 3);
		
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
	
	SECTION("Polymorphic") {
		auto container = kangaru::polymorphic_container{int_source{}};
		
		auto const i = container.provide<int>();
		CHECK(i == 3);
		
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
}

