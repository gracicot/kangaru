#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "kangaru/kangaru.hpp"

#include <random>
#include <limits>

inline namespace {

static std::mt19937 random{std::random_device{}()};

static std::uniform_int_distribution<int> any_int_distribution{
	std::numeric_limits<int>::min(),
	std::numeric_limits<int>::max()
};

static std::uniform_real_distribution<double> any_double_distribution{
	std::numeric_limits<double>::min(),
	std::numeric_limits<double>::max()
};

namespace testcase_generic_inject_mapped {
	struct Service1 {};
	struct Definition1 : kgr::service<Service1> {};
	struct Service2 {};
	struct Definition2 : kgr::single_service<Service2> {};
	
	auto service_map(const Service1&) -> Definition1;
	auto service_map(const Service2&) -> Definition2;
	
	TEST_CASE("The container inject service parameters using service map", "[generic_lambdas]") {
		kgr::container c;
		
		bool called = false;
		
		SECTION("Deduces one argument") {
			c.invoke([&](auto a) {
				REQUIRE((std::is_same<std::decay_t<decltype(a)>, int>::value));
				called = true;
			}, 1);
			REQUIRE(called);
		}
		
		SECTION("Forward deduced arguments after injection") {
			const auto arg1 = any_int_distribution(random);
			
			auto function = [&](Service1, auto&& a) {
				called = true;
				
				REQUIRE((std::is_same<decltype(a), const int&>::value));
				CHECK(a == arg1);
			};
			
			c.invoke(function, arg1);
			
			REQUIRE(called);
		}
		
		SECTION("Forward many deduced arguments after injection") {
			const auto arg1 = any_int_distribution(random);
			const auto arg2 = any_double_distribution(random);
			
			auto function = [&](Service1, auto&& a, auto&& b) {
				called = true;
				
				REQUIRE((std::is_same<decltype(a), const int&>::value));
				REQUIRE((std::is_same<decltype(b), const double&>::value));
				CHECK(a == arg1);
				CHECK(b == arg2);
			};
			
			c.invoke(function, arg1, arg2);
			
			REQUIRE(called);
		}
		
		SECTION("Forward variadic deduced arguments after injection") {
			auto function = [&](Service1, Service2&, auto&&... a) {
				called = true;
				REQUIRE(sizeof...(a) == 3);
			};
			
			c.invoke(function, "test", 2.9, std::tuple<>{});
			
			REQUIRE(called);
			REQUIRE(c.contains<Definition2>());
		}
		
		SECTION("Forward mix of deduced and regular parameter after injection") {
			const auto arg1 = any_int_distribution(random);
			
			auto function = [&](Service1, Service2&, int a, auto b) {
				called = true;
				REQUIRE((std::is_same<std::decay_t<decltype(b)>, double>::value));
				CHECK(a == arg1);
			};
			
			c.invoke(function, arg1, 9.3);
			
			REQUIRE(called);
			REQUIRE(c.contains<Definition2>());
		}
		
		SECTION("Forward mix of variadic deduced and regular parameter after injection") {
			const auto arg1 = any_int_distribution(random);
			
			auto function = [&](Service1, Service2&, int a, auto... b) {
				called = true;
				REQUIRE(sizeof...(b) == 4);
				CHECK(a == arg1);
			};
			
			c.invoke(function, arg1, 9.3, "test", 3, std::tuple<>{});
			
			REQUIRE(called);
			REQUIRE(c.contains<Definition2>());
		}
	}
}

}
