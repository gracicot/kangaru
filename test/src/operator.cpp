#include <catch2/catch.hpp>
#include <kangaru/kangaru.hpp>
#include <string>

namespace {
	struct Service {
		friend auto service_map(Service const&) -> kgr::autowire;
	};
}

TEST_CASE("Invoker is constructible from a container or with a service", "[operator]") {
	kgr::container container;
	
	auto invoker1 = container.service<kgr::invoker_service>();
	
	REQUIRE((std::is_same<decltype(invoker1), kgr::invoker>::value));
	
	kgr::invoker invoker2{container};
	
	REQUIRE(sizeof(invoker2) == sizeof(kgr::container*));
	
	SECTION("Invoker has a call operator") {
		bool called = false;
		auto lambda = [&] { called = true; return std::string{"value-from-lambda"}; };
		
		REQUIRE(invoker1(lambda) == "value-from-lambda");
	}
	
	SECTION("Can inject service") {
		bool called = false;
		auto lambda = [&](Service, kgr::container&, kgr::invoker) { called = true; return std::string{"value-from-lambda"}; };
		
		REQUIRE(invoker1(lambda) == "value-from-lambda");
	}
	
	SECTION("Can get additional arugments service") {
		bool called = false;
		auto lambda = [&](Service, int) { called = true; return std::string{"value-from-lambda"}; };
		
		REQUIRE(invoker1(lambda, 1) == "value-from-lambda");
	}
	
	SECTION("Reject invalid functions") {
		auto lambda = [&](Service, float, std::string) { };
		
		REQUIRE((!kgr::detail::is_callable<kgr::invoker, decltype(lambda)>::value));
		REQUIRE((!kgr::detail::is_callable<kgr::invoker, decltype(lambda), float>::value));
		REQUIRE((!kgr::detail::is_callable<kgr::invoker, decltype(lambda), std::string>::value));
		REQUIRE((kgr::detail::is_callable<kgr::invoker, decltype(lambda), float, std::string>::value));
	}
}

TEST_CASE("Container can fork with a service", "[operator]") {
	struct Service1 {};
	struct Service2 {};
	struct Definition1 : kgr::single_service<Service1> {};
	struct Definition2 : kgr::single_service<Service2> {};
	
	auto container1 = kgr::container{};
	
	container1.emplace<Definition1>();
	
	auto container2 = container1.service<kgr::fork_service>();
	
	REQUIRE((std::is_same<decltype(container2), decltype(container1)>::value));
	REQUIRE(container2.contains<Definition1>());
	REQUIRE(!container2.contains<Definition2>());
	
	container2.emplace<Definition2>();
	
	REQUIRE(container2.contains<Definition2>());
	REQUIRE(!container1.contains<Definition2>());
}

TEST_CASE("Generators are object that contstruct a particular service", "[operator]") {
	struct Service1 {};
	struct Definition1 : kgr::service<Service1> {};
	
	auto container = kgr::container{};
	auto generator = container.service<kgr::generator_service<Definition1>>();
	
	CHECK(sizeof(generator) == sizeof(kgr::container*));
	
	auto service = generator();
	
	REQUIRE((std::is_same<decltype(service), Service1>::value));
}

TEST_CASE("Lazy service defer service call", "[operator]") {
	static bool service1_constructed;
	static bool service2_constructed;
	
	service1_constructed = false;
	service2_constructed = false;
	
	struct Service1 {
		bool constructed = (service1_constructed = true);
	};
	
	struct Service2 {
		bool constructed = (service2_constructed = true);
	};
	
	struct Definition1 : kgr::single_service<Service1> {};
	struct Definition2 : kgr::service<Service2> {};
	
	auto container = kgr::container{};
	auto lazy1 = container.service<kgr::lazy_service<Definition1>>();
	
	CHECK(!service1_constructed);
	CHECK(sizeof(lazy1) == sizeof(kgr::container*) + sizeof(Service1*));
	
	auto& service1 = *lazy1;
	
	CHECK(service1_constructed);
	REQUIRE((std::is_same<decltype(service1), Service1&>::value));
	
	auto lazy2 = container.service<kgr::lazy_service<Definition2>>();
	
	CHECK(!service2_constructed);
	
	auto service2 = *lazy2;
	
	CHECK(service2_constructed);
	REQUIRE((std::is_same<decltype(service2), Service2>::value));
}
