#include <catch2/catch.hpp>
#include <kangaru-prev/kangaru.hpp>

TEST_CASE("Container returns a service from a definition", "[service]") {
	struct Service {};
	struct Definition {
		explicit Definition(kgr::in_place_t) {}
		
		Service forward() {
			return std::move(service);
		}
		
		static auto construct() -> decltype(kgr::inject()) {
			return kgr::inject();
		}
		
		Service service;
	};
	
	struct Result {
		static std::false_type test(...) {
			return {};
		}
		
		static std::true_type test(Service&&) {
			return {};
		}
	};
	
	REQUIRE(Result::test(kgr::container{}.service<Definition>()));
}
	
TEST_CASE("Container returns a service by moving it", "[service]") {
	struct Service {
		Service() = default;
		Service(Service&&) = default;
		Service& operator=(Service&&) = default;
		
		bool copied = false;
		
		Service(const Service&) {
			copied = true;
		}
		
		Service& operator=(const Service&) {
			copied = true;
			
			return *this;
		}
	};
	
	struct Definition : kgr::service<Service> {};
	
	kgr::container c;
	
	auto service = c.service<Definition>();
	
	REQUIRE_FALSE(c.service<Definition>().copied);
	REQUIRE_FALSE(service.copied);
}
