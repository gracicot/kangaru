#include <catch2/catch.hpp>
#include <kangaru/kangaru.hpp>

// We make feature macro available
#include <kangaru/detail/define.hpp>

namespace test_autowire_construct {
	struct service1 {};
	auto service_map(service1 const&) -> kgr::autowire;
	
	struct service2 { friend auto service_map(service2 const&) -> kgr::autowire; };
	
	TEST_CASE("autowire can construct a service", "[autowire]") {
		SECTION("Without dependency") {
			REQUIRE(kgr::detail::is_service_valid<kgr::mapped_service_t<service1>>::value);
			(void) kgr::container{}.service<kgr::mapped_service_t<service1>>();
		}
		
		SECTION("Without dependency with a friend function") {
			REQUIRE(kgr::detail::is_service_valid<kgr::mapped_service_t<service2>>::value);
			(void) kgr::container{}.service<kgr::mapped_service_t<service2>>();
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
			REQUIRE(kgr::detail::is_service_valid<kgr::mapped_service_t<service1>>::value);
			(void) container.service<kgr::mapped_service_t<service1>>();
			
			CHECK(nb_s1_constructed == 1);
			
			REQUIRE(kgr::detail::is_service_valid<kgr::mapped_service_t<service2>>::value);
			(void) container.service<kgr::mapped_service_t<service2>>();
			
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
		service3& s3;
		
		friend auto service_map(service3 const&) -> kgr::autowire_single;
	};
	
	TEST_CASE("autowire single behave as a single service", "[autowire]") {
		nb_s1_constructed = 0;
		
		REQUIRE(kgr::detail::is_single<kgr::mapped_service_t<service1>>::value);
		kgr::container container;
	
		SECTION("When injected as dependency") {
			REQUIRE(kgr::detail::is_service_valid<kgr::mapped_service_t<service1>>::value);
			REQUIRE(kgr::detail::is_service_valid<kgr::mapped_service_t<service2>>::value);
			
			auto& s1 = container.service<kgr::mapped_service_t<service1>>();
			CHECK(nb_s1_constructed == 1);
			
			auto s2 = container.service<kgr::mapped_service_t<service2>>();
			CHECK(nb_s1_constructed == 1);
			
			REQUIRE(&s1 == &s2.s1);
		}
		
		SECTION("Cannot inject itself into itself") {
			REQUIRE(!kgr::detail::is_service_valid<kgr::mapped_service_t<service3>>::value);
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
			REQUIRE(kgr::detail::is_service_valid<kgr::mapped_service_t<service2>>::value);
			(void) container.service<kgr::mapped_service_t<service2>>();
			REQUIRE(nb_s1_constructed == 1);
		}
		
		SECTION("Work with multiple levels of dependencies") {
			REQUIRE(kgr::detail::is_service_valid<kgr::mapped_service_t<service3>>::value);
			(void) container.service<kgr::mapped_service_t<service3>>();
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
		
		REQUIRE(kgr::detail::is_service_valid<kgr::mapped_service_t<service1>>::value);
		REQUIRE(kgr::detail::is_service_valid<kgr::mapped_service_t<service2>>::value);
		(void) container.service<kgr::mapped_service_t<service1>>();
		(void) container.service<kgr::mapped_service_t<service2>>();
	}
}

namespace test_autowire_circular_error {
	struct service3;
	
	struct service1 {
		service3& s;
	};
	
	struct service2 {
		service1& s;
	};
	
	struct service3 {
		service2& s;
	};
	
	auto service_map(service1 const&) -> kgr::single_service<service1, kgr::autowire>;
	auto service_map(service2 const&) -> kgr::single_service<service2, kgr::autowire>;
	auto service_map(service3 const&) -> kgr::single_service<service3, kgr::autowire>;
	
	TEST_CASE("autowire detect circular dependency", "[autowire]") {
#ifndef KGR_KANGARU_MSVC_DISABLE_VALIDATION_AUTOWIRE
		REQUIRE(!kgr::detail::is_service_valid<kgr::mapped_service_t<service1>>::value);
		REQUIRE(!kgr::detail::is_service_valid<kgr::mapped_service_t<service2>>::value);
		REQUIRE(!kgr::detail::is_service_valid<kgr::mapped_service_t<service3>>::value);
#endif
	}
}

namespace test_autowire_service_error {
	struct ill_formed {};
	
	auto service_map(ill_formed const&) -> kgr::single_service<ill_formed, kgr::dependency<kgr::invoker_service>>;
	
	struct service2 {
		ill_formed& s1;
		kgr::container& container;
		
		friend auto service_map(service2 const&) -> kgr::service<service2, kgr::autowire>;
	};
	
	TEST_CASE("autowire service detect errors in autowired dependencies", "[autowire]") {
		kgr::container container;
		
#ifndef KGR_KANGARU_MSVC_DISABLE_VALIDATION_AUTOWIRE
		REQUIRE(!kgr::detail::is_service_valid<kgr::mapped_service_t<ill_formed>>::value);
		REQUIRE(!kgr::detail::is_service_valid<kgr::mapped_service_t<service2>>::value);
#endif
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

	TEST_CASE("Autowiring uses the map sent as parameter", "[service_map]") {
		kgr::container container;
		bad_constructed = false;
		
		REQUIRE((std::is_same<kgr::mapped_service_t<service1, kgr::map<map>>, definition1>::value));
		
		(void) container.service<definition2>();
		REQUIRE(!bad_constructed);
		
		(void) container.service<not_definition>();
		CHECK(bad_constructed);
	}
}

namespace test_autowire_default_services {
	struct service1 { service1(kgr::invoker) {} };
	struct service2 { service2(kgr::container, kgr::container&) {} };
	
	struct service3 { service3(std::unique_ptr<service1>, std::shared_ptr<service2>) {} };
	struct service4 { service4(std::shared_ptr<service3>, std::unique_ptr<service1>) {} };
	
	auto service_map(std::unique_ptr<service1> const&) -> kgr::autowire_unique;
	auto service_map(std::shared_ptr<service2> const&) -> kgr::autowire_shared;
	auto service_map(std::shared_ptr<service3> const&) -> kgr::autowire_shared;
	auto service_map(std::shared_ptr<service4> const&) -> kgr::autowire;
	
	auto service_map(std::unique_ptr<service1> const&, kgr::map_t<>) -> kgr::autowire_unique;
	auto service_map(std::shared_ptr<service2> const&, kgr::map_t<>) -> kgr::autowire_shared;
	auto service_map(std::shared_ptr<service3> const&, kgr::map_t<>) -> kgr::autowire_shared;
	auto service_map(std::shared_ptr<service4> const&, kgr::map_t<>) -> kgr::autowire;
	
	TEST_CASE("Autowire work with generic services provided", "[service_map]") {
		REQUIRE(kgr::detail::is_service_valid<kgr::mapped_service_t<std::unique_ptr<service1>>>::value);
		REQUIRE(kgr::detail::is_service_valid<kgr::mapped_service_t<std::shared_ptr<service2>>>::value);
		REQUIRE(kgr::detail::is_service_valid<kgr::mapped_service_t<std::shared_ptr<service3>>>::value);
		REQUIRE(kgr::detail::is_service_valid<kgr::mapped_service_t<std::shared_ptr<service4>>>::value);
		
		kgr::container container;
		
		(void) container.service<kgr::mapped_service_t<std::unique_ptr<service1>>>();
		(void) container.service<kgr::mapped_service_t<std::shared_ptr<service2>>>();
		(void) container.service<kgr::mapped_service_t<std::shared_ptr<service3>>>();
		(void) container.service<kgr::mapped_service_t<std::shared_ptr<service4>>>();
	}
}

namespace test_autowire_priority {
	int service3_nth_constructor_called = 0;
	struct service1 {};
	
	struct service2 {
		service2(std::unique_ptr<service1> _s1, std::unique_ptr<service1> _s2 = {}) : s1{std::move(_s1)}, s2{std::move(_s2)} {}
		std::unique_ptr<service1> s1;
		std::unique_ptr<service1> s2 = {};
	};
	
	struct service3 {
		service3(std::unique_ptr<service1>, int = 0) { service3_nth_constructor_called = 1; }
		service3(std::unique_ptr<service1>, kgr::container&, int) { service3_nth_constructor_called = 2; }
		service3(std::unique_ptr<service1>, kgr::container&, kgr::invoker, int) { service3_nth_constructor_called = 3; }
	};
	
	struct definition1 : kgr::unique_service<service1> {};
	struct definition2 : kgr::service<service2, kgr::autowire> {};
	struct definition3 : kgr::service<service3, kgr::autowire> {};
	
	auto service_map(std::unique_ptr<service1> const&) -> definition1;
	
	TEST_CASE("Autowire uses the maximum amount of services deductible", "[service_map]") {
		kgr::container container;
		
		auto service1 = container.service<definition2>();
		
		REQUIRE(service1.s1);
		REQUIRE(service1.s2);
		
		auto service2 = container.service<definition3>(32);
		
		(void) service2;
		
		REQUIRE(service3_nth_constructor_called == 3);
	}
}
