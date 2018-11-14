#include <catch2/catch.hpp>
#include <kangaru/kangaru.hpp>

#define METHOD(...) ::kgr::method<decltype(__VA_ARGS__), __VA_ARGS__>



TEST_CASE("Container call each function in autocall", "[autocall]") {
	SECTION("One method called") {
		struct Service {
			bool called = false;

			void function() {
				CHECK(!called);
				called = true;
			}
		};

		struct Definition : kgr::service<Service>, kgr::autocall<METHOD(&Service::function)> {};

		REQUIRE(kgr::container{}.service<Definition>().called);
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
		
		auto service = kgr::container{}.service<Definition>();
		
		REQUIRE(service.called1);
		REQUIRE(service.called2);
		REQUIRE(service.called3);
	}
}

namespace testcase_autocall_many_function_type {

struct Service {
	bool called = false;

	void static static_function(Service& s) {
		s.called = true;
	}
};

void nonmember_function(Service& s) {
	s.called = true;
}

TEST_CASE("Container call each different function type", "[autocall]") {
	SECTION("Non member function called") {
		struct Definition : kgr::service<Service>, kgr::autocall<METHOD(nonmember_function)> {};

		REQUIRE(kgr::container{}.service<Definition>().called);
	}

	SECTION("Non member function called") {
		struct Definition : kgr::service<Service>, kgr::autocall<METHOD(&Service::static_function)> {};

		REQUIRE(kgr::container{}.service<Definition>().called);
	}

	SECTION("Non member function called") {
		struct BadDefinition : kgr::service<Service>, kgr::autocall<METHOD(&Service::called)> {};

		REQUIRE((!kgr::detail::is_valid_autocall_function<BadDefinition, METHOD(&Service::called)>::value));
		REQUIRE(!kgr::detail::is_service_valid<BadDefinition>::value);
	}
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
	
	REQUIRE(kgr::container{}.service<Definition>().called);
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
		REQUIRE(kgr::container{}.service<Definition>().called);
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
		REQUIRE(kgr::container{}.service<Definition>().called);
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
		REQUIRE(kgr::container{}.service<Definition>().called);
		REQUIRE(injected_constructed);
	}
}

namespace testcase_autocall_invoke_call {
	static bool injected_constructed = false;

	struct InjectedService {
		InjectedService() {
			injected_constructed = true;
		}
	};

	struct Service {
		bool called_member = false;
		static bool called_static;

		void function(InjectedService) {
			called_member = true;
		}

		static void static_function(Service&, InjectedService) {
			called_static = true;
		}
	};
	
	bool Service::called_static = false;

	struct InjectedDefinition : kgr::service<InjectedService> {};

	struct Definition : kgr::service<Service>, kgr::autocall<
		kgr::invoke<METHOD(&Service::function), InjectedDefinition>,
		kgr::invoke<METHOD(&Service::static_function), InjectedDefinition>
	> {};

	TEST_CASE("Container inject parameter in autocall function using the specified service in invoke", "[autocall]") {
		auto&& s = kgr::container{}.service<Definition>();
		REQUIRE(s.called_member);
		REQUIRE(s.called_static);
		REQUIRE(injected_constructed);
	}
}

namespace testcase_autocall_invoke_call_multiple {
	static bool injected_constructed = false;
	static bool injected_definition1_constructed = false;
	static bool injected_definition2_constructed = false;

	struct InjectedService {
		InjectedService() {
			injected_constructed = true;
		}
	};

	struct Service {
		bool called = false;

		void function(InjectedService, InjectedService) {
			called = true;
		}
	};

	struct InjectedDefinition1 : kgr::service<InjectedService> { InjectedDefinition1() { injected_definition1_constructed = true; } };
	struct InjectedDefinition2 : kgr::service<InjectedService> { InjectedDefinition2() { injected_definition2_constructed = true; } };

	struct Definition : kgr::service<Service>, kgr::autocall<
		kgr::invoke<METHOD(&Service::function), InjectedDefinition1, InjectedDefinition2>
	> {};

	TEST_CASE("Container inject multiple parameter in autocall function using the specified service in invoke", "[autocall]") {
		REQUIRE(kgr::container{}.service<Definition>().called);
		REQUIRE(injected_constructed);
		REQUIRE(injected_definition1_constructed);
		REQUIRE(injected_definition2_constructed);
	}
}
