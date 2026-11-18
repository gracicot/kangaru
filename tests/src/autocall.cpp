#include <catch2/catch_test_macros.hpp>
#include <kangaru/kangaru.hpp>
#include <kangaru/compatibility_v4.hpp>

#include "v4_test_helper.hpp"

#define METHOD(...) ::kgr::method<decltype(__VA_ARGS__), __VA_ARGS__>



TEST_CASE("Container call each function in autocall", "[autocall]") {
	SECTION("One method called") {
		struct Service {
			int called = 0;
			
			void function() {
				++called;
			}
		};
		
		struct Definition : kgr::service<Service>, kgr::autocall<METHOD(&Service::function)> {};
		
		REQUIRE(kgr::container{}.service<Definition>().called == 1);
	}
	
	SECTION("Multiple method called in order") {
		struct Service {
			int called1 = 0, called2 = 0, called3 = 0;
			
			void function1() {
				CHECK(called1 == 0);
				CHECK(called2 == 0);
				CHECK(called3 == 0);
				++called1;
			}
			
			void function2() {
				CHECK(called1 == 1);
				CHECK(called2 == 0);
				CHECK(called3 == 0);
				++called2;
			}
			
			void function3() {
				CHECK(called1 == 1);
				CHECK(called2 == 1);
				CHECK(called3 == 0);
				++called3;
			}
		};
		
		struct Definition : kgr::service<Service>, kgr::autocall<
			METHOD(&Service::function1), METHOD(&Service::function2), METHOD(&Service::function3)
		> {};
		
		auto service = kgr::container{}.service<Definition>();
		
		REQUIRE(service.called1 == 1);
		REQUIRE(service.called2 == 1);
		REQUIRE(service.called3 == 1);
	}
}

namespace testcase_autocall_many_function_type {

struct Service {
	int called = 0;

	void static static_function(Service& s) {
		++s.called;
	}
};

void nonmember_function(Service& s) {
	++s.called;
}

TEST_CASE("Container call each different function type", "[autocall]") {
	SECTION("Non member function called") {
		struct Definition : kgr::service<Service>, kgr::autocall<METHOD(nonmember_function)> {};
		
		REQUIRE(kgr::container{}.service<Definition>().called == 1);
	}
	
	SECTION("Non member function called") {
		struct Definition : kgr::service<Service>, kgr::autocall<METHOD(&Service::static_function)> {};
		
		REQUIRE(kgr::container{}.service<Definition>().called == 1);
	}
	
	SECTION("Non member function called") {
		struct BadDefinition : kgr::service<Service>, kgr::autocall<METHOD(&Service::called)> {};
		REQUIRE((!kgr::detail::is_valid_autocall_function<BadDefinition, METHOD(&Service::called)>::value));
		// Verifying this would introduce cycles
		//REQUIRE(!kgr::detail::is_service_valid<BadDefinition>::value);
	}
}
}

TEST_CASE("Container inject parameter in autocall function using an invoke call", "[autocall]") {
	static int injected_constructed = 0;
	
	struct InjectedService {
		InjectedService() {
			++injected_constructed;
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
	
	REQUIRE(kgr::container{}.service<Definition>().called == 1);
	REQUIRE(injected_constructed == 1);
}

namespace testcase_autocall_map {
	static int injected_constructed = 0;
	
	struct InjectedService {
		InjectedService() {
			++injected_constructed;
		}
	};
	
	struct Service {
		int called = 0;
		
		void function(InjectedService) {
			++called;
		}
	};
	
	struct InjectedDefinition : kgr::service<InjectedService> {};
	
	struct Definition : kgr::service<Service>, kgr::autocall<
		METHOD(&Service::function)
	> {};
	
	auto service_map(InjectedService const&) -> InjectedDefinition;
		
	TEST_CASE("Container inject parameter in autocall function using the service map", "[autocall]") {
		REQUIRE(kgr::container{}.service<Definition>().called == 1);
		REQUIRE(injected_constructed == 1);
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
		REQUIRE(kgr::container{}.service<Definition>().called == 1);
		REQUIRE(injected_constructed == 1);
	}
}

namespace testcase_autocall_custom_map_no_map {
	static int injected_constructed = 0;
	
	struct Map {};
	
	struct InjectedService {
		InjectedService() {
			++injected_constructed;
		}
	};
	
	struct Service {
		int called = 0;
		
		void function(InjectedService) {
			++called;
		}
	};
	
	struct InjectedDefinition : kgr::service<InjectedService> {};
	
	struct Definition : kgr::service<Service>, kgr::autocall<kgr::map<Map>,
		METHOD(&Service::function)
	> {};
	
	auto service_map(InjectedService const&) -> InjectedDefinition;
		
	TEST_CASE("Container inject parameter in autocall function using the service map with a custom map fallback to normal map", "[autocall]") {
		REQUIRE(kgr::container{}.service<Definition>().called == 1);
		REQUIRE(injected_constructed == 1);
	}
}

namespace testcase_autocall_invoke_call {
	static int injected_constructed = 0;

	struct InjectedService {
		InjectedService() {
			++injected_constructed;
		}
	};

	struct Service {
		int called_member = 0;
		inline static int called_static = 0;

		void function(InjectedService) {
			++called_member;
		}

		static void static_function(Service&, InjectedService) {
			++called_static;
		}
	};
	
	struct InjectedDefinition : kgr::service<InjectedService> {};

	struct Definition : kgr::service<Service>, kgr::autocall<
		kgr::invoke<METHOD(&Service::function), InjectedDefinition>,
		kgr::invoke<METHOD(&Service::static_function), InjectedDefinition>
	> {};

	TEST_CASE("Container inject parameter in autocall function using the specified service in invoke", "[autocall]") {
		auto&& s = kgr::container{}.service<Definition>();
		REQUIRE(s.called_member == 1);
		REQUIRE(s.called_static == 1);
		REQUIRE(injected_constructed == 2);
	}
}

namespace testcase_autocall_invoke_call_multiple {
	static int injected_constructed = 0;
	static int injected_definition1_constructed = 0;
	static int injected_definition2_constructed = 0;

	struct InjectedService {
		InjectedService() {
			++injected_constructed;
		}
	};

	struct Service {
		int called = 0;

		void function(InjectedService, InjectedService) {
			++called;
		}
	};

	struct InjectedDefinition1 : kgr::service<InjectedService> { InjectedDefinition1() { ++injected_definition1_constructed; } };
	struct InjectedDefinition2 : kgr::service<InjectedService> { InjectedDefinition2() { ++injected_definition2_constructed; } };

	struct Definition : kgr::service<Service>, kgr::autocall<
		kgr::invoke<METHOD(&Service::function), InjectedDefinition1, InjectedDefinition2>
	> {};

	TEST_CASE("Container inject multiple parameter in autocall function using the specified service in invoke", "[autocall]") {
		REQUIRE(kgr::container{}.service<Definition>().called == 1);
		REQUIRE(injected_constructed == 2);
		REQUIRE(injected_definition1_constructed == 1);
		REQUIRE(injected_definition2_constructed == 1);
	}
}
