#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "kangaru/kangaru.hpp"

namespace basic_mapping {
	struct Service {};
	struct Definition : kgr::Service<Service> {};
	auto service_map(Service const&) -> Definition;
	
	TEST_CASE("The basic service map maps function to services", "[service_map]") {
		REQUIRE((std::is_same<kgr::service_map_t<Service>, Definition>::value));
	}
}
