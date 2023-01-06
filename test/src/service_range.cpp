#include <catch2/catch.hpp>
#include <kangaru-prev/kangaru.hpp>
#include <algorithm>
#include <array>
#include <initializer_list>

namespace service_range_test {

template<typename It, typename V>
void test_iterator_values(It b, It e, std::initializer_list<V> values) {
	for (auto value : values) {
		auto has_value = std::any_of(b, e, [&](decltype((*e)) elem) {
			return elem.type == value;
		});
		
		CHECK(has_value);
	}
}

template<typename It, typename V>
void test_iterator_not_values(It b, It e, std::initializer_list<V> values) {
	for (auto value : values) {
		auto exclude_value = std::all_of(b, e, [&](decltype((*e)) elem) {
			return elem.type != value;
		});
		
		CHECK(exclude_value);
	}
}

enum struct Type { BaseT, Derived1T, Derived2T };

struct Base {
	Base(Type t = Type::BaseT) : type{ t } {}
	Type type;
};

struct BaseService : kgr::single_service<Base>, kgr::polymorphic {};

struct Derived1Service : kgr::single_service<Base>, kgr::overrides<BaseService> {
	static auto construct() noexcept -> kgr::inject_result<Type> {
		return kgr::inject(Type::Derived1T);
	}
};

struct Derived2Service : kgr::single_service<Base>, kgr::overrides<BaseService> {
	static auto construct() noexcept -> kgr::inject_result<Type> {
		return kgr::inject(Type::Derived2T);
	}
};

struct AbstractService : kgr::abstract_service<Base> {};
struct Concrete1Service : kgr::single_service<Base>, kgr::overrides<AbstractService> {};
struct Concrete2Service : kgr::single_service<Base>, kgr::overrides<AbstractService> {};

}

TEST_CASE("The container holds a list of overriders", "[service_range, virtual]") {
	using namespace service_range_test;
	kgr::container container;
	
	SECTION("Can retrieve") {
		container.emplace<BaseService>();
		REQUIRE(container.service<BaseService>().type == Type::BaseT);
		
		container.emplace<Derived1Service>();
		
		REQUIRE(container.service<BaseService>().type == Type::Derived1T);
		
		SECTION("All service of type") {
			container.emplace<Derived2Service>();
			REQUIRE(container.service<BaseService>().type == Type::Derived2T);
			
			auto range = container.service<kgr::override_range_service<BaseService>>();
			
			test_iterator_values(
				range.begin(), range.end(),
				{Type::BaseT, Type::Derived1T, Type::Derived2T}
			);
			
			CHECK(std::distance(range.begin(), range.end()) == 3);
		}
	}
	
	SECTION("Only service in the fork") {
		kgr::container fork;
		
		SECTION("filtering except") {
			container.emplace<BaseService>();
			container.emplace<Derived1Service>();
			fork = container.fork(kgr::except<Derived1Service>{});
			fork.emplace<Derived2Service>();
		}
		
		SECTION("filtering only") {
			container.emplace<BaseService>();
			container.emplace<Derived1Service>();
			fork = container.fork(kgr::only<BaseService>{});
			fork.emplace<Derived2Service>();
		}
		
		SECTION("emplace in original") {
			container.emplace<BaseService>();
			fork = container.fork();
			container.emplace<Derived1Service>();
			fork.emplace<Derived2Service>();
		}
		
		auto const range_original = container.service<kgr::override_range_service<BaseService>>();
		auto const range_fork = fork.service<kgr::override_range_service<BaseService>>();
		
		test_iterator_values(
			range_fork.begin(), range_fork.end(),
			{Type::BaseT, Type::Derived2T}
		);
		
		test_iterator_values(
			range_original.begin(), range_original.end(),
			{Type::BaseT, Type::Derived1T}
		);
		
		test_iterator_not_values(
			range_fork.begin(), range_fork.end(),
			{Type::Derived1T}
		);
		
		test_iterator_not_values(
			range_original.begin(), range_original.end(),
			{Type::Derived2T}
		);
	}
	
	SECTION("Of abstract services") {
		container.emplace<Concrete1Service>(Type::Derived1T);
		container.emplace<Concrete2Service>(Type::Derived2T);
		
		auto const range = container.service<kgr::override_range_service<AbstractService>>();
		
		test_iterator_values(
			range.begin(), range.end(),
			{Type::Derived1T, Type::Derived2T}
		);
		
		test_iterator_not_values(
			range.begin(), range.end(),
			{Type::BaseT}
		);
		
		CHECK(std::distance(range.begin(), range.end()) == 2);
	}
}
