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
}

namespace template_construct {
struct Service {};
struct Definition {
	template<typename T>
	static auto construct(T) -> decltype(kgr::inject()) { return kgr::inject(); }
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
}
}
