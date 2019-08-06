#include <catch2/catch.hpp>
#include <kangaru/kangaru.hpp>

TEST_CASE("The definition may not contain virtual members", "[definition]") {
	struct Service {};
	struct Definition : kgr::service<Service> {
		virtual ~Definition() {}
	};
	
	REQUIRE_FALSE(kgr::detail::is_service<Definition>::value);
}

TEST_CASE("The definition may not forward void", "[definition]") {
	struct Definition : kgr::abstract {
		void forward() {}
	};
	
	REQUIRE_FALSE(kgr::detail::is_service<Definition>::value);
}

TEST_CASE("The definition can be construct using emplace", "[definition]") {
	struct Service {};
	struct Definition {
		static auto construct() -> kgr::inject_result<> { return {}; }
		void emplace() {}
		Service forward() { return {}; }
	};
	
	REQUIRE(kgr::detail::is_service_valid<Definition>::value);
	
	(void) kgr::container{}.service<Definition>();
}

TEST_CASE("The definition can have arguments including other services", "[definition]") {
	struct Service1 {};
	struct Service2 {};
	struct Definition1 {
		static auto construct() -> kgr::inject_result<> { return kgr::inject(); }
		void emplace() {}
		Service1 forward() { return {}; }
	};
	struct Definition2 {
		static auto construct(kgr::inject_t<Definition1> d1, int) -> kgr::inject_result<Service1> { return kgr::inject(d1.forward()); }
		void emplace(Service1) {}
		Service2 forward() { return {}; }
	};
	
	REQUIRE(kgr::detail::is_service_valid<Definition2, int>::value);
	(void) kgr::container{}.service<Definition2>(123);
}

TEST_CASE("The definition cannot be construct with emplace differing construct", "[definition]") {
	struct Service {};
	struct Definition {
		static auto construct() -> decltype(kgr::inject(int{})) { return kgr::inject(int{}); }
		void emplace() {}
		Service forward() { return {}; }
	};
	
	REQUIRE_FALSE(kgr::detail::is_service_valid<Definition>::value);
}

TEST_CASE("The definition cannot be only be construct with emplace if parameters are the same", "[definition]") {
	struct Service {};
	struct Definition {
		static auto construct(double) -> decltype(kgr::inject(int{}, char{})) { return kgr::inject(int{}, char{}); }
		void emplace(int, char) {}
		Service forward() { return {}; }
	};
	
	REQUIRE_FALSE(kgr::detail::is_service_valid<Definition>::value);
	REQUIRE((kgr::detail::is_service_valid<Definition, double>::value));
	
	(void) kgr::container{}.service<Definition>(0.0);
}

namespace template_construct {

struct Service {};
struct Definition {
	template<typename T>
	static auto construct(T&&) -> decltype(kgr::inject()) { return kgr::inject(); }
	void emplace() {}
	Service forward() { return {}; }
};

TEST_CASE("The construct function can be template", "[definition]") {
	REQUIRE_FALSE(kgr::detail::is_service_valid<Definition>::value);
	REQUIRE((kgr::detail::is_service_valid<Definition, double>::value));
	REQUIRE((kgr::detail::is_service_valid<Definition, float>::value));
	REQUIRE((kgr::detail::is_service_valid<Definition, int>::value));
	REQUIRE((kgr::detail::is_service_valid<Definition, std::tuple<>>::value));
	REQUIRE_FALSE((kgr::detail::is_service_valid<Definition, int, double>::value));
	
	kgr::container c;
	(void) c.service<Definition>(0.0);
	(void) c.service<Definition>(0.0f);
	(void) c.service<Definition>(0);
	(void) c.service<Definition>(std::tuple<>{});
}
} // namespace template_construct

namespace template_construct_param {

struct Service {};
struct Definition {
	template<typename T>
	static auto construct(int, T&&) -> decltype(kgr::inject()) { return kgr::inject(); }
	void emplace() {}
	Service forward() { return {}; }
};

TEST_CASE("The construct function can be a mix of non templated and template parameters", "[definition]") {
	REQUIRE_FALSE(kgr::detail::is_service_valid<Definition>::value);
	REQUIRE_FALSE((kgr::detail::is_service_valid<Definition, double>::value));
	REQUIRE_FALSE((kgr::detail::is_service_valid<Definition, float>::value));
	REQUIRE_FALSE((kgr::detail::is_service_valid<Definition, int>::value));
	REQUIRE_FALSE((kgr::detail::is_service_valid<Definition, std::tuple<>>::value));
	REQUIRE((kgr::detail::is_service_valid<Definition, int, double>::value));
	REQUIRE((kgr::detail::is_service_valid<Definition, int, float>::value));
	REQUIRE((kgr::detail::is_service_valid<Definition, int, int>::value));
	REQUIRE((kgr::detail::is_service_valid<Definition, int, std::tuple<>>::value));
	
	kgr::container c;
	(void) c.service<Definition>(0, 0.0);
	(void) c.service<Definition>(0, 0.0f);
	(void) c.service<Definition>(0, 0);
	(void) c.service<Definition>(0, std::tuple<>{});
}
} // namespace template_construct_param

namespace template_construct_inject {

struct Service {};
struct Service2 {};
struct Definition2 : kgr::service<Service2> {};

struct DefinitionA {
	template<typename T>
	static auto construct(kgr::inject_t<Definition2>, int, T&&) -> decltype(kgr::inject()) { return kgr::inject(); }
	void emplace() {}
	Service forward() { return {}; }
};

struct DefinitionB {
	template<typename... T>
	static auto construct(kgr::inject_t<Definition2>, int, T&&...) -> decltype(kgr::inject()) { return kgr::inject(); }
	void emplace() {}
	Service forward() { return {}; }
};

struct DefinitionC {
	template<typename... T>
	static auto construct(kgr::inject_t<Definition2>, DefinitionB, int, T&&...) -> decltype(kgr::inject()) { return kgr::inject(); }
	void emplace() {}
	Service forward() { return {}; }
};

TEST_CASE("The construct function can be a mix of non templated and template parameters and injected services", "[definition]") {
	REQUIRE((kgr::detail::is_service_valid<DefinitionA, int, double>::value));
	REQUIRE((kgr::detail::is_service_valid<DefinitionA, int, float>::value));
	REQUIRE((kgr::detail::is_service_valid<DefinitionA, int, int>::value));
	REQUIRE((kgr::detail::is_service_valid<DefinitionA, int, std::tuple<>>::value));
	REQUIRE_FALSE((kgr::detail::is_service_valid<DefinitionA, int, double, std::tuple<>>::value));
	REQUIRE_FALSE((kgr::detail::is_service_valid<DefinitionA, int, float, std::tuple<>>::value));
	REQUIRE_FALSE((kgr::detail::is_service_valid<DefinitionA, int, int, std::tuple<>>::value));
	REQUIRE_FALSE((kgr::detail::is_service_valid<DefinitionA, int, std::tuple<>, std::tuple<>>::value));
	
	REQUIRE((kgr::detail::is_service_valid<DefinitionB, int, double, float>::value));
	REQUIRE((kgr::detail::is_service_valid<DefinitionB, int, float, float>::value));
	REQUIRE((kgr::detail::is_service_valid<DefinitionB, int, int, float>::value));
	REQUIRE((kgr::detail::is_service_valid<DefinitionB, int, std::tuple<>, float>::value));
	REQUIRE((kgr::detail::is_service_valid<DefinitionB, int, double, std::tuple<>>::value));
	REQUIRE((kgr::detail::is_service_valid<DefinitionB, int, float, std::tuple<>>::value));
	REQUIRE((kgr::detail::is_service_valid<DefinitionB, int, int, std::tuple<>>::value));
	REQUIRE((kgr::detail::is_service_valid<DefinitionB, int, std::tuple<>, std::tuple<>>::value));
	
	REQUIRE((kgr::detail::is_service_valid<DefinitionC, DefinitionB, int, double, float>::value));
	REQUIRE((kgr::detail::is_service_valid<DefinitionC, DefinitionB, int, float, float>::value));
	REQUIRE((kgr::detail::is_service_valid<DefinitionC, DefinitionB, int, int, float>::value));
	REQUIRE((kgr::detail::is_service_valid<DefinitionC, DefinitionB, int, std::tuple<>, float>::value));
	
	kgr::container c;
	(void) c.service<DefinitionA>(0, 0.0);
	(void) c.service<DefinitionA>(0, 0.0f);
	(void) c.service<DefinitionA>(0, 0);
	(void) c.service<DefinitionA>(0, std::tuple<>{});
	
	(void) c.service<DefinitionB>(0, 0.0, 0.0f);
	(void) c.service<DefinitionB>(0, 0.0f, 0.0f);
	(void) c.service<DefinitionB>(0, 0, 0.0f);
	(void) c.service<DefinitionB>(0, std::tuple<>{}, 0.0f);
	(void) c.service<DefinitionB>(0, 0.0, std::tuple<>{});
	(void) c.service<DefinitionB>(0, 0.0f, std::tuple<>{});
	(void) c.service<DefinitionB>(0, 0, std::tuple<>{});
	(void) c.service<DefinitionB>(0, std::tuple<>{}, std::tuple<>{});
	
	(void) c.service<DefinitionC>(DefinitionB{}, 0, 0.0);
	(void) c.service<DefinitionC>(DefinitionB{}, 0, 0.0f);
	(void) c.service<DefinitionC>(DefinitionB{}, 0, 0);
	(void) c.service<DefinitionC>(DefinitionB{}, 0, std::tuple<>{});
}
} // namespace template_construct_inject

namespace in_place_constructor {

bool constructor_called = false;

struct Service {};
struct Definition {
	explicit Definition(kgr::in_place_t) { constructor_called = true; }
	static auto construct() -> decltype(kgr::inject()) { return kgr::inject(); }
	Service forward() { return {}; }
};

TEST_CASE("The container call the constructor with the in_place_t constructor", "[definition]") {
	(void) kgr::container{}.service<Definition>();
	REQUIRE(constructor_called);
}
} // namespace in_place_constructor

namespace no_in_place {

bool constructor_called = false;
bool emplace_called = false;

struct Service {};
struct Definition {
	Definition() { constructor_called = true; }
	static auto construct() -> decltype(kgr::inject()) { return kgr::inject(); }
	void emplace() { emplace_called = true; }
	Service forward() { return {}; }
};

TEST_CASE("The container call the default if no constructor with the in_place_t constructor is defined", "[definition]") {
	(void) kgr::container{}.service<Definition>();
	REQUIRE(emplace_called);
	REQUIRE(constructor_called);
}
} // namespace no_in_place

namespace in_place_recieves_injected_parameters {

bool constructor_called = false;

struct Service {};
struct Definition {
	Definition(kgr::in_place_t, int&&, double&) { constructor_called = true; }
	static auto construct(double& a) -> decltype(kgr::inject(int{}, a)) { return kgr::inject(1, a); }
	Service forward() { return {}; }
};

TEST_CASE("The forward injected parameters to the Definition constructor", "[definition]") {
	REQUIRE((kgr::detail::is_service_valid<Definition, double&>::value));
	double a = 0;
	(void) kgr::container{}.service<Definition>(a);
	REQUIRE(constructor_called);
}
} // namespace in_place_recieves_injected_parameters

namespace definition_constructor_priority {

bool constructor_called = false;
bool emplace_called = false;

struct Service {};
struct Definition {
	Definition(kgr::in_place_t, int, double&) { constructor_called = true; }
	
	template<typename T>
	static auto construct(int a, T& b) -> decltype(kgr::inject(std::move(a), b)) { return kgr::inject(std::move(a), b); }
	
	template<typename... Args>
	void emplace(Args&&...) { emplace_called = true; }
	Service forward() { return {}; }
};

TEST_CASE("The container prefer the in_place_t constructor over emplace function", "[definition]") {
	double d = 0;
	(void) kgr::container{}.service<Definition>(0, d);
	
	REQUIRE(constructor_called);
	REQUIRE_FALSE(emplace_called);
}
} // namespace definition_constructor_priority

TEST_CASE("Supplied services has different constraints", "[definition]") {
	struct Service { explicit Service(int) {} };
	struct Definition : kgr::single_service<Service>, kgr::supplied {};
	
	SECTION("Are still valid even if not constructible") {
		REQUIRE(kgr::detail::is_service_valid<Definition>::value);
	}
	
	SECTION("Need arguments when emplaced") {
		REQUIRE_FALSE(kgr::detail::is_emplace_valid<Definition>::value);
		REQUIRE(kgr::detail::is_emplace_valid<Definition, int>::value);
	}
}
