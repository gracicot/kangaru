#include <catch2/catch.hpp>
#include <kangaru-prev/kangaru.hpp>

TEST_CASE("Injected singles are the same returned by the container", "[dependency]") {
	struct Service1 {};
	
	struct Service2 {
		Service2() = default;
		explicit Service2(Service1& s) : s1{&s} {}
		
		Service1* s1 = nullptr;
	};
	
	struct Definition1 : kgr::single_service<Service1> {};
	struct Definition2 : kgr::service<Service2, kgr::dependency<Definition1>> {};
	
	kgr::container c;
	
	REQUIRE(c.service<Definition2>().s1 == &c.service<Definition1>());
}

TEST_CASE("Injected arguments are sent correctly to the constructor", "[dependency]") {
	struct Service1 {};
	
	struct Service2 {
		Service2() = default;
		explicit Service2(Service1& s) : s1{&s} {}
		
		Service1* s1 = nullptr;
	};
	
	struct Definition1 : kgr::single_service<Service1> {};
	
	struct Definition2 {
		explicit Definition2(kgr::in_place_t) {}
		Definition2(kgr::in_place_t, Service1& dep) : service{dep} {}
		
		Service2 forward() {
			return std::move(service);
		}
		
		static auto construct(kgr::inject_t<Definition1> d1) -> decltype(kgr::inject(d1.forward())) {
			return kgr::inject(d1.forward());
		}
		
		Service2 service;
	};
	
	kgr::container c;
	
	REQUIRE(c.service<Definition2>().s1 == &c.service<Definition1>());
}
	
TEST_CASE("Injected arguments can be single and non single", "[dependency]") {
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
	
	struct Definition1 : kgr::single_service<Service1> {};
	struct Definition2 : kgr::service<Service2> {};
	
	struct Definition3 {
		Definition3(kgr::in_place_t, Service1& dep1, Service2 dep2) : service{dep1, std::move(dep2)} {}
		
		Service3 forward() {
			return std::move(service);
		}
		
		static auto construct(kgr::inject_t<Definition1> d1, kgr::inject_t<Definition2> d2) -> decltype(kgr::inject(d1.forward(), d2.forward())) {
			return kgr::inject(d1.forward(), d2.forward());
		}
		
		Service3 service;
	};
	
	(void) kgr::container{}.service<Definition3>();
	
	REQUIRE(constructor_called);
}

TEST_CASE("Container injects arguments recursively", "[dependency]") {
	static bool service1_constructed;
	static bool service2_constructed;
	static bool service3_one_constructed;
	static bool service3_two_constructed;
	static bool service4_constructed;
	
	service1_constructed = false;
	service2_constructed = false;
	service3_one_constructed = false;
	service3_two_constructed = false;
	service4_constructed = false;
	
	struct Service1 {
		Service1() { service1_constructed = true; }
	};
	
	struct Service2 {
		Service2() = default;
		explicit Service2(Service1) { service2_constructed = true; }
	};
	
	struct Service3 {
		Service3() = default;
		Service3(Service1, Service2) { service3_two_constructed = true; }
		explicit Service3(Service2) { service3_one_constructed = true; }
	};
	
	struct Service4 {
		Service4() = default;
		explicit Service4(Service3) { service4_constructed = true; }
	};
	
	struct Definition1 : kgr::service<Service1> {};
	struct Definition2 : kgr::service<Service2, kgr::dependency<Definition1>> {};
	struct Definition3One : kgr::service<Service3, kgr::dependency<Definition2>> {};
	struct Definition3Two : kgr::service<Service3, kgr::dependency<Definition1, Definition2>> {};
	struct Definition4 : kgr::service<Service4, kgr::dependency<Definition3Two>> {};
	
	SECTION("Injected service can have dependencies") {
		(void) kgr::container{}.service<Definition3One>();
		
		CHECK(service1_constructed);
		CHECK(service2_constructed);
		CHECK(!service3_two_constructed);
		CHECK(!service4_constructed);
		REQUIRE(service3_one_constructed);
	}
	
	SECTION("Injected service can have multiple dependencies") {
		(void) kgr::container{}.service<Definition4>();
		
		CHECK(service1_constructed);
		CHECK(service2_constructed);
		CHECK(!service3_one_constructed);
		CHECK(service3_two_constructed);
		REQUIRE(service4_constructed);
	}
}
