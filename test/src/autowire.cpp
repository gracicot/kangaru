#include "catch.hpp"
#include "kangaru/kangaru.hpp"

namespace test_autowire_construct {

struct service1 {};
auto service_map(service1 const&) -> kgr::autowire;

struct service2 { friend auto service_map(service2 const&) -> kgr::autowire; };

TEST_CASE("autowire can construct a service", "[container]") {
	SECTION("Without dependency") {
		REQUIRE(kgr::detail::is_service_valid<kgr::autowired<service1>>::value);
		(void) kgr::container{}.service<kgr::autowired<service1>>();
	}
	
	SECTION("Without dependency with a friend function") {
		REQUIRE(kgr::detail::is_service_valid<kgr::autowired<service2>>::value);
		(void) kgr::container{}.service<kgr::autowired<service2>>();
	}
}

}

namespace test_autowire_depencency {

int nb_s1_constructed;

struct service1 { service1() { nb_s1_constructed++; } };
auto service_map(service1 const&) -> kgr::autowire;

struct service2 {
	service1 s1;
	
	friend auto service_map(service2 const&) -> kgr::autowire;
};

TEST_CASE("autowire can have depencencies", "[container]") {
	nb_s1_constructed = 0;
	SECTION("Without dependency") {
		REQUIRE(kgr::detail::is_service_valid<kgr::autowired<service1>>::value);
		(void) kgr::container{}.service<kgr::autowired<service1>>();
		
		CHECK(nb_s1_constructed == 1);
		
		REQUIRE(kgr::detail::is_service_valid<kgr::autowired<service2>>::value);
		(void) kgr::container{}.service<kgr::autowired<service2>>();
		
		CHECK(nb_s1_constructed == 2);
	}
}

}

namespace test_autowire_single {

int nb_s1_constructed;

struct service1 { service1() { nb_s1_constructed++; } };
auto service_map(service1 const&) -> kgr::autowire_single;

struct service2 {
	service1& s1;
	
	friend auto service_map(service2 const&) -> kgr::autowire;
};

struct service3 {
	service3&& s3;
	
	friend auto service_map(service3 const&) -> kgr::autowire;
};

struct service4 {
	kgr::container forked;
	service2 s2;
	
	friend auto service_map(service4 const&) -> kgr::autowire;
};

struct service5 {
	kgr::container& ref;
	
	friend auto service_map(service5 const&) -> kgr::autowire_service<service5, kgr::autowire>;
};


TEST_CASE("autowire single behave as a single service", "[container]") {
	nb_s1_constructed = 0;
	
	REQUIRE(kgr::detail::is_single<kgr::autowired<service1>>::value);
	kgr::container c;
	
	SECTION("When injected as dependency") {
		REQUIRE(kgr::detail::is_service_valid<kgr::autowired<service1>>::value);
		REQUIRE(kgr::detail::is_service_valid<kgr::autowired<service2>>::value);
		
		auto& s1 = c.service<kgr::autowired<service1>>();
		CHECK(nb_s1_constructed == 1);
		
		auto s2 = c.service<kgr::autowired<service2>>();
		CHECK(nb_s1_constructed == 1);
		
		REQUIRE(&s1 == &s2.s1);
	}
	
	SECTION("Cannot inject itself into itself") {
		REQUIRE(!kgr::detail::is_service_valid<kgr::autowired<service3>>::value);
	}
	
	SECTION("Work with multiple dependencies") {
		REQUIRE(kgr::detail::is_service_valid<kgr::autowired<service4>>::value);
		(void) kgr::container{}.service<kgr::autowired<service4>>();
	}
}

}
