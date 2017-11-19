#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "kangaru/kangaru.hpp"

#define METHOD(...) ::kgr::method<decltype(__VA_ARGS__), __VA_ARGS__>

TEST_CASE("Container all each function in autocall", "[autocall]") {
	SECTION("One method called") {
		struct Service {
			bool called = false;
			
			void function() {
				CHECK(!called);
				called = true;
			}
		};
		
		struct Definition : kgr::service<Service>, kgr::autocall<METHOD(&Service::function)> {};
		
		REQUIRE(kgr::container {}.service<Definition>().called);
	}
	
	SECTION("Multiple method called in order") {
		struct Service {
			bool called1 = false, called2 = false, called3 = false;
			
			void function1() {
				CHECK(!called1);
				CHECK(!called2);
				CHECK(!called3);
				called1 = true;
			}
			
			void function2() {
				CHECK(called1);
				CHECK(!called2);
				CHECK(!called3);
				called2 = true;
			}
			
			void function3() {
				CHECK(called1);
				CHECK(called2);
				CHECK(!called3);
				called3 = true;
			}
		};
		
		struct Definition : kgr::service<Service>, kgr::autocall<
			METHOD(&Service::function1), METHOD(&Service::function2), METHOD(&Service::function3)
		> {};
		
		auto service = kgr::container {}.service<Definition>();
		
		REQUIRE(service.called1);
		REQUIRE(service.called2);
		REQUIRE(service.called3);
	}
}

TEST_CASE("Container inject parameter in autocall function using an invoke call", "[autocall]") {
	static bool injected_constructed = false;
	
	struct InjectedService {
		InjectedService() {
			injected_constructed = true;
		}
	};
	
	struct Service {
		bool called = false;
		
		void function(InjectedService) {
			called = true;
		}
	};
	
	struct InjectedDefinition : kgr::service<InjectedService> {};
	struct Definition : kgr::service<Service>, kgr::autocall<
		kgr::invoke<METHOD(&Service::function), InjectedDefinition>
	> {};
	
	REQUIRE(kgr::container {}.service<Definition>().called);
	REQUIRE(injected_constructed);
}

namespace testcase_autocall_map {
	static bool injected_constructed = false;
	
	struct InjectedService {
		InjectedService() {
			injected_constructed = true;
		}
	};
	
	struct Service {
		bool called = false;
		
		void function(InjectedService) {
			called = true;
		}
	};
	
	struct InjectedDefinition : kgr::service<InjectedService> {};
	
	struct Definition : kgr::service<Service>, kgr::autocall<
		METHOD(&Service::function)
	> {};
	
	auto service_map(InjectedService const&) -> InjectedDefinition;
		
	TEST_CASE("Container inject parameter in autocall function using the service map", "[autocall]") {
		REQUIRE(kgr::container {}.service<Definition>().called);
		REQUIRE(injected_constructed);
	}
}

namespace testcase_autocall_custom_map {
	static bool injected_constructed = false;
	
	struct Map {};
	
	struct InjectedService {
		InjectedService() {
			injected_constructed = true;
		}
	};
	
	struct Service {
		bool called = false;
		
		void function(InjectedService) {
			called = true;
		}
	};
	
	struct InjectedDefinition : kgr::service<InjectedService> {};
	
	struct Definition : kgr::service<Service>, kgr::autocall<kgr::map<Map>,
		METHOD(&Service::function)
	> {};
	
	auto service_map(InjectedService const&, kgr::map_t<Map>) -> InjectedDefinition;
		
	TEST_CASE("Container inject parameter in autocall function using the service map with a custom map", "[autocall]") {
		REQUIRE(kgr::container {}.service<Definition>().called);
		REQUIRE(injected_constructed);
	}
}


namespace testcase_autocall_custom_map_no_map {
	static bool injected_constructed = false;
	
	struct Map {};
	
	struct InjectedService {
		InjectedService() {
			injected_constructed = true;
		}
	};
	
	struct Service {
		bool called = false;
		
		void function(InjectedService) {
			called = true;
		}
	};
	
	struct InjectedDefinition : kgr::service<InjectedService> {};
	
	struct Definition : kgr::service<Service>, kgr::autocall<kgr::map<Map>,
		METHOD(&Service::function)
	> {};
	
	auto service_map(InjectedService const&) -> InjectedDefinition;
		
	TEST_CASE("Container inject parameter in autocall function using the service map with a custom map fallback to normal map", "[autocall]") {
		REQUIRE(kgr::container {}.service<Definition>().called);
		REQUIRE(injected_constructed);
	}
}
