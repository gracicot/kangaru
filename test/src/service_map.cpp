#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "kangaru/kangaru.hpp"

namespace basic_mapping {
	struct Service {};
	struct Definition : kgr::service<Service> {};
	
	auto service_map(Service const&) -> Definition;
	
	TEST_CASE("The basic service map maps function to services using service_map", "[service_map]") {
		REQUIRE((std::is_same<kgr::service_map_t<Service>, Definition>::value));
	}
}

namespace empty_map_mapping {
	struct Service {};
	struct Definition : kgr::service<Service> {};
	struct NotDefinition : kgr::service<Service> {};
	
	auto service_map(Service const&, kgr::map_t<>) -> Definition;
	auto service_map(Service const&) -> NotDefinition;
	
	TEST_CASE("The service map maps function to services using service_map tagged", "[service_map]") {
		REQUIRE((std::is_same<kgr::service_map_t<Service>, Definition>::value));
	}
}

namespace basic_mapped_mapping {
	struct Service {};
	struct Definition : kgr::service<Service> {};
	struct NotDefinition : kgr::service<Service> {};
	struct Map {};
	
	auto service_map(Service const&, kgr::map_t<Map>) -> Definition;
	auto service_map(Service const&) -> NotDefinition;
	
	TEST_CASE("The service map maps function to services using service_map with specific map", "[service_map]") {
		REQUIRE((std::is_same<kgr::service_map_t<Service, kgr::map<Map>>, Definition>::value));
	}
}

namespace prioritized_single_mapped_mapping {
	struct Service {};
	struct Definition : kgr::service<Service> {};
	struct NotDefinition : kgr::service<Service> {};
	struct Map1 {};
	struct Map2 {};
	
	auto service_map(Service const&, kgr::map_t<Map1>) -> Definition;
	auto service_map(Service const&, kgr::map_t<Map2>) -> NotDefinition;
	auto service_map(Service const&) -> NotDefinition;
	
	TEST_CASE("The service map maps function to services using service_map with specific map, choosing the right map", "[service_map]") {
		REQUIRE((std::is_same<kgr::service_map_t<Service, kgr::map<Map1>>, Definition>::value));
	}
}

namespace prioritized_multi_mapped_mapping {
	struct Service {};
	struct Definition : kgr::service<Service> {};
	struct NotDefinition : kgr::service<Service> {};
	struct Map1 {};
	struct Map2 {};
	
	auto service_map(Service const&, kgr::map_t<Map1>) -> Definition;
	auto service_map(Service const&, kgr::map_t<Map2>) -> NotDefinition;
	auto service_map(Service const&) -> NotDefinition;
	
	TEST_CASE("The service map maps function to services using service_map with specific map, choosing the right map by priority", "[service_map]") {
		REQUIRE((std::is_same<kgr::service_map_t<Service, kgr::map<Map1, Map2>>, Definition>::value));
		REQUIRE((std::is_same<kgr::service_map_t<Service, kgr::map<Map2, Map1>>, NotDefinition>::value));
	}
}

namespace prioritized_multi_mapped_mapping_fallback1 {
	struct Service {};
	struct Definition : kgr::service<Service> {};
	struct NotDefinition : kgr::service<Service> {};
	struct Map1 {};
	struct Map2 {};
	
	auto service_map(Service const&, kgr::map_t<Map1>) -> void;
	auto service_map(Service const&, kgr::map_t<Map2>) -> Definition;
	auto service_map(Service const&) -> NotDefinition;
	
	TEST_CASE("The service map maps function to services fallbacking to a valid service", "[service_map]") {
		REQUIRE((std::is_same<kgr::service_map_t<Service, kgr::map<Map1, Map2>>, Definition>::value));
		REQUIRE((std::is_same<kgr::service_map_t<Service, kgr::map<Map2, Map1>>, Definition>::value));
	}
}

namespace prioritized_multi_mapped_mapping_fallback2 {
	struct Service {};
	struct Definition : kgr::service<Service> {};
	struct Map1 {};
	struct Map2 {};
	
	auto service_map(Service const&, kgr::map_t<Map1>) -> void;
	auto service_map(Service const&, kgr::map_t<Map2>) -> void;
	auto service_map(Service const&) -> Definition;
	
	TEST_CASE("The service map maps function to services fallbacking to void map for a valid service", "[service_map]") {
		REQUIRE((std::is_same<kgr::service_map_t<Service, kgr::map<Map1, Map2>>, Definition>::value));
		REQUIRE((std::is_same<kgr::service_map_t<Service, kgr::map<Map2, Map1>>, Definition>::value));
	}
}

namespace prioritized_multi_mapped_mapping_multi_service {
	struct Service1 {};
	struct Service2 {};
	struct Definition1 : kgr::service<Service1> {};
	struct Definition2 : kgr::service<Service2> {};
	struct NotDefinition1 : kgr::service<Service1> {};
	struct NotDefinition2 : kgr::service<Service2> {};
	struct Map1 {};
	struct Map2 {};
	
	auto service_map(Service1 const&, kgr::map_t<Map1>) -> Definition1;
	auto service_map(Service1 const&, kgr::map_t<Map2>) -> NotDefinition1;
	
	auto service_map(Service2 const&, kgr::map_t<Map2>) -> NotDefinition2;
	auto service_map(Service2 const&, kgr::map_t<Map1>) -> Definition2;
	
	TEST_CASE("The service map maps functions to multiple services", "[service_map]") {
		REQUIRE((std::is_same<kgr::service_map_t<Service1, kgr::map<Map1, Map2>>, Definition1>::value));
		REQUIRE((std::is_same<kgr::service_map_t<Service2, kgr::map<Map1, Map2>>, Definition2>::value));
		REQUIRE((std::is_same<kgr::service_map_t<Service1, kgr::map<Map2, Map1>>, NotDefinition1>::value));
		REQUIRE((std::is_same<kgr::service_map_t<Service2, kgr::map<Map2, Map1>>, NotDefinition2>::value));
	}
}

namespace prioritized_multi_mapped_invalid_mapping_multi_service {
	struct Service1 {};
	struct Service2 {};
	struct Definition1 : kgr::service<Service1> {};
	struct Definition2 : kgr::service<Service2> {};
	struct NotDefinition1 : kgr::service<Service1> {};
	struct NotDefinition2 : kgr::service<Service2> {};
	struct Map1 {};
	struct Map2 {};
	
	auto service_map(Service1 const&, kgr::map_t<Map1>) -> Definition2;
	auto service_map(Service1 const&, kgr::map_t<Map2>) -> NotDefinition1;
	
	auto service_map(Service2 const&, kgr::map_t<Map2>) -> NotDefinition2;
	auto service_map(Service2 const&, kgr::map_t<Map1>) -> Definition1;
	
	TEST_CASE("The service map maps functions to multiple services, fallbacking to a valid service", "[service_map]") {
		REQUIRE((std::is_same<kgr::service_map_t<Service1, kgr::map<Map1, Map2>>, NotDefinition1>::value));
		REQUIRE((std::is_same<kgr::service_map_t<Service2, kgr::map<Map1, Map2>>, NotDefinition2>::value));
	}
}

namespace mapped_value_category {
	struct Service1 {};
	struct Definition1 : kgr::service<Service1> {};
	struct Definition2 : kgr::service<Service1> {};
	
	auto service_map(Service1 const&) -> Definition1;
	auto service_map(Service1&&) -> Definition2;
	
	TEST_CASE("The service map maps functions to service with different value category", "[service_map]") {
		REQUIRE((std::is_same<kgr::service_map_t<const Service1&>, Definition1>::value));
		REQUIRE((std::is_same<kgr::service_map_t<Service1&&>, Definition2>::value));
	}
}
