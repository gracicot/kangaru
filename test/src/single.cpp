#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "kangaru/kangaru.hpp"

TEST_CASE("Container creates a single", "[single]") {
	SECTION("Construct the single one time") {
		kgr::Container c;
		
		static int constructed = 0;
		
		struct Service {
			Service() { constructed++; }
		};
		
		struct Definition : kgr::SingleService<Service> {};
		
		(void) c.service<Definition>();
		
		REQUIRE(constructed == 1);
	}
	
	SECTION("Construct the single one time with multiple invocation") {
		kgr::Container c;
		
		static int constructed = 0;
		
		struct Service {
			Service() { constructed++; }
		};
		
		struct Definition : kgr::SingleService<Service> {};
		
		(void) c.service<Definition>();
		(void) c.service<Definition>();
		
		REQUIRE(constructed == 1);
	}
	
	SECTION("Returns the same instance") {
		kgr::Container c;
		
		struct Service {};
		struct Definition : kgr::SingleService<Service> {};
		
		REQUIRE(&c.service<Definition>() == &c.service<Definition>());
	}
	
	SECTION("Don't move or copy") {
		kgr::Container c;
		
		static bool constructed = false;
		static bool displaced = false;
		
		struct Service {
			Service() {
				constructed = true;
			}
			
			Service(const Service&) {
				displaced = true;
			}
			
			Service(Service&&) {
				displaced = true;
			}
			
			Service& operator=(const Service&) {
				displaced = true;
				
				return *this;
			}
			
			Service& operator=(Service&&) {
				displaced = true;
				 
				return *this;
			}
		};
		
		struct Definition : kgr::SingleService<Service> {};
		
		(void) c.service<Definition>();
		
		REQUIRE(constructed);
		REQUIRE_FALSE(displaced);
	}
	
	SECTION("Are deleted when the container dies") {
		static bool deleted = false;
		
		struct Service { ~Service() { deleted = true; } };
		struct Definition : kgr::SingleService<Service> {};
		
		{
			kgr::Container c;
			
			(void) c.service<Definition>();
			
			REQUIRE_FALSE(deleted);
		}
		
		REQUIRE(deleted);
	}
}

TEST_CASE("Singles are defined", "[single]") {
	SECTION("By being abstract") {
		struct Definition : kgr::Abstract {};
		
		REQUIRE(kgr::detail::is_single<Definition>{});
	}
	
	SECTION("By extending single") {
		struct Definition : kgr::Single {};
		
		REQUIRE(kgr::detail::is_single<Definition>{});
	}
}

TEST_CASE("Singles are not copiable", "[single]") {
	SECTION("By being abstract") {
		struct Definition : kgr::Abstract {};
		
		REQUIRE_FALSE(std::is_copy_constructible<Definition>{});
	}
	
	SECTION("By extending single") {
		struct Definition : kgr::Single {};
		
		REQUIRE_FALSE(std::is_copy_constructible<Definition>{});
	}
}
