#include <catch2/catch.hpp>
#include <kangaru/kangaru.hpp>
#include <algorithm>
#include <array>
#include <initializer_list>

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

TEST_CASE("The container holds a list of overriders", "[service_range, virtual]") {
	enum struct Type { Base, Derived1, Derived2 };
	
	struct Base {
		Base() = default;
		Base(Type t) : type{t} {}
		Type type = Type::Base;
	};
	
	struct BaseService : kgr::single_service<Base>, kgr::polymorphic {};
	
	struct Derived1Service : kgr::single_service<Base>, kgr::overrides<BaseService> {
		static auto construct() noexcept -> kgr::inject_result<Type> {
			return kgr::inject(Type::Derived1);
		}
	};
	
	struct Derived2Service : kgr::single_service<Base>, kgr::overrides<BaseService> {
		static auto construct() noexcept -> kgr::inject_result<Type> {
			return kgr::inject(Type::Derived2);
		}
	};
	
	kgr::container container;
	
	SECTION("Can retrieve") {
		container.emplace<BaseService>();
		REQUIRE(container.service<BaseService>().type == Type::Base);
		
		container.emplace<Derived1Service>();
		
		REQUIRE(container.service<BaseService>().type == Type::Derived1);
		
		SECTION("All service of type") {
			container.emplace<Derived2Service>();
			REQUIRE(container.service<BaseService>().type == Type::Derived2);
			
			auto range = container.service<kgr::override_range_service<BaseService>>();
			
			test_iterator_values(
				range.begin(), range.end(),
				{Type::Base, Type::Derived1, Type::Derived2}
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
		
		auto range_original = container.service<kgr::override_range_service<BaseService>>();
		auto range_fork = fork.service<kgr::override_range_service<BaseService>>();
		
		test_iterator_values(
			range_fork.begin(), range_fork.end(),
			{Type::Base, Type::Derived2}
		);
		
		test_iterator_values(
			range_original.begin(), range_original.end(),
			{Type::Base, Type::Derived1}
		);
		
		test_iterator_not_values(
			range_fork.begin(), range_fork.end(),
			{Type::Derived1}
		);
		
		test_iterator_not_values(
			range_original.begin(), range_original.end(),
			{Type::Derived2}
		);
	}
}
