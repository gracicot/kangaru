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

struct service_d {
	explicit service_d(service_c& c, service_aggregate services) noexcept : sc{c}, services{services} {}
	service_c& sc;
	service_aggregate services;
	
	friend auto attribute(kangaru::allow_runtime_caching<std::shared_ptr<service_d>>) -> std::true_type;
};

struct int_source {
	auto provide() & -> int {
		return 3;
	}
	
	auto provide() && -> int {
		return 4;
	}
	
	auto provide() const& -> int {
		return 5;
	}
	
	auto provide() const&& -> int {
		return 6;
	}
};

struct service_a_child_1 : service_a {
	friend auto attribute(kangaru::allow_runtime_caching<service_a_child_1&>) -> std::true_type;
	friend auto attribute(kangaru::overrides_types_in_cache<service_a_child_1&>) -> std::tuple<service_a&>;
};

struct service_a_child_2 : service_a {
	friend auto attribute(kangaru::allow_runtime_caching<service_a_child_2&>) -> std::true_type;
	friend auto attribute(kangaru::overrides_types_in_cache<service_a_child_2&>) -> std::tuple<service_a&>;
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

struct has_second_step {
	int a = 0;
	
	static auto init(has_second_step& instance) -> void {
		instance.a = 9;
	}
	
	friend auto attribute(kangaru::allow_runtime_caching<has_second_step&>) -> std::true_type;
	friend auto attribute(kangaru::second_step_init<has_second_step&>) -> kangaru::call_function<
		[](has_second_step& instance, kangaru::forwarded_source auto&& source) {
			has_second_step::init(instance);
		}
	>;
};

struct non_cached_with_second_step {
	explicit constexpr non_cached_with_second_step(has_second_step first) : first{first} {}
	has_second_step first;
	int a = 0;
	
	static auto init(non_cached_with_second_step& instance) -> void {
		instance.a = 9;
	}
	
	friend auto attribute(kangaru::second_step_init<non_cached_with_second_step>) -> kangaru::call_function<
		[](non_cached_with_second_step& instance, kangaru::forwarded_source auto&& source) {
			non_cached_with_second_step::init(instance);
		}
	>;
};

TEST_CASE("Container act a bit like kangaru 4", "[container]") {
	auto container = kangaru::container{};
	
	SECTION("Allow replacing") {
		auto& a1 = container.provide<service_a&>();
		CHECK(container.has_in_cache<service_a&>());
		auto& a2 = container.replace<service_a&>(kangaru::reference_source<service_a>{});
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

	SECTION("Supports provided services") {
		auto base = kangaru::object_source{kangaru::reference_source{concrete{8.5f}}};
		auto container = kangaru::container{base};
		auto& c = kangaru::provide<concrete&>(container);
		CHECK(c.value == 8.5f);
	}
	
	SECTION("Allow injection using shared pointers") {
		auto container = kangaru::container{};
		
		SECTION("Simple caching of shared pointers") {
			auto s = kangaru::provide<std::shared_ptr<shared_abstract>>(container);
			CHECK(s == kangaru::provide<std::shared_ptr<shared_abstract>>(container));
		}
	}
}

TEST_CASE("Polymorphic container act a bit like kangaru 4 with polymorphic services", "[container]") {
	auto container = kangaru::polymorphic_container{};
	
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
				container.replace<service_a_child_2&>(kangaru::reference_source{service_a_child_2{}});
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
	
	SECTION("Supports second step for cached types") {
		auto& a = kangaru::provide<has_second_step&>(container);
		CHECK(a.a == 9);
		auto b = kangaru::provide<non_cached_with_second_step>(container);
		CHECK(b.a == 9);
	}
}

TEST_CASE("Container uses the base source") {
	SECTION("Forwards") {
		auto container = kangaru::container{int_source{}};
		
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

	SECTION("container base source with factory functions") {
		auto base = kangaru::make_container_base_source(
			kangaru::constructor_function<kangaru::reference_source<dependent_on_provided>>{},
			[]() -> kangaru::object_source<std::shared_ptr<dynamic_provided_abstract>> {
				return kangaru::object_source<std::shared_ptr<dynamic_provided_abstract>>{std::make_shared<dynamic_provided_concrete>(3)};
			}
		);
		
		auto container = kangaru::container{base};
		auto provided = kangaru::provide<std::shared_ptr<dynamic_provided_abstract>>(container);
		
		CHECK(provided->value == 3);
		CHECK(kangaru::provide<dependent_on_provided>(container).ptr == provided);
	}
	
	SECTION("container base source with dynamic supplied instances") {
		auto container = kangaru::container{
			kangaru::make_container_base_source(kangaru::allow_assume_cached)
		};
		
		CHECK_THROWS_AS(kangaru::provide<std::shared_ptr<dynamic_provided_abstract>>(container), kangaru::not_found_exception);
		
		SECTION("Can provide the instance dynamically using replace") {
			auto provided = container.replace<std::shared_ptr<dynamic_provided_concrete>>(
				kangaru::make_in_place<kangaru::shared_pointer_source<dynamic_provided_concrete>>(15)
			);
			
			auto from_container = kangaru::provide<std::shared_ptr<dynamic_provided_concrete>>(container);
			CHECK(from_container == provided);
			CHECK(from_container->value == 15);
		}
	}
	
	SECTION("container base source with dynamic supplied instances and factory functions") {
		auto base = kangaru::make_container_base_source(
			kangaru::allow_assume_cached,
			kangaru::throw_if_not_found{},
			kangaru::constructor_function<kangaru::reference_source<dependent_on_provided>>{},
			[]() -> kangaru::object_source<std::shared_ptr<dynamic_provided_abstract>> {
				return kangaru::object_source<std::shared_ptr<dynamic_provided_abstract>>{std::make_shared<dynamic_provided_concrete>(3)};
			}
		);
		
		auto container = kangaru::polymorphic_container{base};
		auto provided = kangaru::provide<std::shared_ptr<dynamic_provided_abstract>>(container);
		
		CHECK(provided->value == 3);
		CHECK(kangaru::provide<dependent_on_provided>(container).ptr == provided);
	}
	
	SECTION("Simple caching of shared pointers") {
		auto container = kangaru::container{};
		auto s = kangaru::provide<std::shared_ptr<service_d>>(container);
		CHECK(s == kangaru::provide<std::shared_ptr<service_d>>(container));
		CHECK(container.has_in_cache<std::shared_ptr<service_d>>());
	}
}

TEST_CASE("Polymorphic container uses the base source", "[container]") {
	SECTION("Forwards") {
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
	
	SECTION("Supports provided services") {
		auto base = kangaru::object_source{kangaru::reference_source{concrete{8.5f}}};
		auto container = kangaru::polymorphic_container{base};
		auto& c = kangaru::provide<concrete&>(container);
		CHECK(c.value == 8.5f);
	}
	
	SECTION("Allow injection using shared pointers") {
		auto container = kangaru::polymorphic_container{};
		
		SECTION("Simple caching of shared pointers") {
			auto s = kangaru::provide<std::shared_ptr<shared_abstract>>(container);
			CHECK(s == kangaru::provide<std::shared_ptr<shared_abstract>>(container));
		}
		
		SECTION("Allow shared pointers to override") {
			kangaru::provide<std::shared_ptr<shared_concrete>>(container)->value = 16;
			CHECK(kangaru::provide<std::shared_ptr<shared_abstract>>(container)->value == 16);
		}
	}
	
	SECTION("container base source with factory functions") {
		auto base = kangaru::make_container_base_source(
			kangaru::constructor_function<kangaru::reference_source<dependent_on_provided>>{},
			[]() -> kangaru::object_source<std::shared_ptr<dynamic_provided_abstract>> {
				return kangaru::object_source<std::shared_ptr<dynamic_provided_abstract>>{std::make_shared<dynamic_provided_concrete>(3)};
			}
		);
		
		auto container = kangaru::polymorphic_container{base};
		auto provided = kangaru::provide<std::shared_ptr<dynamic_provided_abstract>>(container);
		
		CHECK(provided->value == 3);
		CHECK(kangaru::provide<dependent_on_provided>(container).ptr == provided);
	}
	
	SECTION("container base source with dynamic supplied instances") {
		auto container = kangaru::polymorphic_container{
			kangaru::make_container_base_source(kangaru::allow_assume_cached)
		};
		
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
	
	SECTION("container base source with dynamic supplied instances and factory functions") {
		auto base = kangaru::make_container_base_source(
			kangaru::allow_assume_cached,
			kangaru::throw_if_not_found{},
			kangaru::constructor_function<kangaru::reference_source<dependent_on_provided>>{},
			[]() -> kangaru::object_source<std::shared_ptr<dynamic_provided_abstract>> {
				return kangaru::object_source<std::shared_ptr<dynamic_provided_abstract>>{std::make_shared<dynamic_provided_concrete>(3)};
			}
		);
		
		auto container = kangaru::polymorphic_container{base};
		auto provided = kangaru::provide<std::shared_ptr<dynamic_provided_abstract>>(container);
		
		CHECK(provided->value == 3);
		CHECK(kangaru::provide<dependent_on_provided>(container).ptr == provided);
	}
	
	SECTION("Supports second step for cached types") {
		auto container = kangaru::polymorphic_container{};
		auto& a = kangaru::provide<has_second_step&>(container);
		CHECK(a.a == 9);
		auto b = kangaru::provide<non_cached_with_second_step>(container);
		CHECK(b.a == 9);
	}
}

