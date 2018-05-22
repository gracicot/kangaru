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

TEST_CASE("The container can call functions using invoke", "[invoke]") {
	kgr::container c;
	
	bool called = false;
	
	c.invoke([&] { called = true; });
	
	REQUIRE(called);
}

TEST_CASE("The container returns value from invoked function", "[invoke]") {
	kgr::container c;
	
	bool called = false;
	const static auto value = any_int_distribution(random);
	using value_type = decltype(value);
	
	auto function = [&] {
		called = true;
		
		return value;
	};
	
	using returned_type = decltype(c.invoke(function));
	
	REQUIRE((std::is_same<std::remove_cv<returned_type>::type, std::remove_cv<value_type>::type>::value));
	
	auto returned_value = c.invoke(function);
	
	REQUIRE(called);
	REQUIRE(returned_value == value);
}

namespace testcase_arguments {
	struct generic_functor1 {
		explicit generic_functor1(bool& c, int e1, double e2) noexcept : called{c}, expected1{e1}, expected2{e2} {}
		
		template<typename T1, typename T2>
		void operator()(T1&& a, T2&& b) {
			called = true;
			
			REQUIRE((std::is_same<typename std::decay<T1>::type, int>::value));
			REQUIRE((std::is_same<typename std::decay<T2>::type, double>::value));
			
			CHECK(a == expected1);
			CHECK(b == expected2);
		}
		
	private:
		bool& called;
		int expected1;
		double expected2;
	};
	
	struct generic_functor2 {
		explicit generic_functor2(bool& c, int e1, double e2) noexcept : called{c}, expected1{e1}, expected2{e2} {}
		
		template<typename T>
		void operator()(int a, T&& b) {
			called = true;
			
			REQUIRE((std::is_same<typename std::decay<T>::type, double>::value));
			
			CHECK(a == expected1);
			CHECK(b == expected2);
		}
		
	private:
		bool& called;
		int expected1;
		double expected2;
	};

	TEST_CASE("The container forwards arguments to the function", "[invoke]") {
		kgr::container c;
		
		bool called = false;
		
		const auto arg1 = any_int_distribution(random);
		const auto arg2 = any_double_distribution(random);
		
		SECTION("Regular parameter") {
			auto function = [&](int a, double b) {
				called = true;
				
				CHECK(a == arg1);
				CHECK(b == arg2);
			};
			
			c.invoke(function, arg1, arg2);
			
			REQUIRE(called);
		}
		
		SECTION("Templated parameter") {
			c.invoke(generic_functor1{called, arg1, arg2}, arg1, arg2);
			
			REQUIRE(called);
		}
		
		SECTION("Mix templated and regular parameter") {
			c.invoke(generic_functor2{called, arg1, arg2}, arg1, arg2);
			
			REQUIRE(called);
		}
	}
} // namespace testcase_arguments

namespace testcase_inject_mapped {
	struct Service1 {};
	struct Definition1 : kgr::service<Service1> {};
	struct Service2 {};
	struct Definition2 : kgr::single_service<Service2> {};
	
	auto service_map(const Service1&) -> Definition1;
	auto service_map(const Service2&) -> Definition2;
	
	struct generic_functor1 {
		explicit generic_functor1(bool& c, int e1, double e2) noexcept : called{c}, expected1{e1}, expected2{e2} {}
		
		template<typename T1, typename T2>
		void operator()(Service2&, Service1, T1&& a, T2&& b) {
			called = true;
			
			REQUIRE((std::is_same<typename std::decay<T1>::type, int>::value));
			REQUIRE((std::is_same<typename std::decay<T2>::type, double>::value));
			
			CHECK(a == expected1);
			CHECK(b == expected2);
		}
		
	private:
		bool& called;
		int expected1;
		double expected2;
	};
	
	struct generic_functor2 {
		explicit generic_functor2(bool& c, int e1, double e2) noexcept : called{c}, expected1{e1}, expected2{e2} {}
		
		template<typename T1>
		void operator()(Service2&, Service1, int a, T1&& b) {
			called = true;
			
			REQUIRE((std::is_same<typename std::decay<T1>::type, double>::value));
			
			CHECK(a == expected1);
			CHECK(b == expected2);
		}
		
	private:
		bool& called;
		int expected1;
		double expected2;
	};
	
	struct generic_functor3 {
		explicit generic_functor3(bool& c, int e1, std::size_t s) noexcept : called{c}, expected1{e1}, size{s} {}
		
		template<typename... Ts>
		void operator()(Service2&, Service1, int a, Ts&&... bs) {
			called = true;
			
			CHECK(a == expected1);
			CHECK(sizeof...(bs) == size);
		}
		
	private:
		bool& called;
		int expected1;
		std::size_t size;
	};

	TEST_CASE("The container inject service parameters using service map", "[invoke]") {
		kgr::container c;
		
		bool called = false;
		
		SECTION("Inject one argument") {
			c.invoke([&](Service1) { called = true; });
			REQUIRE(called);
		}
		
		SECTION("Inject singles") {
			c.invoke([&](Service2&) { called = true; });
			REQUIRE(called);
			REQUIRE(c.contains<Definition2>());
		}
		
		SECTION("Inject many parameter") {
			c.invoke([&](Service2&, Service1) { called = true; });
			REQUIRE(called);
			REQUIRE(c.contains<Definition2>());
		}
		
		SECTION("Forward arguments after injection") {
			const auto arg1 = any_int_distribution(random);
			const auto arg2 = any_double_distribution(random);
			
			auto function = [&](Service2&, Service1, int a, double b) {
				called = true;
				
				CHECK(a == arg1);
				CHECK(b == arg2);
			};
			
			c.invoke(function, arg1, arg2);
			
			REQUIRE(called);
			REQUIRE(c.contains<Definition2>());
		}
		
		SECTION("Forward deduced arguments after injection") {
			const auto arg1 = any_int_distribution(random);
			const auto arg2 = any_double_distribution(random);
			
			c.invoke(generic_functor1{called, arg1, arg2}, arg1, arg2);
			
			REQUIRE(called);
			REQUIRE(c.contains<Definition2>());
		}
		
		SECTION("Forward mix of deduced and regular parameter after injection") {
			const auto arg1 = any_int_distribution(random);
			const auto arg2 = any_double_distribution(random);
			
			c.invoke(generic_functor2{called, arg1, arg2}, arg1, arg2);
			
			REQUIRE(called);
			REQUIRE(c.contains<Definition2>());
		}
		
		SECTION("Forward mix of variadic deduced and regular parameter after injection") {
			const auto arg1 = any_int_distribution(random);
			
			c.invoke(generic_functor3{called, arg1, 4}, arg1, 1, "arg2", 3.0, 4.f);
			
			REQUIRE(called);
			REQUIRE(c.contains<Definition2>());
		}
	}
}

} // namespace
