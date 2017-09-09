#define CATCH_CONFIG_MAIN
#include "catch.hpp"
#include "kangaru/kangaru.hpp"

TEST_CASE("Definition can override another", "[virtual]") {
	SECTION("Bases are instanciated, but not used after instanciate the derived") {
		struct ServiceBase {};
		struct ServiceDerived : ServiceBase {};
		
		static int base_forward_called = 0;
		static int derived_forward_called = 0;
		static bool base_instanciated = false;
		
		struct DefinitionBase : kgr::SingleService<ServiceBase>, kgr::Virtual {
			DefinitionBase() {
				base_instanciated = true;
			}
			
			ServiceBase forward() {
				base_forward_called++;
				return SingleService::forward();
			}
		};
		
		struct DefinitionDerived : kgr::SingleService<ServiceDerived>, kgr::Overrides<DefinitionBase> {
			ServiceDerived forward() {
				derived_forward_called++;
				return SingleService::forward();
			}
		};
		
		kgr::Container c;
		
		(void) c.service<DefinitionBase>();
		(void) c.service<DefinitionDerived>();
		(void) c.service<DefinitionDerived>();
		
		REQUIRE(base_instanciated);
		REQUIRE(base_forward_called == 1);
		REQUIRE(derived_forward_called == 2);
	}
	
	SECTION("Bases are not instanciated when an override already exists") {
		struct ServiceBase {};
		struct ServiceDerived : ServiceBase {};
		
		static bool base_instanciated = false;
		
		struct DefinitionBase : kgr::SingleService<ServiceBase>, kgr::Virtual {
			DefinitionBase() {
				base_instanciated = true;
			}
		};
		
		struct DefinitionDerived : kgr::SingleService<ServiceDerived>, kgr::Overrides<DefinitionBase> {};
		
		kgr::Container c;
		
		(void) c.service<DefinitionDerived>();
		(void) c.service<DefinitionBase>();
		
		REQUIRE_FALSE(base_instanciated);
	}
}

TEST_CASE("Definition can override multiple definition", "[virtual]") {
	SECTION("Bases are instanciated, but not used after instanciate the derived") {
		struct ServiceBase1 {};
		struct ServiceBase2 {};
		struct ServiceDerived : ServiceBase1, ServiceBase2 {};
		
		static int base1_forward_called = 0;
		static int base2_forward_called = 0;
		static int derived_forward_called = 0;
		static bool base1_instanciated = false;
		static bool base2_instanciated = false;
		
		struct DefinitionBase1 : kgr::SingleService<ServiceBase1>, kgr::Virtual {
			DefinitionBase1() {
				base1_instanciated = true;
			}
			
			ServiceBase1 forward() {
				base1_forward_called++;
				return SingleService::forward();
			}
		};
		
		struct DefinitionBase2 : kgr::SingleService<ServiceBase2>, kgr::Virtual {
			DefinitionBase2() {
				base2_instanciated = true;
			}
			
			ServiceBase2 forward() {
				base2_forward_called++;
				return SingleService::forward();
			}
		};
		
		struct DefinitionDerived : kgr::SingleService<ServiceDerived>, kgr::Overrides<DefinitionBase1, DefinitionBase2> {
			ServiceDerived forward() {
				derived_forward_called++;
				return SingleService::forward();
			}
		};
		
		kgr::Container c;
		
		(void) c.service<DefinitionBase1>();
		(void) c.service<DefinitionBase2>();
		(void) c.service<DefinitionDerived>();
		(void) c.service<DefinitionDerived>();
		
		REQUIRE(base1_instanciated);
		REQUIRE(base2_instanciated);
		REQUIRE(base1_forward_called == 1);
		REQUIRE(base2_forward_called == 1);
		REQUIRE(derived_forward_called == 2);
	}
	
	SECTION("Bases are not instanciated when an override already exists partial") {
		struct ServiceBase1 {};
		struct ServiceBase2 {};
		struct ServiceDerived : ServiceBase1, ServiceBase2 {};
		
		static bool base1_instanciated = false;
		static bool base2_instanciated = false;
		
		struct DefinitionBase1 : kgr::SingleService<ServiceBase1>, kgr::Virtual {
			DefinitionBase1() {
				base1_instanciated = true;
			}
		};
		
		
		struct DefinitionBase2 : kgr::SingleService<ServiceBase2>, kgr::Virtual {
			DefinitionBase2() {
				base2_instanciated = true;
			}
		};
		
		struct DefinitionDerived : kgr::SingleService<ServiceDerived>, kgr::Overrides<DefinitionBase1, DefinitionBase2> {};
		
		kgr::Container c;
		
		(void) c.service<DefinitionBase1>();
		(void) c.service<DefinitionDerived>();
		(void) c.service<DefinitionBase2>();
		
		REQUIRE(base1_instanciated);
		REQUIRE_FALSE(base2_instanciated);
	}
	
	SECTION("Bases are not instanciated when an override already exists total") {
		struct ServiceBase1 {};
		struct ServiceBase2 {};
		struct ServiceDerived : ServiceBase1, ServiceBase2 {};
		
		static bool base1_instanciated = false;
		static bool base2_instanciated = false;
		
		struct DefinitionBase1 : kgr::SingleService<ServiceBase1>, kgr::Virtual {
			DefinitionBase1() {
				base1_instanciated = true;
			}
		};
		
		
		struct DefinitionBase2 : kgr::SingleService<ServiceBase2>, kgr::Virtual {
			DefinitionBase2() {
				base2_instanciated = true;
			}
		};
		
		struct DefinitionDerived : kgr::SingleService<ServiceDerived>, kgr::Overrides<DefinitionBase1, DefinitionBase2> {};
		
		kgr::Container c;
		
		(void) c.service<DefinitionDerived>();
		(void) c.service<DefinitionBase1>();
		(void) c.service<DefinitionBase2>();
		
		REQUIRE_FALSE(base1_instanciated);
		REQUIRE_FALSE(base2_instanciated);
	}
}

TEST_CASE("Definition can override with multiple level and virtual inheritance", "[virtual]") {
	static bool base_instanciated;
	static bool middle1_instanciated;
	static bool middle2_instanciated;
	static bool derived_instanciated;
	
	static int base_forward_called;
	static int middle1_forward_called;
	static int middle2_forward_called;
	static int derived_forward_called;
	
	base_instanciated = false;
	middle1_instanciated = false;
	middle2_instanciated = false;
	derived_instanciated = false;
	
	base_forward_called = 0;
	middle1_forward_called = 0;
	middle2_forward_called = 0;
	derived_forward_called = 0;
	
	struct ServiceBase {};
	struct ServiceMiddle1 : virtual ServiceBase {};
	struct ServiceMiddle2 : virtual ServiceBase {};
	struct ServiceDerived : ServiceMiddle1, ServiceMiddle2 {};
	
	struct DefinitionBase : kgr::SingleService<ServiceBase>, kgr::Virtual {
		DefinitionBase() {
			base_instanciated = true;
		}
		
		ServiceBase& forward() {
			base_forward_called++;
			return SingleService::forward();
		}
	};
	
	struct DefinitionMiddle1 : kgr::SingleService<ServiceMiddle1>, kgr::Overrides<DefinitionBase> {
		DefinitionMiddle1() {
			middle1_instanciated = true;
		}
		
		ServiceMiddle1& forward() {
			middle1_forward_called++;
			return SingleService::forward();
		}
	};
	
	struct DefinitionMiddle2 : kgr::SingleService<ServiceMiddle2>, kgr::Overrides<DefinitionBase> {
		DefinitionMiddle2() {
			middle2_instanciated = true;
		}
		
		ServiceMiddle2& forward() {
			middle2_forward_called++;
			return SingleService::forward();
		}
	};
	
	struct DefinitionDerived : kgr::SingleService<ServiceDerived>, kgr::Overrides<DefinitionMiddle1, DefinitionMiddle2> {
		DefinitionDerived() {
			derived_instanciated = true;
		}
		
		ServiceDerived& forward() {
			derived_forward_called++;
			return SingleService::forward();
		}
	};
	
	kgr::Container c;
	
	SECTION("When derived is instanciated, overriden services are not") {
		(void) c.service<DefinitionDerived>();
		(void) c.service<DefinitionMiddle1>();
		(void) c.service<DefinitionMiddle2>();
		
		REQUIRE(derived_instanciated);
		REQUIRE_FALSE(base_instanciated);
		REQUIRE_FALSE(middle1_instanciated);
		REQUIRE_FALSE(middle2_instanciated);
		
		REQUIRE(base_forward_called == 0);
		REQUIRE(middle1_forward_called == 0);
		REQUIRE(middle2_forward_called == 0);
		REQUIRE(derived_forward_called == 3);
	}
	
	SECTION("Derived can override later") {
		(void) c.service<DefinitionMiddle1>();
		(void) c.service<DefinitionDerived>();
		(void) c.service<DefinitionMiddle2>();
		(void) c.service<DefinitionMiddle1>();
		
		REQUIRE(derived_instanciated);
		REQUIRE_FALSE(base_instanciated);
		REQUIRE(middle1_instanciated);
		REQUIRE_FALSE(middle2_instanciated);
		
		REQUIRE(base_forward_called == 0);
		REQUIRE(middle1_forward_called == 1);
		REQUIRE(middle2_forward_called == 0);
		REQUIRE(derived_forward_called == 3);
	}
	
	SECTION("Overriden services can still be invoked by calling services it's overriding") {
		(void) c.service<DefinitionMiddle1>();
		
		REQUIRE(middle1_forward_called == 1);
		
		(void) c.service<DefinitionDerived>();
		(void) c.service<DefinitionMiddle2>();
		(void) c.service<DefinitionMiddle1>();
		(void) c.service<DefinitionBase>();
		
		REQUIRE(derived_instanciated);
		REQUIRE_FALSE(base_instanciated);
		REQUIRE(middle1_instanciated);
		REQUIRE_FALSE(middle2_instanciated);
		
		REQUIRE(base_forward_called == 0);
		REQUIRE(middle1_forward_called == 2);
		REQUIRE(middle2_forward_called == 0);
		REQUIRE(derived_forward_called == 3);
	}
	
	SECTION("Base is still called if all overriding definitions are overriden") {
		(void) c.service<DefinitionBase>();
		(void) c.service<DefinitionDerived>();
		(void) c.service<DefinitionMiddle2>();
		(void) c.service<DefinitionMiddle1>();
		
		REQUIRE(derived_instanciated);
		REQUIRE(base_instanciated);
		REQUIRE_FALSE(middle1_instanciated);
		REQUIRE_FALSE(middle2_instanciated);
		
		REQUIRE(base_forward_called == 1);
		REQUIRE(middle1_forward_called == 0);
		REQUIRE(middle2_forward_called == 0);
		REQUIRE(derived_forward_called == 3);
	}
	
	SECTION("Instance returned by the container is the same") {
		(void) c.service<DefinitionDerived>();
		
		REQUIRE(derived_instanciated);
		REQUIRE_FALSE(base_instanciated);
		REQUIRE_FALSE(middle1_instanciated);
		REQUIRE_FALSE(middle2_instanciated);
		
		REQUIRE(base_forward_called == 0);
		REQUIRE(middle1_forward_called == 0);
		REQUIRE(middle2_forward_called == 0);
		REQUIRE(derived_forward_called == 1);
		
		REQUIRE(static_cast<ServiceMiddle1*>(&c.service<DefinitionDerived>()) == &c.service<DefinitionMiddle1>());
		REQUIRE(static_cast<ServiceMiddle2*>(&c.service<DefinitionDerived>()) == &c.service<DefinitionMiddle2>());
	}
}
