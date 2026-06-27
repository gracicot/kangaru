#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_template_test_macros.hpp>
#include <kangaru/kangaru.hpp>
#include <map>

#include <fmt/core.h>

template<typename T>
struct unmapped_dependent_on {
	explicit unmapped_dependent_on(T value) : value(std::move(value)) {}
	T value;
};

template<typename T>
struct mapped_dependent_on {
	explicit mapped_dependent_on(T value) : value(std::move(value)) {}
	T value;
	
	template<typename U>
	friend auto attribute(kangaru::allow_runtime_caching<mapped_dependent_on<U>>) -> std::true_type;
};

template<typename T, typename C>
auto test_provide(C& container, auto check) {
	SECTION("Provide direct") {
		decltype(auto) result = kangaru::provide<T>(container);
		check(static_cast<T>(result));
	}
	
	SECTION("Provide through a mapped type") {
		auto result = kangaru::provide<mapped_dependent_on<T>>(container);
		check(static_cast<T>(result.value));
		
		SECTION("then through mapped") {
			auto result = kangaru::provide<mapped_dependent_on<mapped_dependent_on<T>>>(container);
			check(static_cast<T>(result.value.value));
			
			SECTION("then through mapped") {
				auto result = kangaru::provide<mapped_dependent_on<mapped_dependent_on<mapped_dependent_on<T>>>>(container);
				check(static_cast<T>(result.value.value.value));
			}
		}
		
		SECTION("then through unmapped") {
			auto result = kangaru::provide<unmapped_dependent_on<mapped_dependent_on<T>>>(container);
			check(static_cast<T>(result.value.value));
			
			SECTION("then through unmapped") {
				auto result = kangaru::provide<unmapped_dependent_on<mapped_dependent_on<mapped_dependent_on<T>>>>(container);
				check(static_cast<T>(result.value.value.value));
				
				SECTION("then through mapped") {
					auto result = kangaru::provide<mapped_dependent_on<unmapped_dependent_on<mapped_dependent_on<mapped_dependent_on<T>>>>>(container);
					check(static_cast<T>(result.value.value.value.value));
				}
				
				SECTION("then through unmapped") {
					auto result = kangaru::provide<unmapped_dependent_on<unmapped_dependent_on<mapped_dependent_on<mapped_dependent_on<T>>>>>(container);
					check(static_cast<T>(result.value.value.value.value));
				}
			}
		}
	}
	
	SECTION("Provide through a unmapped type") {
		auto result = kangaru::provide<unmapped_dependent_on<T>>(container);
		check(static_cast<T>(result.value));
		
		SECTION("then through mapped") {
			auto result = kangaru::provide<mapped_dependent_on<unmapped_dependent_on<T>>>(container);
			check(static_cast<T>(result.value.value));
		}
		
		SECTION("then through unmapped") {
			auto result = kangaru::provide<unmapped_dependent_on<unmapped_dependent_on<T>>>(container);
			check(static_cast<T>(result.value.value));
			
			SECTION("then through unmapped") {
				auto result = kangaru::provide<unmapped_dependent_on<unmapped_dependent_on<unmapped_dependent_on<T>>>>(container);
				check(static_cast<T>(result.value.value.value));
			}
		}
	}
}

struct empty {};
struct empty_injectable {};

template<>
struct kangaru::allow_empty_injection<empty_injectable> : std::true_type {};

struct service_a {
	int i;
	friend auto attribute(kangaru::allow_runtime_caching<service_a&>) -> std::true_type;
};

struct service_b {
	service_a& a;
	
	friend auto attribute(kangaru::allow_runtime_caching<service_b&>) -> std::true_type;
};

struct service_non_strict {
	service_a const& a;
	
	friend auto attribute(kangaru::allow_runtime_caching<service_non_strict&>) -> std::true_type;
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

struct int_wrapper {
	int i;
};

struct service_a_child_1 : service_a {
	service_a_child_1() = default;
	explicit service_a_child_1(int i) : service_a{i} {}
	friend auto attribute(kangaru::allow_runtime_caching<service_a_child_1&>) -> std::true_type;
	friend auto attribute(kangaru::overrides_types_in_cache<service_a_child_1&>) -> std::tuple<service_a&>;
};

struct service_a_child_2 : service_a {
	service_a_child_2() = default;
	explicit service_a_child_2(int i) : service_a{i} {}
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
		instance.a += 9;
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
		instance.a += 9;
	}
	
	friend auto attribute(kangaru::second_step_init<non_cached_with_second_step>) -> kangaru::call_function<
		[](non_cached_with_second_step& instance, kangaru::forwarded_source auto&& source) {
			non_cached_with_second_step::init(instance);
		}
	>;
};

struct alias_container {
	static auto make_container(auto&&... args) {
		return kangaru::container{std::forward<decltype(args)>(args)...};
	}
};

struct alias_polymorphic_container {
	static auto make_container(auto&&... args) {
		return kangaru::polymorphic_container{std::forward<decltype(args)>(args)...};
	}
};

TEMPLATE_TEST_CASE("Container act a bit like kangaru 4", "[container]",
	(alias_container),
	(alias_polymorphic_container)
) {
	auto container = TestType::make_container();
	
	static_assert(not kangaru::source_of<decltype(container)&, empty>);
	static_assert(kangaru::source_of<decltype(container)&, empty_injectable>);
	
	if constexpr (std::same_as<TestType, alias_polymorphic_container>) {
		auto c = TestType::make_container(kangaru::none_source{}, kangaru::exhaustive_construction{}, kangaru::polymorphic_map<std::map<kangaru::type_id, kangaru::any_source_of_one_ref>>{});
		c.template provide<service_c&>();
	} else {
		auto c = TestType::make_container(kangaru::none_source{}, kangaru::exhaustive_construction{}, std::map<kangaru::type_id, void*>{});
		c.template provide<service_c&>();
	}
	
	SECTION("Default construction is non strict") {
		static_assert(kangaru::source_of<decltype(container)&, service_non_strict>);
		
		SECTION("Can replace construction with strict") {
			if constexpr (std::same_as<TestType, alias_container>) {
				auto container = TestType::make_container(
					kangaru::none_source{},
					kangaru::exhaustive_strict_construction{},
					std::unordered_map<kangaru::type_id, void*>{},
					kangaru::default_heap_storage{}
				);
				
				static_assert(kangaru::source_of<decltype(container)&, service_b&>);
				static_assert(not kangaru::source_of<decltype(container)&, service_non_strict&>);
			} else {
				auto container = TestType::make_container(
					kangaru::none_source{},
					kangaru::exhaustive_strict_construction{},
					kangaru::polymorphic_map<std::unordered_map<kangaru::type_id, kangaru::any_source_of_one_ref>>{},
					kangaru::default_heap_storage{}
				);
				
				static_assert(kangaru::source_of<decltype(container)&, service_b&>);
				static_assert(not kangaru::source_of<decltype(container)&, service_non_strict&>);
			}
		}
	}
	
	SECTION("Allow replacing") {
		auto& a1 = container.template provide<service_a&>();
		CHECK(container.template has_in_cache<service_a&>());
		auto& a2 = container.template replace<service_a&>(kangaru::reference_source<service_a>{});
		auto& a3 = container.template provide<service_a&>();
		
		CHECK(std::addressof(a1) != std::addressof(a2));
		CHECK(std::addressof(a2) == std::addressof(a3));
	}
	
	SECTION("Containers are movable") {
		auto& a1 = container.template provide<service_a&>();
		auto c = std::move(container);
		container = {};
		auto& a2 = c.template provide<service_a&>();
		CHECK(std::addressof(a1) == std::addressof(a2));
	}
	
	SECTION("Allow replacing instances") {
		SECTION("Replacing keeps old instance alive") {
			auto& b = container.template provide<service_b&>();
			REQUIRE(container.template has_in_cache<service_a&>());
			REQUIRE(container.template has_in_cache<service_b&>());
			
			SECTION("Can get directly from cache") {
				auto b_ref = std::as_const(container).template get_from_cache<service_b&>();
				REQUIRE(b_ref);
				CHECK(std::addressof(*b_ref) == std::addressof(b));
			}
			
			container.template replace<service_a&>(kangaru::reference_source{service_a{}});
			auto& a = kangaru::provide<service_a&>(container);
			CHECK(std::addressof(a) != std::addressof(b.a));
		}
		
		SECTION("Replacing without override") {
			REQUIRE(not container.template has_in_cache<service_a&>());
			
			SECTION("Access empty cache") {
				auto a_ref = std::as_const(container).template get_from_cache<service_a&>();
				REQUIRE(not a_ref);
			}
			
			auto& a1 = container.template provide<service_a&>();
			REQUIRE(container.template has_in_cache<service_a&>());
			a1.i = 8;
				
			auto injector = kangaru::make_spread_injector(kangaru::ref(container));
			auto& ac2 = container.template replace<service_a&>(kangaru::in_place_construct{[&]{
				return injector([](service_a& previous) {
					return kangaru::reference_source{service_a{previous.i - 1}};
				});
			}});
			
			auto& a2 = container.template provide<service_a&>();
			CHECK(a2.i == 7);
			
			container.template replace<service_a&>(kangaru::reference_source{service_a{2}});
			
			CHECK(kangaru::provide<service_a&>(container).i == 2);
		}
		
		SECTION("Replacing with override") {
			if constexpr (std::same_as<TestType, alias_polymorphic_container>) {
				auto& ac1 = container.template provide<service_a_child_1&>();
				CHECK(container.template has_in_cache<service_a_child_1&>());
				CHECK(container.template has_in_cache<service_a&>());
				ac1.i = 9;
				
				auto injector = kangaru::make_spread_injector(kangaru::ref(container));
				auto& ac2 = container.template replace<service_a_child_2&>(kangaru::in_place_construct{[&]{
					return injector([](service_a& previous) {
						return kangaru::reference_source{service_a_child_2{previous.i - 1}};
					});
				}});
				
				auto& a = container.template provide<service_a&>();
				CHECK(a.i == 8);
				CHECK(std::addressof(static_cast<service_a&>(ac2)) == std::addressof(a));
			}
		}
	}
	
	SECTION("Can create scoped instances") {
		auto& a1 = std::same_as<TestType, alias_polymorphic_container>
			? static_cast<service_a&>(container.template provide<service_a_child_1&>())
			: container.template provide<service_a&>();
		
		SECTION("Scoping inherit the parent's instances") {
			auto c = container.scoped();
			auto& a2 = container.template provide<service_a&>();
			CHECK(std::addressof(a1) == std::addressof(a2));
			
			auto& b1 = container.template provide<service_b&>();
			auto& b2 = c.template provide<service_b&>();
			
			CHECK(std::addressof(b1) != std::addressof(b2));
			c.template provide<service_c&>();
			CHECK(c.template has_in_cache<service_c&>());
			
			if constexpr (std::same_as<TestType, alias_polymorphic_container>) {
				container.template replace<service_a_child_2&>(kangaru::reference_source{service_a_child_2{}});
				auto& a1 = container.template provide<service_a_child_1&>();
				auto& a = container.template provide<service_a&>();
				CHECK(std::addressof(a1) != std::addressof(a));
			}
		}
		
		SECTION("Scoping erase don't affect parent or child") {
			auto c = container.scoped();
			REQUIRE(c.template has_in_cache<service_a&>());
			c.template erase<service_a&>();
			REQUIRE(not c.template has_in_cache<service_a&>());
			REQUIRE(container.template has_in_cache<service_a&>());
			c.template provide<service_a&>();
			REQUIRE(c.template has_in_cache<service_a&>());
			REQUIRE(container.template has_in_cache<service_a&>());
			container.template erase<service_a&>();
			REQUIRE(c.template has_in_cache<service_a&>());
			REQUIRE(not container.template has_in_cache<service_a&>());
			
			if constexpr (std::same_as<alias_polymorphic_container, TestType>) {
				c.template replace<std::shared_ptr<shared_concrete>>(kangaru::shared_pointer_source<shared_concrete>{});
				REQUIRE(c.template has_in_cache<std::shared_ptr<shared_concrete>>());
				REQUIRE(c.template has_in_cache<std::shared_ptr<shared_abstract>>());
				REQUIRE(not container.template has_in_cache<std::shared_ptr<shared_concrete>>());
				REQUIRE(not container.template has_in_cache<std::shared_ptr<shared_abstract>>());
			}
			
			SECTION("Scope scope is similar") {
				auto cc = c.scoped();
				REQUIRE(cc.template has_in_cache<service_a&>());
				cc.template provide<service_b&>();
				REQUIRE(not c.template has_in_cache<service_b&>());
				REQUIRE(not container.template has_in_cache<service_b&>());
			}
		}
		
		CHECK(not container.template has_in_cache<service_c&>());
	}
	
	SECTION("Reuse instances that can override each other") {
		auto& a1 = container.template provide<service_a_child_1&>();
		auto& a = container.template provide<service_a&>();
		auto& c = container.template provide<service_c&>();
		
		if (std::same_as<alias_polymorphic_container, TestType>) {
			CHECK(std::addressof(c.services.sa) == std::addressof(a1));
		} else {
			// Does not override on non polymorphic container
			CHECK(std::addressof(c.services.sa) == std::addressof(a));
		}
	}
	
	SECTION("Reuses instances") {
		REQUIRE(not container.template has_in_cache<service_a&>());
		auto& a = container.template provide<service_a&>();
		auto& b = container.template provide<service_b&>();
		CHECK(std::addressof(a) == std::addressof(b.a));
		
		auto& c = container.template provide<service_c&>();
		
		CHECK(std::addressof(c.services.sa) == std::addressof(a));
	}
	
	SECTION("Can forget instances") {
		SECTION("Erase don't delete existing instances") {
			auto& a = container.template provide<service_a&>();
			a.i = 8;
			REQUIRE(a.i == 8);
			REQUIRE(container.template has_in_cache<service_a&>());
			container.template erase<service_a&>();
			REQUIRE(not container.template has_in_cache<service_a&>());
			
			SECTION("Access erased object is null") {
				auto a_ref = std::as_const(container).template get_from_cache<service_a&>();
				REQUIRE(not a_ref);
			}
			
			auto& a2 = container.template provide<service_a&>();
			CHECK(std::addressof(a) != std::addressof(a2));
			
			// If a is deleted, asan will trigger
			REQUIRE(a.i == 8);
		}
		
		SECTION("Erase don't erase overrides") {
			if constexpr (std::same_as<TestType, alias_polymorphic_container>) {
				auto& a1 = container.template provide<service_a_child_1&>();
				a1.i = 8;
				REQUIRE(container.template has_in_cache<service_a_child_1&>());
				REQUIRE(container.template has_in_cache<service_a&>());
				auto& a = container.template provide<service_a&>();
				CHECK(a.i == 8);
				CHECK(std::addressof(a1) == std::addressof(a));
				
				SECTION("Access polymorphic object") {
					auto a1_ref = std::as_const(container).template get_from_cache<service_a_child_1&>();
					REQUIRE(a1_ref);
					CHECK(&a1 == std::addressof(*a1_ref));
					kangaru::optional<service_a&> a_ref = std::as_const(container).template get_from_cache<service_a&>();
					REQUIRE(a_ref);
					CHECK(&(container.template provide<service_a&>()) == &*a_ref);
				}
				
				container.template erase<service_a_child_1&>();
				REQUIRE(not container.template has_in_cache<service_a_child_1&>());
				REQUIRE(container.template has_in_cache<service_a&>());
				
				auto& a_after = container.template provide<service_a&>();
				CHECK(std::addressof(a_after) == std::addressof(a));
				
				SECTION("Access polymorphic object") {
					auto a1_ref = std::as_const(container).template get_from_cache<service_a_child_1&>();
					REQUIRE(not a1_ref);
					auto a_ref = std::as_const(container).template get_from_cache<service_a&>();
					REQUIRE(a_ref);
					CHECK(std::addressof(a1) == std::addressof(*a_ref));
				}
			}
		}
	}
	
	SECTION("Works with injectors") {
		auto injector = kangaru::make_spread_injector(kangaru::ref(container));
		
		injector([](service_a& a, service_c& c) {
			CHECK(std::addressof(c.services.sa) == std::addressof(a));
			CHECK(std::addressof(c.services.sb.a) == std::addressof(a));
		});
	}
	
	SECTION("Simple caching of shared pointers") {
		SECTION("Can get directly from cache") {
			auto d_ref = std::as_const(container).template get_from_cache<std::shared_ptr<service_d>>();
			REQUIRE(not d_ref);
		}
		
		auto s = kangaru::provide<std::shared_ptr<service_d>>(container);
		CHECK(s == kangaru::provide<std::shared_ptr<service_d>>(container));
		CHECK(container.template has_in_cache<std::shared_ptr<service_d>>());
		
		SECTION("Can get directly from cache") {
			auto d_ref = std::as_const(container).template get_from_cache<std::shared_ptr<service_d>>();
			REQUIRE(d_ref);
			CHECK(*d_ref == s);
		}
	}
	
	SECTION("Allow injection using shared pointers") {
		auto container = TestType::make_container();
		
		SECTION("Simple caching of shared pointers") {
			auto s = kangaru::provide<std::shared_ptr<shared_abstract>>(container);
			CHECK(s == kangaru::provide<std::shared_ptr<shared_abstract>>(container));
			static_assert(not kangaru::source_of<decltype(container)&, std::shared_ptr<shared_abstract>&>);
			static_assert(not kangaru::source_of<decltype(container)&, std::shared_ptr<shared_abstract> const&>);
			static_assert(not kangaru::source_of<decltype(container)&, std::shared_ptr<shared_abstract>&&>);
			static_assert(not kangaru::source_of<decltype(container)&, std::shared_ptr<shared_abstract> const&&>);
			
			auto injector = kangaru::make_simple_injector(kangaru::ref(container));
			auto take_by_value = [](std::shared_ptr<shared_abstract>) {};
			auto take_by_ref = [](std::shared_ptr<shared_abstract>&) {};
			auto take_by_const_ref = [](std::shared_ptr<shared_abstract> const&) {};
			auto take_by_rref = [](std::shared_ptr<shared_abstract>&&) {};
			auto take_by_const_rref = [](std::shared_ptr<shared_abstract> const&&) {};
			
			static_assert(kangaru::callable<decltype(injector)&, decltype(take_by_value)>);
			static_assert(not kangaru::callable<decltype(injector)&, decltype(take_by_ref)>);
			static_assert(kangaru::callable<decltype(injector)&, decltype(take_by_const_ref)>);
			static_assert(kangaru::callable<decltype(injector)&, decltype(take_by_rref)>);
			static_assert(kangaru::callable<decltype(injector)&, decltype(take_by_const_rref)>);
		}
		
		SECTION("Allow shared pointers to override") {
			if constexpr (std::same_as<TestType, alias_polymorphic_container>) {
				kangaru::provide<std::shared_ptr<shared_concrete>>(container)->value = 16;
				CHECK(kangaru::provide<std::shared_ptr<shared_abstract>>(container)->value == 16);
			}
		}
	}
	
	SECTION("Supports second step for cached types") {
		auto& a = kangaru::provide<has_second_step&>(container);
		CHECK(a.a == 9);
		auto b = kangaru::provide<non_cached_with_second_step>(container);
		CHECK(b.a == 9);
	}
}

TEMPLATE_TEST_CASE("Container uses the base source", "[container]",
	(alias_container),
	(alias_polymorphic_container)
) {
	SECTION("Forwards") {
		auto container = TestType::make_container(int_source{});
		
		SECTION("lvalues") {
			auto& a = container.template provide<service_a&>();
			CHECK(a.i == 3);
		}
		
		SECTION("rvalues") {
			auto& a = std::move(container).template provide<service_a&>();
			CHECK(a.i == 4);
		}
		
		SECTION("lvalues deep") {
			auto& c = container.template provide<service_c&>();
			CHECK(c.services.sa.i == 3);
		}
		
		SECTION("rvalues deep") {
			auto& c = std::move(container).template provide<service_c&>();
			CHECK(c.services.sa.i == 4);
		}
		
		SECTION("lvalues direct") {
			auto i = container.template provide<int>();
			CHECK(i == 3);
		}
		
		SECTION("rvalues direct") {
			auto i = std::move(container).template provide<int>();
			CHECK(i == 4);
		}
		
		SECTION("lvalues deep direct") {
			auto w = container.template provide<int_wrapper>();
			CHECK(w.i == 3);
		}
		
		SECTION("rvalues deep direct") {
			auto w = std::move(container).template provide<int_wrapper>();
			CHECK(w.i == 4);
		}
	}
	
	SECTION("Supports provided services") {
		auto base = kangaru::make_container_base(kangaru::object_source{kangaru::reference_source{concrete{8.5f}}});
		auto container = TestType::make_container(base);
		auto& c = kangaru::provide<concrete&>(container);
		CHECK(c.value == 8.5f);
	}
	
	SECTION("container base source with factory functions") {
		auto base = kangaru::make_container_base(
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
		auto container = TestType::make_container(
			kangaru::make_container_base(kangaru::allow_assume_cached)
		);
		
		CHECK_THROWS_AS(kangaru::provide<std::shared_ptr<dynamic_provided_abstract>>(container), kangaru::not_found_exception);
		
		SECTION("Can provide the instance dynamically using replace") {
			auto provided = container.template replace<std::shared_ptr<dynamic_provided_concrete>>(
				kangaru::construct_in_place<kangaru::shared_pointer_source<dynamic_provided_concrete>>(15)
			);
			
			auto from_container = kangaru::provide<std::shared_ptr<dynamic_provided_concrete>>(container);
			CHECK(from_container == provided);
			CHECK(from_container->value == 15);
			
			auto unmapped = kangaru::provide<unmapped_dependent_on<std::shared_ptr<dynamic_provided_concrete>>>(container);
			CHECK(unmapped.value == from_container);
			
			if constexpr (std::same_as<alias_polymorphic_container, TestType>) {
				auto from_container = kangaru::provide<std::shared_ptr<dynamic_provided_abstract>>(container);
				CHECK(from_container == provided);
				
				auto mapped = kangaru::provide<mapped_dependent_on<std::shared_ptr<dynamic_provided_abstract>>>(container);
				CHECK(mapped.value == from_container);
				
				auto unmapped = kangaru::provide<unmapped_dependent_on<std::shared_ptr<dynamic_provided_abstract>>>(container);
				CHECK(unmapped.value == from_container);
			}
			
			static_assert(not kangaru::source_of<decltype(container)&, mapped_dependent_on<int>>);
			static_assert(not kangaru::source_of<decltype(container)&, unmapped_dependent_on<int>>);
			static_assert(not kangaru::source_of<decltype(container)&, float>);
		}
	}
	
	SECTION("container base source with dynamic supplied instances and factory functions") {
		auto base = kangaru::make_container_base(
			kangaru::allow_assume_cached,
			kangaru::throw_if_not_found{},
			kangaru::constructor_function<kangaru::reference_source<dependent_on_provided>>{},
			[]() -> kangaru::object_source<std::shared_ptr<dynamic_provided_abstract>> {
				return kangaru::object_source<std::shared_ptr<dynamic_provided_abstract>>{std::make_shared<dynamic_provided_concrete>(3)};
			}
		);
		
		auto container = TestType::make_container(base);
		auto provided = kangaru::provide<std::shared_ptr<dynamic_provided_abstract>>(container);
		
		SECTION ("Provide directly") {
			CHECK(provided->value == 3);
			CHECK(kangaru::provide<dependent_on_provided>(container).ptr == provided);
		}
		
		test_provide<std::shared_ptr<dynamic_provided_abstract>>(container, [&](auto value) {
			CHECK(value == provided);
		});
		
		static_assert(not kangaru::source_of<decltype(container)&, mapped_dependent_on<int>>);
		static_assert(not kangaru::source_of<decltype(container)&, unmapped_dependent_on<int>>);
		static_assert(not kangaru::source_of<decltype(container)&, float>);
	}
}

TEST_CASE("Container base source", "[container]") {
	auto self_contained_base = [](auto&& base) {
		return kangaru::with_recursion{
			kangaru::make_source_with_passthrough<1>(
				kangaru::make_source_with_provide_using_source<std::decay_t<decltype(base)>::template source_for>(
					kangaru::with_construction{
						kangaru::seal_source(kangaru::ref(base.source)),
						base.construction,
					}
				)
			),
		};
	};
	
	SECTION("Default assume cached") {
		auto base = kangaru::make_container_base(
			kangaru::allow_assume_cached
		);
		
		CHECK_THROWS_AS(kangaru::provide<kangaru::shared_pointer_source<dynamic_provided_abstract>>(base.source), kangaru::not_found_exception);
		
		static_assert(not kangaru::source_of<decltype(base.source), std::shared_ptr<dynamic_provided_abstract>>);
		static_assert(kangaru::source_of<decltype(base.source), kangaru::shared_pointer_source<dynamic_provided_abstract>>);
		static_assert(not kangaru::source_of<decltype(base.source), int>);
	}

	SECTION("Assume cached with specific not found source") {
		auto base = kangaru::make_container_base(
			kangaru::allow_assume_cached,
			kangaru::throw_if_not_found{}
		);
		
		CHECK_THROWS_AS(kangaru::provide<kangaru::shared_pointer_source<dynamic_provided_abstract>>(base.source), kangaru::not_found_exception);
		
		static_assert(not kangaru::source_of<decltype(base.source), std::shared_ptr<dynamic_provided_abstract>>);
		static_assert(kangaru::source_of<decltype(base.source), kangaru::shared_pointer_source<dynamic_provided_abstract>>);
		static_assert(not kangaru::source_of<decltype(base.source), int>);
	}

	SECTION("Assume cached with lambdas") {
		auto base = kangaru::make_container_base(
			kangaru::allow_assume_cached,
			kangaru::throw_if_not_found{},
			[](std::shared_ptr<dynamic_provided_abstract> ptr) {
				return kangaru::object_source<mapped_dependent_on<std::shared_ptr<dynamic_provided_abstract>>>{ptr};
			}
		);
		
		CHECK_THROWS_AS(
			kangaru::provide<kangaru::object_source<mapped_dependent_on<std::shared_ptr<dynamic_provided_abstract>>>>(
				self_contained_base(base)
			),
			kangaru::not_found_exception
		);
	}

	SECTION("Assume cached with lambdas") {
		auto base = kangaru::make_container_base(
			[]() -> std::shared_ptr<dynamic_provided_abstract> {
				return std::make_shared<dynamic_provided_concrete>(32);
			},
			[](std::shared_ptr<dynamic_provided_abstract> ptr) {
				return kangaru::object_source<mapped_dependent_on<std::shared_ptr<dynamic_provided_abstract>>>{ptr};
			}
		);
		
		CHECK(
			kangaru::provide<mapped_dependent_on<std::shared_ptr<dynamic_provided_abstract>>>(
				kangaru::provide<kangaru::object_source<mapped_dependent_on<std::shared_ptr<dynamic_provided_abstract>>>>(
					self_contained_base(base)
				)
			).value->value == 32
		);
	}

	SECTION("Assume cached with lambdas and a source") {
		auto base = kangaru::make_container_base(
			kangaru::none_source{},
			[]() -> std::shared_ptr<dynamic_provided_abstract> {
				return std::make_shared<dynamic_provided_concrete>(32);
			},
			[](std::shared_ptr<dynamic_provided_abstract> ptr) {
				return kangaru::object_source<mapped_dependent_on<std::shared_ptr<dynamic_provided_abstract>>>{ptr};
			}
		);
		
		CHECK(
			kangaru::provide<mapped_dependent_on<std::shared_ptr<dynamic_provided_abstract>>>(
				kangaru::provide<kangaru::object_source<mapped_dependent_on<std::shared_ptr<dynamic_provided_abstract>>>>(
					self_contained_base(base)
				)
			).value->value == 32
		);
	}

	SECTION("Custom injector") {
		auto base1 = kangaru::make_container_base(
			kangaru::make_strict_spread_injector_function{},
			kangaru::none_source{},
			[] {
				return 1;
			},
			[](int&&) {
				return std::string{};
			}
		);
		
		auto base2 = kangaru::make_container_base(
			kangaru::make_strict_spread_injector_function{},
			kangaru::none_source{},
			[] {
				return 1;
			},
			[](int) {
				return std::string{};
			}
		);
		
		auto base3 = kangaru::make_container_base(
			kangaru::make_spread_injector_function{},
			kangaru::none_source{},
			[] {
				return 1;
			},
			[](int&&) {
				return std::string{};
			}
		);
		
		auto base4 = kangaru::make_container_base(
			kangaru::allow_assume_cached,
			kangaru::make_strict_spread_injector_function{},
			kangaru::throw_if_not_found{},
			kangaru::none_source{},
			[](std::shared_ptr<dynamic_provided_abstract>) {
				return 1;
			},
			[](int) {
				return std::string{};
			}
		);
		
		auto base5 = kangaru::make_container_base(
			kangaru::allow_assume_cached,
			kangaru::make_strict_spread_injector_function{},
			kangaru::throw_if_not_found{},
			kangaru::none_source{},
			[](std::shared_ptr<dynamic_provided_abstract> const&) {
				return 1;
			}
		);
		
		static_assert(kangaru::source_of<decltype(self_contained_base(base1)), int>);
		static_assert(not kangaru::source_of<decltype(self_contained_base(base1)), std::string>);
		static_assert(kangaru::source_of<decltype(self_contained_base(base2)), int>);
		static_assert(kangaru::source_of<decltype(self_contained_base(base2)), std::string>);
		static_assert(kangaru::source_of<decltype(self_contained_base(base3)), int>);
		static_assert(kangaru::source_of<decltype(self_contained_base(base3)), std::string>);
		static_assert(kangaru::source_of<decltype(self_contained_base(base4)), int>);
		static_assert(kangaru::source_of<decltype(self_contained_base(base4)), std::string>);
		static_assert(not kangaru::source_of<decltype(self_contained_base(base5)), int>);
	}

	SECTION("Default assume abort") {
		auto base = kangaru::make_container_base(
			kangaru::allow_assume_cached,
			kangaru::abort_if_not_found{}
		);
		
		static_assert(not kangaru::source_of<decltype(base.source), std::shared_ptr<dynamic_provided_abstract>>);
		static_assert(kangaru::source_of<decltype(base.source), kangaru::shared_pointer_source<dynamic_provided_abstract>>);
		static_assert(not kangaru::source_of<decltype(base.source), int>);
	}

	SECTION("Default assume terminate") {
		auto base = kangaru::make_container_base(
			kangaru::allow_assume_cached,
			kangaru::terminate_if_not_found{}
		);
		
		static_assert(not kangaru::source_of<decltype(base.source), std::shared_ptr<dynamic_provided_abstract>>);
		static_assert(kangaru::source_of<decltype(base.source), kangaru::shared_pointer_source<dynamic_provided_abstract>>);
		static_assert(not kangaru::source_of<decltype(base.source), int>);
	}
}
