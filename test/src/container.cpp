#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "kangaru/kangaru.hpp"

TEST_CASE("emplace add a service to the container", "[container]") {
	struct Service {};
	struct Definition : kgr::single_service<Service> {};
	
	kgr::container c;
	REQUIRE(c.emplace<Definition>());
	
	SECTION("The service is added") {
		REQUIRE(c.contains<Definition>());
	}
	
	SECTION("The service cannot be added again") {
		auto service = &c.service<Definition>();
		
		REQUIRE_FALSE(c.emplace<Definition>());
		REQUIRE(service == &c.service<Definition>());
	}
}

TEST_CASE("replace add service to the container even if already constructed", "[container]") {
	static bool destructor_called;
	static int constructed_count;
	
	constructed_count = 0;
	destructor_called = false;
	
	struct Service {
		Service() {
			constructed_count++;
		}
		
		~Service() {
			destructor_called = true;
		}
	};
	
	struct Definition : kgr::single_service<Service> {};
	
	kgr::container c;
	
	REQUIRE(c.emplace<Definition>());
	REQUIRE(c.contains<Definition>());
	REQUIRE(constructed_count == 1);
	REQUIRE_FALSE(destructor_called);
	
	auto service = &c.service<Definition>();
	
	c.replace<Definition>();
	
	REQUIRE(c.contains<Definition>());
	REQUIRE(constructed_count == 2);
	REQUIRE_FALSE(destructor_called);
	REQUIRE(service != &c.service<Definition>());
}

TEST_CASE("Container can fork", "[container]") {
	struct Service {};
	struct Definition1 : kgr::single_service<Service> {};
	struct Definition2 : kgr::single_service<Service> {};
	
	kgr::container c1;
	c1.emplace<Definition1>();
	
	SECTION("A fork observe the original container") {
		auto c2 = c1.fork();
		
		REQUIRE(c1.contains<Definition1>());
		REQUIRE(c2.contains<Definition1>());
	}
	
	SECTION("A fork don't observe services added after the fork") {
		auto c2 = c1.fork();
		
		c1.emplace<Definition2>();
		
		REQUIRE(c1.contains<Definition2>());
		REQUIRE_FALSE(c2.contains<Definition2>());
	}
}

TEST_CASE("Container can rebase", "[container]") {
	struct Service {};
	struct Definition : kgr::single_service<Service> {};
	
	kgr::container c1;
	auto c2 = c1.fork();
	
	REQUIRE_FALSE(c1.contains<Definition>());
	REQUIRE_FALSE(c2.contains<Definition>());
	
	SECTION("Rebase add service added after the fork") {
		c1.emplace<Definition>();
		
		REQUIRE(c1.contains<Definition>());
		REQUIRE_FALSE(c2.contains<Definition>());
		
		c2.rebase(c1);
		
		REQUIRE(c1.contains<Definition>());
		REQUIRE(c2.contains<Definition>());
	}
	
	SECTION("Rebase from a unrelated container") {
		kgr::container c3;
		c1.emplace<Definition>();
		
		REQUIRE(c1.contains<Definition>());
		REQUIRE_FALSE(c3.contains<Definition>());
		
		c3.rebase(c1);
		
		REQUIRE(c1.contains<Definition>());
		REQUIRE(c3.contains<Definition>());
	}
	
	SECTION("Rebase keep services in case of conflicts") {
		c1.emplace<Definition>();
		c2.emplace<Definition>();
		
		REQUIRE(c1.contains<Definition>());
		REQUIRE(c2.contains<Definition>());
		
		auto service1 = &c1.service<Definition>();
		auto service2 = &c2.service<Definition>();
		
		REQUIRE(service1 != service2);
		
		c2.rebase(c1);
		
		REQUIRE(c1.contains<Definition>());
		REQUIRE(c2.contains<Definition>());
		
		auto service1after = &c1.service<Definition>();
		auto service2after = &c2.service<Definition>();
		
		REQUIRE(service1after == service1);
		REQUIRE(service1after != service2after);
		REQUIRE(service2after == service2);
	}
}

TEST_CASE("Container can merge", "[container]") {
	struct Service {};
	struct Definition : kgr::single_service<Service> {};
	
	kgr::container c1;
	kgr::container c2;
	
	SECTION("Merge will bring all service to the target") {
		c2.emplace<Definition>();
		
		REQUIRE_FALSE(c1.contains<Definition>());
		REQUIRE(c2.contains<Definition>());
		
		c1.merge(std::move(c2));
		
		REQUIRE(c1.contains<Definition>());
		REQUIRE(c2.contains<Definition>());
	}
	
	SECTION("Merge keep services in case of conflicts") {
		c1.emplace<Definition>();
		c2.emplace<Definition>();
		
		REQUIRE(c1.contains<Definition>());
		REQUIRE(c2.contains<Definition>());
		
		auto service1 = &c1.service<Definition>();
		auto service2 = &c2.service<Definition>();
		
		REQUIRE(service1 != service2);
		
		c1.merge(std::move(c2));
		
		REQUIRE(c1.contains<Definition>());
		REQUIRE(c2.contains<Definition>());
		
		auto service1after = &c1.service<Definition>();
		auto service2after = &c2.service<Definition>();
		
		REQUIRE(service1after == service1);
		REQUIRE(service1after != service2after);
		REQUIRE(service2after == service2);
	}
}
