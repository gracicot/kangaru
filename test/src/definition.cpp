#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "kangaru/kangaru.hpp"

TEST_CASE("The definition may not contain virtual members", "[definition]") {
	struct Service {};
	struct Definition : kgr::Service<Service> {
		virtual ~Definition();
	};
	
	REQUIRE_FALSE(kgr::detail::is_service<Definition>::value);
}

TEST_CASE("The definition may not forward void", "[definition]") {
	struct Definition : kgr::Abstract {
		void forward();
	};
	
	REQUIRE_FALSE(kgr::detail::is_service<Definition>::value);
}

TEST_CASE("The definition can be construct using emplace", "[definition]") {
	struct Service {};
	struct Definition {
		static std::tuple<> construct() { return {}; }
		void emplace() {}
		Service forward() { return {}; }
	};
	
	REQUIRE(kgr::detail::is_service_valid<Definition>::value);
	
	(void) kgr::Container{}.service<Definition>();
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
	
	(void) kgr::Container{}.service<Definition>(0.0);
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
	
	kgr::Container c;
	(void) c.service<Definition>(0.0);
	(void) c.service<Definition>(0.0f);
	(void) c.service<Definition>(0);
	(void) c.service<Definition>(std::tuple<>{});
}
}

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
	
	kgr::Container c;
	(void) c.service<Definition>(0, 0.0);
	(void) c.service<Definition>(0, 0.0f);
	(void) c.service<Definition>(0, 0);
	(void) c.service<Definition>(0, std::tuple<>{});
}
}

namespace template_construct_inject {
struct Service {};
struct Service2 {};
struct Definition2 : kgr::Service<Service2> {};

struct DefinitionA {
	template<typename T>
	static auto construct(kgr::Inject<Definition2>, int, T&&) -> decltype(kgr::inject()) { return kgr::inject(); }
	void emplace() {}
	Service forward() { return {}; }
};

struct DefinitionB {
	template<typename... T>
	static auto construct(kgr::Inject<Definition2>, int, T&&...) -> decltype(kgr::inject()) { return kgr::inject(); }
	void emplace() {}
	Service forward() { return {}; }
};

struct DefinitionC {
	template<typename... T>
	static auto construct(kgr::Inject<Definition2>, DefinitionB, int, T&&...) -> decltype(kgr::inject()) { return kgr::inject(); }
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
	
	kgr::Container c;
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
}
