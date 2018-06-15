#include "catch.hpp"
#include "kangaru/kangaru.hpp"

namespace test_autowire_construct {
	struct service1 {};
	auto service_map(service1 const&) -> kgr::autowire;

	struct service2 { friend auto service_map(service2 const&) -> kgr::autowire; };

	TEST_CASE("autowire can construct a service", "[autowire]") {
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

	TEST_CASE("autowire can have depencencies", "[autowire]") {
		nb_s1_constructed = 0;
		kgr::container container;

		SECTION("Without dependency") {
			REQUIRE(kgr::detail::is_service_valid<kgr::autowired<service1>>::value);
			(void) container.service<kgr::autowired<service1>>();
		
			CHECK(nb_s1_constructed == 1);
		
			REQUIRE(kgr::detail::is_service_valid<kgr::autowired<service2>>::value);
			(void) container.service<kgr::autowired<service2>>();
		
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

	TEST_CASE("autowire single behave as a single service", "[autowire]") {
		nb_s1_constructed = 0;
	
		REQUIRE(kgr::detail::is_single<kgr::autowired<service1>>::value);
		kgr::container container;
	
		SECTION("When injected as dependency") {
			REQUIRE(kgr::detail::is_service_valid<kgr::autowired<service1>>::value);
			REQUIRE(kgr::detail::is_service_valid<kgr::autowired<service2>>::value);
		
			auto& s1 = container.service<kgr::autowired<service1>>();
			CHECK(nb_s1_constructed == 1);
		
			auto s2 = container.service<kgr::autowired<service2>>();
			CHECK(nb_s1_constructed == 1);
		
			REQUIRE(&s1 == &s2.s1);
		}
	}
}

namespace test_autowire_multiple_dependency {
	int nb_s1_constructed;

	struct service1 { service1() { nb_s1_constructed++; } };
	auto service_map(service1 const&) -> kgr::autowire_single;

	struct service2 {
		service1& s1;
		kgr::container& container;

		friend auto service_map(service2 const&) -> kgr::autowire;
	};

	struct service3 {
		service3(kgr::container f, service2 s) : forked(std::move(f)), s2(std::move(s)) {}
		kgr::container forked;
		service2 s2;

		friend auto service_map(service3 const&) -> kgr::autowire;
	};

	TEST_CASE("autowired service can have multiple dependencis", "[autowire]") {
		nb_s1_constructed = 0;
		kgr::container container;

		SECTION("Work with multiple dependencies") {
			REQUIRE(kgr::detail::is_service_valid<kgr::autowired<service2>>::value);
			(void) container.service<kgr::autowired<service2>>();
			REQUIRE(nb_s1_constructed == 1);
		}

		SECTION("Work with multiple levels of dependencies") {
			REQUIRE(kgr::detail::is_service_valid<kgr::autowired<service3>>::value);
			(void) container.service<kgr::autowired<service3>>();
			REQUIRE(nb_s1_constructed == 1);
		}
	}
}

namespace test_autowire_normal_services {
	struct service1 {
		kgr::invoker i;
	};

	auto service_map(service1 const&) -> kgr::single_service<service1, kgr::autowire>;

	struct service2 {
		service1& s1;
		kgr::container& container;

		friend auto service_map(service2 const&) -> kgr::service<service2, kgr::autowire>;
	};

	TEST_CASE("autowire works with other service types", "[autowire]") {
		kgr::container container;

		REQUIRE(kgr::detail::is_service_valid<kgr::autowired<service1>>::value);
		REQUIRE(kgr::detail::is_service_valid<kgr::autowired<service2>>::value);
		(void) container.service<kgr::autowired<service1>>();
		(void) container.service<kgr::autowired<service2>>();
	}
}

namespace test_autowire_mapped {
	bool bad_constructed = false;

	struct service_bad {
		service_bad() { bad_constructed = true; }
	};
	struct service1 {
		service1() = default;
		service1(service_bad) {}
	};

	struct service2 {
		service1 s1;
	};

	struct map {};
	struct definition2 : kgr::service<service2, kgr::mapped_autowire<kgr::map<map>>> {};

	struct definition1 : kgr::service<service1> {};
	struct definition_bad : kgr::service<service_bad> {};
	struct not_definition : kgr::service<service1, kgr::dependency<definition_bad>> {};

	auto service_map(service1 const&, kgr::map_t<map>) -> definition1;
	auto service_map(service1 const&) -> not_definition;
	auto service_map(service2 const&, kgr::map_t<map>) -> definition2;

	TEST_CASE("The service map maps function to services using service_map with specific map", "[service_map]") {
		kgr::container container;
		bad_constructed = false;

		REQUIRE((std::is_same<kgr::autowired<service1, kgr::map<map>>, definition1>::value));

		(void) container.service<definition2>();
		REQUIRE(!bad_constructed);

		(void) container.service<not_definition>();
		CHECK(bad_constructed);
	}
}
