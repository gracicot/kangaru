#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "kangaru/kangaru.hpp"

TEST_CASE("Container returns a service", "[service]") {
	SECTION("Returns the service from definition") {
		struct Service {};
		struct Definition {
			Definition(kgr::in_place_t) {}
			
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
			
			static std::true_type test(Service) {
				return {};
			}
		};
		
		REQUIRE(Result::test(kgr::Container{}.service<Definition>()));
	}
	
	SECTION("Move the service") {
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
		
		struct Definition : kgr::Service<Service> {};
		
		REQUIRE_FALSE(kgr::Container{}.service<Definition>().copied);
	}
}
