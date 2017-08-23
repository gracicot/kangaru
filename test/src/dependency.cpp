#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "kangaru/kangaru.hpp"

TEST_CASE("Container injects arguments", "[dependency]") {
	SECTION("Injected singles are the same returned by the container") {
		struct Service1 {};
		
		struct Service2 {
			Service2() = default;
			Service2(Service1& s) : s1{&s} {}
			
			Service1* s1 = nullptr;
		};
		
		struct Definition1 : kgr::SingleService<Service1> {};
		struct Definition2 : kgr::Service<Service2, kgr::Dependency<Definition1>> {};
		
		kgr::Container c;
		
		REQUIRE(c.service<Definition2>().s1 == &c.service<Definition1>());
	}
	
	SECTION("injected arguments are sent correctly to the constructor") {
		struct Service1 {};
		
		struct Service2 {
			Service2() = default;
			Service2(Service1& s) : s1{&s} {}
			
			Service1* s1 = nullptr;
		};
		
		struct Definition1 : kgr::SingleService<Service1> {};
		
		struct Definition2 {
			Definition2(kgr::in_place_t) {}
			Definition2(kgr::in_place_t, Service1& dep) : service{dep} {}
			
			Service2 forward() {
				return std::move(service);
			}
			
			static auto construct(kgr::Inject<Definition1> d1) -> decltype(kgr::inject(d1.forward())) {
				return kgr::inject(d1.forward());
			}
			
			Service2 service;
		};
		
		kgr::Container c;
		
		REQUIRE(c.service<Definition2>().s1 == &c.service<Definition1>());
	}
	
	SECTION("injected arguments can be single and non single") {
		static bool constructor_called = false;
		
		struct Service1 {};
		struct Service2 {};
		
		struct Service3 {
			Service3() = default;
			
			Service3(Service1& s, Service2) : s1{&s} {
				constructor_called = true;
			}
			
			Service1* s1 = nullptr;
		};
		
		struct Definition1 : kgr::SingleService<Service1> {};
		struct Definition2 : kgr::Service<Service2> {};
		
		struct Definition3 {
			Definition3(kgr::in_place_t, Service1& dep1, Service2 dep2) : service{dep1, std::move(dep2)} {}
			
			Service3 forward() {
				return std::move(service);
			}
			
			static auto construct(kgr::Inject<Definition1> d1, kgr::Inject<Definition2> d2) -> decltype(kgr::inject(d1.forward(), d2.forward())) {
				return kgr::inject(d1.forward(), d2.forward());
			}
			
			Service3 service;
		};
		
		(void) kgr::Container{}.service<Definition3>();
		
		REQUIRE(constructor_called);
	}
}

TEST_CASE("Container injects arguments recursively", "[dependency]") {
	SECTION("Injected service can have dependencies") {
		static bool service1_constructed = false;
		static bool service2_constructed = false;
		static bool service3_constructed = false;
		
		struct Service1 {
			Service1() { service1_constructed = true; }
		};
		
		struct Service2 {
			Service2() = default;
			Service2(Service1) { service2_constructed = true; }
		};
		
		struct Service3 {
			Service3() = default;
			Service3(Service2) { service3_constructed = true; }
		};
		
		struct Definition1 : kgr::Service<Service1> {};
		struct Definition2 : kgr::Service<Service2, kgr::Dependency<Definition1>> {};
		struct Definition3 : kgr::Service<Service3, kgr::Dependency<Definition2>> {};
		
		(void) kgr::Container{}.service<Definition3>();
		
		CHECK(service1_constructed);
		CHECK(service2_constructed);
		REQUIRE(service3_constructed);
	}
	
	SECTION("Injected service can have multiple dependencies") {
		static bool service1_constructed = false;
		static bool service2_constructed = false;
		static bool service3_constructed = false;
		static bool service4_constructed = false;
		
		struct Service1 {
			Service1() { service1_constructed = true; }
		};
		
		struct Service2 {
			Service2() { service2_constructed = true; }
		};
		
		struct Service3 {
			Service3() = default;
			Service3(Service1, Service2) { service3_constructed = true; }
		};
		
		struct Service4 {
			Service4() = default;
			Service4(Service3) { service4_constructed = true; }
		};
		
		struct Definition1 : kgr::Service<Service1> {};
		struct Definition2 : kgr::Service<Service2> {};
		struct Definition3 : kgr::Service<Service3, kgr::Dependency<Definition1, Definition2>> {};
		struct Definition4 : kgr::Service<Service4, kgr::Dependency<Definition3>> {};
		
		(void) kgr::Container{}.service<Definition4>();
		
		CHECK(service1_constructed);
		CHECK(service2_constructed);
		CHECK(service3_constructed);
		REQUIRE(service4_constructed);
	}
}
