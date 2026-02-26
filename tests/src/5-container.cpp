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

struct service_a_child_1 : service_a {
	friend auto attribute(kangaru::allow_runtime_caching<service_a_child_1&>) -> std::true_type;
	friend auto attribute(kangaru::overrides_types_in_cache<service_a_child_1&>) -> std::tuple<service_a&>;
};

struct service_a_child_2 : service_a {
	friend auto attribute(kangaru::allow_runtime_caching<service_a_child_2&>) -> std::true_type;
	friend auto attribute(kangaru::overrides_types_in_cache<service_a_child_2&>) -> std::tuple<service_a&>;
};

TEST_CASE("Container act a bit like kangaru 4 with polymorphic services", "[container]") {
	auto container = kangaru::polymorphic_container{kangaru::none_source{}};
	
	SECTION("Allow replacing instances that overrides") {
		auto& ac1 = container.provide<service_a_child_1&>();
		CHECK(container.has_in_cache<service_a_child_1&>());
		CHECK(container.has_in_cache<service_a&>());
		ac1.i = 9;
		
		auto injector = kangaru::make_spread_injector(kangaru::ref(container));
		auto& ac2 = container.replace<service_a_child_2&>(kangaru::in_place_construct{[&]{
			return injector([](service_a& previous) {
				return kangaru::reference_source{service_a_child_2{service_a{.i = previous.i - 1}}};
			});
		}});
		
		auto& a = container.provide<service_a&>();
		CHECK(a.i == 8);
		CHECK(std::addressof(static_cast<service_a&>(ac2)) == std::addressof(a));
	}
	
	SECTION("Can create scoped instances") {
		auto& a1 = container.provide<service_a_child_1&>();
		{
			auto c = container.scoped();
			auto& a = container.provide<service_a&>();
			CHECK(std::addressof(a1) == std::addressof(a));
			
			{
				container.replace<service_a_child_2&>(kangaru::in_place_construct{[]{ return kangaru::reference_source{service_a_child_2{}}; }});
				auto& a1 = container.provide<service_a_child_1&>();
				auto& a = container.provide<service_a&>();
				CHECK(std::addressof(a1) != std::addressof(a));
			}
		}
	}
	
	SECTION("Reuse instances that can override each other") {
		auto& a1 = container.provide<service_a_child_1&>();
		a1.i = 9;
		
		auto& a = container.provide<service_a&>();
		auto& b = container.provide<service_b&>();
		
		CHECK(a.i == 9);
		CHECK(std::addressof(a) == std::addressof(b.a));
		CHECK(std::addressof(a1) == std::addressof(static_cast<service_a_child_1&>(b.a)));
		
		auto& c = container.provide<service_c&>();
		
		CHECK(std::addressof(c.services.sa) == std::addressof(static_cast<service_a_child_1&>(a)));
	}
	
	SECTION("Can use injectors") {
		auto injector = kangaru::make_spread_injector(kangaru::ref(container));
		
		auto& a1 = container.provide<service_a_child_1&>();
		injector([&](service_a& a, service_c& c) {
			CHECK(std::addressof(c.services.sa) == std::addressof(a));
			CHECK(std::addressof(c.services.sb.a) == std::addressof(a));
			CHECK(std::addressof(a1) == std::addressof(static_cast<service_a_child_1&>(a)));
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

struct abstract {
	abstract(float value) : value{value} {}
	virtual ~abstract() = 0;
	float value;
};

abstract::~abstract() = default;

struct concrete : abstract {
	concrete(float value) : abstract{value} {}
};

struct shared_abstract {
	std::int32_t value;
	friend auto attribute(kangaru::allow_runtime_caching<std::shared_ptr<shared_abstract>>) -> std::true_type;
};

struct shared_concrete : shared_abstract {
	friend auto attribute(kangaru::allow_runtime_caching<std::shared_ptr<shared_concrete>>) -> std::true_type;
	friend auto attribute(kangaru::overrides_types_in_cache<std::shared_ptr<shared_concrete>>)
		-> std::tuple<std::shared_ptr<shared_abstract>>;
};

struct dynamic_provided_abstract {
	explicit constexpr dynamic_provided_abstract(std::int32_t value) : value{value} {}
	virtual ~dynamic_provided_abstract() = 0;
	
	std::int32_t value;
	
	friend auto attribute(kangaru::assume_runtime_cached<std::shared_ptr<dynamic_provided_abstract>>) -> std::true_type;
};

dynamic_provided_abstract::~dynamic_provided_abstract() = default;

struct dynamic_provided_concrete : dynamic_provided_abstract {
	explicit constexpr dynamic_provided_concrete(std::int32_t value) : dynamic_provided_abstract{value} {}
	
	friend auto attribute(kangaru::assume_runtime_cached<std::shared_ptr<dynamic_provided_concrete>>) -> std::true_type;
	friend auto attribute(kangaru::overrides_types_in_cache<std::shared_ptr<dynamic_provided_concrete>>)
		-> std::tuple<std::shared_ptr<dynamic_provided_abstract>>;
};

struct dependent_on_provided {
	std::shared_ptr<dynamic_provided_abstract> ptr;
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
	
	SECTION("Supports provided services") {
		auto base = kangaru::object_source{kangaru::reference_source{concrete{8.5f}}};
		auto container = kangaru::polymorphic_container{base};
		auto& c = kangaru::provide<concrete&>(container);
		CHECK(c.value == 8.5f);
	}
	
	SECTION("Allow injection using shared pointers") {
		auto container = kangaru::polymorphic_container{kangaru::none_source{}};
		
		SECTION("Simple caching of shared pointers") {
			auto s = kangaru::provide<std::shared_ptr<shared_abstract>>(container);
			CHECK(s == kangaru::provide<std::shared_ptr<shared_abstract>>(container));
		}
		
		SECTION("Allow shared pointers to override") {
			kangaru::provide<std::shared_ptr<shared_concrete>>(container)->value = 16;
			CHECK(kangaru::provide<std::shared_ptr<shared_abstract>>(container)->value == 16);
		}
	}
	
	SECTION("container base") {
		// TODO: Simpler syntax
		auto base = kangaru::enumerate_source<
			kangaru::reference_source<dependent_on_provided>,
			kangaru::object_source<std::shared_ptr<dynamic_provided_abstract>>
		>(kangaru::with_function_call{
			kangaru::none_source{},
			kangaru::call_with_injector{
				kangaru::constructor_function<kangaru::reference_source<dependent_on_provided>>{},
				kangaru::make_strict_spread_injector_function{},
			},
			kangaru::call_with_injector{
				[]() -> kangaru::object_source<std::shared_ptr<dynamic_provided_abstract>> {
					return kangaru::object_source<std::shared_ptr<dynamic_provided_abstract>>{std::make_shared<dynamic_provided_concrete>(3)};
				},
				kangaru::make_strict_spread_injector_function{},
			},
		});
		
		auto container = kangaru::polymorphic_container{base};
		auto provided = kangaru::provide<std::shared_ptr<dynamic_provided_abstract>>(container);
		
		CHECK(provided->value == 3);
		CHECK(kangaru::provide<dependent_on_provided>(container).ptr == provided);
	}
	
	SECTION("container provided") {
		auto base = kangaru::enumerate_source(kangaru::object_source{kangaru::throw_if_not_found{}});
		
		auto container = kangaru::polymorphic_container{base};
		
		CHECK_THROWS_AS(kangaru::provide<std::shared_ptr<dynamic_provided_abstract>>(container), kangaru::not_found_exception);
		
		SECTION("Can provide the instance dynamically using replace") {
			auto provided = container.replace<std::shared_ptr<dynamic_provided_concrete>>(
				kangaru::make_in_place<kangaru::shared_pointer_source<dynamic_provided_concrete>>(15)
			);
			
			auto from_container = kangaru::provide<std::shared_ptr<dynamic_provided_abstract>>(container);
			CHECK(from_container == provided);
			CHECK(from_container->value == 15);
		}
	}
	
	SECTION("container provided") {
		auto base = kangaru::enumerate_source<
			kangaru::reference_source<dependent_on_provided>,
			kangaru::object_source<std::shared_ptr<dynamic_provided_abstract>>,
			kangaru::throw_if_not_found
		>(kangaru::compose(
				kangaru::object_source{kangaru::throw_if_not_found{}},
				kangaru::with_function_call{
					kangaru::call_with_injector{
						kangaru::constructor_function<kangaru::reference_source<dependent_on_provided>>{},
						kangaru::make_strict_spread_injector_function{},
					},
					kangaru::call_with_injector{
						[]() -> kangaru::object_source<std::shared_ptr<dynamic_provided_abstract>> {
							return kangaru::object_source<std::shared_ptr<dynamic_provided_abstract>>{std::make_shared<dynamic_provided_concrete>(3)};
						},
						kangaru::make_strict_spread_injector_function{},
					},
				}
			)
		);
		
		auto container = kangaru::polymorphic_container{base};
		auto provided = kangaru::provide<std::shared_ptr<dynamic_provided_abstract>>(container);
		
		CHECK(provided->value == 3);
		CHECK(kangaru::provide<dependent_on_provided>(container).ptr == provided);
	}
}

