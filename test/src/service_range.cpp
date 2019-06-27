#include <catch2/catch.hpp>
#include <kangaru/kangaru.hpp>
#include <algorithm>

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
		Derived1Service{}.emplace();
		
		container.emplace<Derived1Service>();
		
		REQUIRE(container.service<BaseService>().type == Type::Derived1);
		
		SECTION("All service of type") {
			container.emplace<Derived2Service>();
			REQUIRE(container.service<BaseService>().type == Type::Derived2);
			
			auto range = container.service<kgr::override_range_service<BaseService>>();
			auto elem = range.begin();
			REQUIRE(elem != range.end());
			CHECK((*elem).type == Type::Base);
			
			++elem;
			REQUIRE(elem != range.end());
			CHECK((*elem).type == Type::Derived1);
			
			++elem;
			REQUIRE(elem != range.end());
			CHECK((*elem).type == Type::Derived2);
			CHECK(++elem == range.end());
			CHECK(std::distance(range.begin(), range.end()) == 3);
		}
	}
}
