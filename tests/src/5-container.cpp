#include "kangaru/detail/source_types.hpp"
#include "kangaru/detail/attributes.hpp"
#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>

#include <fmt/core.h>

struct service_a {
	int i;
	friend auto attribute(kangaru::allow_runtime_caching<service_a&>) -> std::true_type;
};

struct service_b {
	service_a& a;
	
	friend auto attribute(kangaru::allow_runtime_caching<service_b&>) -> std::true_type;
};

struct service_aggregate {
	service_a& sa;
	service_b& sb;
};

struct service_c {
	explicit service_c(service_aggregate services) noexcept : services{services} {}
	service_aggregate services;
	
	friend auto attribute(kangaru::allow_runtime_caching<service_c&>) -> std::true_type;
};

TEST_CASE("Container act a bit like kangaru 4", "[container]") {
	auto container = kangaru::container{kangaru::none_source{}};

	SECTION("Allow replacing") {
		auto& a1 = container.provide<service_a&>();
		CHECK(container.has_in_cache<service_a&>());
		auto& a2 = container.replace([]{ return service_a{}; });
		auto& a3 = container.provide<service_a&>();
		
		CHECK(std::addressof(a1) != std::addressof(a2));
		CHECK(std::addressof(a2) == std::addressof(a3));
	}

	SECTION("Can create scoped instances") {
		auto& a1 = container.provide<service_a&>();
		{
			auto c = container.scoped();
			auto& a2 = container.provide<service_a&>();
			CHECK(std::addressof(a1) == std::addressof(a2));
			
			auto& b1 = container.provide<service_b&>();
			auto& b2 = c.provide<service_b&>();
			
			CHECK(std::addressof(b1) != std::addressof(b2));
			c.provide<service_c&>();
			CHECK(c.has_in_cache<service_c&>());
		}
		CHECK(not container.has_in_cache<service_c&>());
	}
	
	SECTION("Reuses instances") {
		auto& a = container.provide<service_a&>();
		auto& b = container.provide<service_b&>();
		CHECK(std::addressof(a) == std::addressof(b.a));
		
		auto& c = container.provide<service_c&>();
		
		CHECK(std::addressof(c.services.sa) == std::addressof(a));
	}
	
	SECTION("Works with injectors") {
		auto injector = kangaru::make_spread_injector(kangaru::ref(container));
		
		injector([](service_a& a, service_c& c) {
			CHECK(std::addressof(c.services.sa) == std::addressof(a));
			CHECK(std::addressof(c.services.sb.a) == std::addressof(a));
		});
	}
}

struct service_aa1 : service_a {
	friend auto attribute(kangaru::allow_runtime_caching<service_aa1&>) -> std::true_type;
	friend auto attribute(kangaru::overrides_types_in_cache<service_aa1&>) -> std::tuple<service_a&>;
};

struct service_aa2 : service_a {
	friend auto attribute(kangaru::allow_runtime_caching<service_aa2&>) -> std::true_type;
	friend auto attribute(kangaru::overrides_types_in_cache<service_aa2&>) -> std::tuple<service_a&>;
};

TEST_CASE("Container act a bit like kangaru 4 with polymorphic services", "[container]") {
	auto container = kangaru::polymorphic_container{kangaru::none_source{}};
	
	SECTION("Allow replacing instances that overrides") {
		auto& aa = container.provide<service_aa1&>();
		CHECK(container.has_in_cache<service_aa1&>());
		CHECK(container.has_in_cache<service_a&>());
		aa.i = 9;
		
		auto& aaa = container.replace([] {
			return service_aa2{service_a{.i = 5}};
		});
		
		auto& a = container.provide<service_a&>();
		CHECK(a.i == 5);
		CHECK(std::addressof(static_cast<service_a&>(aaa)) == std::addressof(a));
	}

	SECTION("Can create scoped instances") {
		auto& a1 = container.provide<service_aa1&>();
		{
			auto c = container.scoped();
			auto& a2 = container.provide<service_a&>();
			CHECK(std::addressof(a1) == std::addressof(a2));
			
			{
				container.replace([]{ return service_aa2{}; });
				auto& a1 = container.provide<service_aa1&>();
				auto& a2 = container.provide<service_a&>();
				CHECK(std::addressof(a1) != std::addressof(a2));
			}
		}
		
	}
	
	SECTION("Reuse instances that can override each other") {
		auto& aa = container.provide<service_aa1&>();
		aa.i = 9;
		
		auto& a = container.provide<service_a&>();
		auto& b = container.provide<service_b&>();
		
		CHECK(a.i == 9);
		CHECK(std::addressof(a) == std::addressof(b.a));
		CHECK(std::addressof(aa) == std::addressof(static_cast<service_aa1&>(b.a)));
		
		auto& c = container.provide<service_c&>();
		
		CHECK(std::addressof(c.services.sa) == std::addressof(static_cast<service_aa1&>(a)));
	}
	
	SECTION("Can use injectors") {
		auto injector = kangaru::make_spread_injector(kangaru::ref(container));
		
		auto& aa = container.provide<service_aa1&>();
		injector([&](service_a& a, service_c& c) {
			CHECK(std::addressof(c.services.sa) == std::addressof(a));
			CHECK(std::addressof(c.services.sb.a) == std::addressof(a));
			CHECK(std::addressof(aa) == std::addressof(static_cast<service_aa1&>(a)));
		});
	}
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

