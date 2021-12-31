#include <catch2/catch.hpp>
#include <kangaru/kangaru.hpp>
#include <string>

namespace invoker_constructible_from_container {

struct map1;
struct map2;

struct Service {
	friend auto service_map(Service const&) -> kgr::autowire;
};

struct Service2 {
	friend auto service_map(Service2 const&, kgr::map_t<map1>) -> kgr::autowire;
};

struct Service3 {
	friend auto service_map(Service3 const&, kgr::map_t<map2>) -> kgr::autowire;
};

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
	
	SECTION("Can use different maps") {
		bool called1 = false;
		auto lambda1 = [&](Service2) { called1 = true; return std::string{"value-from-lambda"}; };
		bool called2 = false;
		auto lambda2 = [&](Service3) { called2 = true; return std::string{"value-from-lambda"}; };
		bool called3 = false;
		auto lambda3 = [&](Service2, Service3) { called3 = true; return std::string{"value-from-lambda"}; };
		
		REQUIRE(invoker1(kgr::map<map1>{}, lambda1) == "value-from-lambda");
		REQUIRE(invoker1(kgr::map<map2>{}, lambda2) == "value-from-lambda");
		REQUIRE(invoker1(kgr::map<map1, map2>{}, lambda3) == "value-from-lambda");
		
		CHECK((!kgr::detail::is_callable<kgr::invoker, decltype(lambda1)>::value));
		CHECK((!kgr::detail::is_callable<kgr::invoker, decltype(lambda2)>::value));
		CHECK((!kgr::detail::is_callable<kgr::invoker, decltype(lambda3)>::value));
		
		CHECK((!kgr::detail::is_callable<kgr::mapped_invoker<kgr::map<struct other_map>>, decltype(lambda1), kgr::map<map1>>::value));
		CHECK((!kgr::detail::is_callable<kgr::mapped_invoker<kgr::map<struct other_map>>, decltype(lambda2), kgr::map<map2>>::value));
		CHECK((!kgr::detail::is_callable<kgr::mapped_invoker<kgr::map<struct other_map>>, decltype(lambda3), kgr::map<map1, map2>>::value));
	}
	
	SECTION("Reject invalid functions") {
		auto lambda = [&](Service, float, std::string) { };
		
		REQUIRE((!kgr::detail::is_callable<kgr::invoker, decltype(lambda)>::value));
		REQUIRE((!kgr::detail::is_callable<kgr::invoker, decltype(lambda), float>::value));
		REQUIRE((!kgr::detail::is_callable<kgr::invoker, decltype(lambda), std::string>::value));
		REQUIRE((kgr::detail::is_callable<kgr::invoker, decltype(lambda), float, std::string>::value));
	}
}

} // namespace invoker_constructible_from_container

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
	CHECK(sizeof(decltype(lazy1)) == sizeof(kgr::container*) + sizeof(void*));
	CHECK(sizeof(kgr::optional<kgr::service_type<Definition1>>) == sizeof(void*));
	CHECK(kgr::detail::is_trivially_copy_constructible<decltype(lazy1)>::value);
	CHECK(kgr::detail::is_trivially_copy_assignable<decltype(lazy1)>::value);
#if !defined(__GNUC__) || __GNUC__ >= 5 // GCC 4 has no support for move triviality
	CHECK(kgr::detail::is_trivially_move_constructible<decltype(lazy1)>::value);
	CHECK(kgr::detail::is_trivially_move_assignable<decltype(lazy1)>::value);
#endif
	
	auto& service1 = *lazy1;
	
	CHECK(service1_constructed);
	REQUIRE((std::is_same<decltype(service1), Service1&>::value));
	
	auto lazy2 = container.service<kgr::lazy_service<Definition2>>();
	
	CHECK(!service2_constructed);
	
	auto service2 = *lazy2;
	
	CHECK(service2_constructed);
	REQUIRE((std::is_same<decltype(service2), Service2>::value));
}

TEST_CASE("Optional act correctly for both value and reference", "[operator]") {
	kgr::optional<int> opt_int;
	
	REQUIRE((std::is_same<decltype(*opt_int), int&>::value));
	REQUIRE((std::is_same<decltype(*static_cast<kgr::optional<int> const&>(opt_int)), int const&>::value));
	
	struct Simple { int member; };
	kgr::optional<Simple> opt_simple;
	
	REQUIRE((std::is_same<decltype(*opt_simple), Simple&>::value));
	REQUIRE((std::is_same<decltype(*static_cast<kgr::optional<Simple> const&>(opt_simple)), Simple const&>::value));
	
	REQUIRE((std::is_same<decltype((opt_simple->member)), int&>::value));
	REQUIRE((std::is_same<decltype((static_cast<kgr::optional<Simple> const&>(opt_simple)->member)), int const&>::value));
	
	REQUIRE(not opt_int.has_value());
	opt_int.emplace(1);
	
	REQUIRE(opt_int.has_value());
	REQUIRE(*opt_int == 1);
	REQUIRE(*opt_int == opt_int.value());
}

TEST_CASE("All operator are mapped", "[service_map, operator]") {
	struct map {};
	struct service1 {};
	struct definition1 : kgr::service<service1> {};
	
	CHECK((std::is_same<kgr::mapped_service_t<kgr::generator<definition1>>, kgr::generator_service<definition1>>::value));
	CHECK((std::is_same<kgr::mapped_service_t<kgr::forked_generator<definition1>>, kgr::forked_generator_service<definition1>>::value));
	CHECK((std::is_same<kgr::mapped_service_t<kgr::invoker>, kgr::invoker_service>::value));
	CHECK((std::is_same<kgr::mapped_service_t<kgr::forked_invoker>, kgr::forked_invoker_service>::value));
	CHECK((std::is_same<kgr::mapped_service_t<kgr::mapped_invoker<kgr::map<map>>>, kgr::mapped_invoker_service<kgr::map<map>>>::value));
	CHECK((std::is_same<kgr::mapped_service_t<kgr::forked_mapped_invoker<kgr::map<map>>>, kgr::forked_mapped_invoker_service<kgr::map<map>, kgr::all>>::value));
	CHECK((std::is_same<kgr::mapped_service_t<kgr::lazy<definition1>>, kgr::lazy_service<definition1>>::value));
	CHECK((std::is_same<kgr::mapped_service_t<kgr::forked_lazy<definition1>>, kgr::forked_lazy_service<definition1, kgr::all>>::value));
	CHECK((std::is_same<kgr::mapped_service_t<kgr::container>, kgr::fork_service>::value));
	CHECK((std::is_same<kgr::mapped_service_t<kgr::container&&>, kgr::fork_service>::value));
	CHECK((std::is_same<kgr::mapped_service_t<kgr::container const&>, kgr::container_service>::value));
	CHECK((std::is_same<kgr::mapped_service_t<kgr::container&>, kgr::container_service>::value));
}
