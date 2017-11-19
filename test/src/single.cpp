#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "kangaru/kangaru.hpp"

TEST_CASE("Container creates a single", "[single]") {
	kgr::container c;
	
	static int constructed;
	constructed = 0;
	
	struct Service {
		Service() { constructed++; }
	};
	
	struct Definition : kgr::single_service<Service> {};
		
	SECTION("Construct the single one time") {
		(void) c.service<Definition>();
		
		REQUIRE(constructed == 1);
	}
	
	SECTION("Construct the single one time with multiple invocation") {
		(void) c.service<Definition>();
		(void) c.service<Definition>();
		
		REQUIRE(constructed == 1);
	}
	
	SECTION("Returns the same instance") {
		REQUIRE(&c.service<Definition>() == &c.service<Definition>());
	}
}

TEST_CASE("Container contains a single after contruction", "[single]") {
	kgr::container c;
	
	struct Service {};
	struct Definition : kgr::single_service<Service> {};
		
	(void) c.service<Definition>();
	
	REQUIRE(c.contains<Definition>());
}

TEST_CASE("Singles are never moved or copied", "[single]") {
	kgr::container c;
		
	static bool constructed = false;
	static bool displaced = false;
	
	struct Service {
		Service() noexcept {
			constructed = true;
		}
		
		Service(const Service&) noexcept {
			displaced = true;
		}
		
		Service(Service&&) noexcept {
			displaced = true;
		}
		
		Service& operator=(const Service&) noexcept {
			displaced = true;
			
			return *this;
		}
		
		Service& operator=(Service&&) noexcept {
			displaced = true;
				
			return *this;
		}
	};
	
	struct Definition : kgr::single_service<Service> {};
	
	SECTION("When created") {
		(void) c.service<Definition>();
		
		REQUIRE(constructed);
		REQUIRE_FALSE(displaced);
	}
	
	SECTION("When the container is moved") {
		(void) c.service<Definition>();
		
		auto c2 = std::move(c);
		
		REQUIRE(constructed);
		REQUIRE_FALSE(displaced);
	}
}

TEST_CASE("Singles are defined using tags", "[single]") {
	SECTION("By being abstract") {
		struct Definition : kgr::abstract {};
		
		REQUIRE(kgr::detail::is_single<Definition>{});
	}
	
	SECTION("By extending single") {
		struct Definition : kgr::single {};
		
		REQUIRE(kgr::detail::is_single<Definition>{});
	}
}

TEST_CASE("Singles are deleted when the container dies", "[single]") {
	static bool deleted = false;
	
	struct Service { ~Service() { deleted = true; } };
	struct Definition : kgr::single_service<Service> {};
	
	{
		kgr::container c;
		
		(void) c.service<Definition>();
		
		REQUIRE_FALSE(deleted);
	}
	
	REQUIRE(deleted);
}

TEST_CASE("Singles are not copiable", "[single]") {
	SECTION("By being abstract") {
		struct Definition : kgr::abstract {};
		
		REQUIRE_FALSE(std::is_copy_constructible<Definition>{});
	}
	
	SECTION("By extending single") {
		struct Definition : kgr::single {};
		
		REQUIRE_FALSE(std::is_copy_constructible<Definition>{});
	}
}
