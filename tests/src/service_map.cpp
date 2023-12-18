#include <catch2/catch_test_macros.hpp>
#include <kangaru-prev/kangaru.hpp>

namespace basic_mapping {
	struct Service {};
	struct Definition : kgr::service<Service> {};
	
	auto service_map(Service const&) -> Definition;
	
	TEST_CASE("The basic service map maps function to services using service_map", "[service_map]") {
		REQUIRE((std::is_same<kgr::mapped_service_t<Service>, Definition>::value));
	}
}

namespace empty_map_mapping {
	struct Service {};
	struct Definition : kgr::service<Service> {};
	struct NotDefinition : kgr::service<Service> {};
	
	auto service_map(Service const&, kgr::map_t<>) -> Definition;
	auto service_map(Service const&) -> NotDefinition;
	
	TEST_CASE("The service map maps function to services using service_map tagged", "[service_map]") {
		REQUIRE((std::is_same<kgr::mapped_service_t<Service>, Definition>::value));
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
		REQUIRE((std::is_same<kgr::mapped_service_t<Service, kgr::map<Map>>, Definition>::value));
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
		REQUIRE((std::is_same<kgr::mapped_service_t<Service, kgr::map<Map1>>, Definition>::value));
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
		REQUIRE((std::is_same<kgr::mapped_service_t<Service, kgr::map<Map1, Map2>>, Definition>::value));
		REQUIRE((std::is_same<kgr::mapped_service_t<Service, kgr::map<Map2, Map1>>, NotDefinition>::value));
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
		REQUIRE((std::is_same<kgr::mapped_service_t<Service, kgr::map<Map1, Map2>>, Definition>::value));
		REQUIRE((std::is_same<kgr::mapped_service_t<Service, kgr::map<Map2, Map1>>, Definition>::value));
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
		REQUIRE((std::is_same<kgr::mapped_service_t<Service, kgr::map<Map1, Map2>>, Definition>::value));
		REQUIRE((std::is_same<kgr::mapped_service_t<Service, kgr::map<Map2, Map1>>, Definition>::value));
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
		REQUIRE((std::is_same<kgr::mapped_service_t<Service1, kgr::map<Map1, Map2>>, Definition1>::value));
		REQUIRE((std::is_same<kgr::mapped_service_t<Service2, kgr::map<Map1, Map2>>, Definition2>::value));
		REQUIRE((std::is_same<kgr::mapped_service_t<Service1, kgr::map<Map2, Map1>>, NotDefinition1>::value));
		REQUIRE((std::is_same<kgr::mapped_service_t<Service2, kgr::map<Map2, Map1>>, NotDefinition2>::value));
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
		REQUIRE((std::is_same<kgr::mapped_service_t<Service1, kgr::map<Map1, Map2>>, NotDefinition1>::value));
		REQUIRE((std::is_same<kgr::mapped_service_t<Service2, kgr::map<Map1, Map2>>, NotDefinition2>::value));
	}
}

namespace mapped_value_category {
	struct Service1 {};
	struct Definition1 : kgr::service<Service1> {};
	struct Definition2 : kgr::service<Service1> {};
	
	auto service_map(Service1 const&) -> Definition1;
	auto service_map(Service1&&) -> Definition2;
	
	TEST_CASE("The service map maps functions to service with different value category", "[service_map]") {
		REQUIRE((std::is_same<kgr::mapped_service_t<const Service1&>, Definition1>::value));
		REQUIRE((std::is_same<kgr::mapped_service_t<Service1&&>, Definition2>::value));
	}
}

namespace indirect_map {
	struct service1 {};
	struct service2 {};
	
	struct indirect_map {
		template<typename T>
		using mapped_service = kgr::service<T>;
	};
	
	auto service_map(service1 const&) -> indirect_map;
	auto service_map(service2&&) -> indirect_map;
	
	TEST_CASE("The indirect map yeild a serivce definition type throught mapped_service<T>", "[service_map]") {
		REQUIRE((std::is_same<kgr::mapped_service_t<service1>, kgr::service<service1>>::value));
		REQUIRE((std::is_same<kgr::mapped_service_t<service2>, kgr::service<service2>>::value));
	}
}

namespace indirect_map_strict {
	struct service1 {};
	struct service2 : service1 {};
	struct service3 { service3(service2&&) {} };
	
	struct indirect_map {
		template<typename T>
		using mapped_service = kgr::service<T>;
	};
	
	struct map1 {};
	struct map2 {};
	struct map3 {};
	struct map4 {};
	
	auto service_map(service1 const&, kgr::map_t<map1>) -> indirect_map;
	auto service_map(service1 const&, kgr::map_t<map2>) -> kgr::service<service2>;
	
	auto service_map(service2 const&, kgr::map_t<map3>) -> kgr::single_service<service2>;
	auto service_map(service1&&, kgr::map_t<map3>) -> indirect_map;
	
	auto service_map(service2 const&, kgr::map_t<map4>) -> kgr::single_service<service2>;
	auto service_map(service3, kgr::map_t<map4>) -> indirect_map;
	
	TEST_CASE("The indirect map is strict to the mapped type", "[service_map]") {
		REQUIRE_FALSE((kgr::detail::is_complete_map<kgr::map<map1>, service2>::value));
		REQUIRE((std::is_same<kgr::mapped_service_t<service2, kgr::map<map2>>, kgr::service<service2>>::value));
		
		CHECK((std::is_same<kgr::mapped_service_t<service2, kgr::map<map3>>, kgr::single_service<service2>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service2 const&, kgr::map<map3>>, kgr::single_service<service2>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service2&&, kgr::map<map3>>, kgr::single_service<service2>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service2 const&&, kgr::map<map3>>, kgr::single_service<service2>>::value));
		
		CHECK((std::is_same<kgr::mapped_service_t<service2, kgr::map<map4>>, kgr::single_service<service2>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service2 const&, kgr::map<map4>>, kgr::single_service<service2>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service2&&, kgr::map<map4>>, kgr::single_service<service2>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service2 const&&, kgr::map<map4>>, kgr::single_service<service2>>::value));
	}
}

namespace indirect_map_value_cat {
	struct service {};
	
	struct indirect_map {
		template<typename T>
		using mapped_service = kgr::service<typename std::decay<T>::type>;
	};
	
	struct single_indirect_map {
		template<typename T>
		using mapped_service = kgr::single_service<typename std::decay<T>::type>;
	};
	
	struct test1 {};
	struct test2 {};
	struct test3 {};
	struct test4 {};
	struct test5 {};
	
	auto service_map(service const&, kgr::map_t<test1>) -> single_indirect_map;
	auto service_map(service&&, kgr::map_t<test1>) -> indirect_map;
	
	auto service_map(service const&, kgr::map_t<test2>) -> single_indirect_map;
	
	auto service_map(service const&, kgr::map_t<test3>) -> single_indirect_map;
	auto service_map(service const&&, kgr::map_t<test3>) -> indirect_map;
	
	auto service_map(service &, kgr::map_t<test4>) -> single_indirect_map;
	auto service_map(service const&, kgr::map_t<test4>) -> indirect_map;
	
	auto service_map(service const&, kgr::map_t<test5>) -> kgr::single_service<service>;
	auto service_map(service const&&, kgr::map_t<test5>) -> indirect_map;
	
	TEST_CASE("The indirect map respect the value category of the mapping", "[service_map]") {
		CHECK((std::is_same<kgr::mapped_service_t<service, kgr::map<test1>>, kgr::service<service>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service const&, kgr::map<test1>>, kgr::single_service<service>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service&&, kgr::map<test1>>, kgr::service<service>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service&, kgr::map<test1>>, kgr::single_service<service>>::value));
		
		CHECK((std::is_same<kgr::mapped_service_t<service, kgr::map<test2>>, kgr::single_service<service>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service const&, kgr::map<test2>>, kgr::single_service<service>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service&&, kgr::map<test2>>, kgr::single_service<service>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service&, kgr::map<test2>>, kgr::single_service<service>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service const&&, kgr::map<test2>>, kgr::single_service<service>>::value));
		
		CHECK((std::is_same<kgr::mapped_service_t<service, kgr::map<test3>>, kgr::service<service>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service const&, kgr::map<test3>>, kgr::single_service<service>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service&&, kgr::map<test3>>, kgr::service<service>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service&, kgr::map<test3>>, kgr::single_service<service>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service const&&, kgr::map<test3>>, kgr::service<service>>::value));
		
		CHECK((std::is_same<kgr::mapped_service_t<service, kgr::map<test4>>, kgr::service<service>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service const&, kgr::map<test4>>, kgr::service<service>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service&&, kgr::map<test4>>, kgr::service<service>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service const&&, kgr::map<test4>>, kgr::service<service>>::value));

		// The following test cannot pass on visual studio. A bug will be reported about this behaviour
#ifndef _MSC_VER
		CHECK((std::is_same<kgr::mapped_service_t<service&, kgr::map<test4>>, kgr::single_service<service>>::value));
#endif
		
		CHECK((std::is_same<kgr::mapped_service_t<service, kgr::map<test5>>, kgr::service<service>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service const&, kgr::map<test5>>, kgr::single_service<service>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service&&, kgr::map<test5>>, kgr::service<service>>::value));
		CHECK((std::is_same<kgr::mapped_service_t<service const&&, kgr::map<test5>>, kgr::service<service>>::value));
	}
}
