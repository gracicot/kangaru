#include <catch2/catch_test_macros.hpp>
#include <kangaru-prev/kangaru.hpp>

TEST_CASE("Definition can override another", "[virtual]") {
	SECTION("Bases are instanciated, but not used after instanciate the derived") {
		struct ServiceBase {};
		struct ServiceDerived : ServiceBase {};
		
		static int base_forward_called = 0;
		static int derived_forward_called = 0;
		static bool base_instanciated = false;
		
		struct DefinitionBase : kgr::single_service<ServiceBase>, kgr::polymorphic {
			DefinitionBase() {
				base_instanciated = true;
			}
			
			ServiceBase forward() {
				base_forward_called++;
				return single_service::forward();
			}
		};
		
		struct DefinitionDerived : kgr::single_service<ServiceDerived>, kgr::overrides<DefinitionBase> {
			ServiceDerived forward() {
				derived_forward_called++;
				return single_service::forward();
			}
		};
		
		kgr::container c;
		
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
		
		struct DefinitionBase : kgr::single_service<ServiceBase>, kgr::polymorphic {
			DefinitionBase() {
				base_instanciated = true;
			}
		};
		
		struct DefinitionDerived : kgr::single_service<ServiceDerived>, kgr::overrides<DefinitionBase> {};
		
		kgr::container c;
		
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
		
		struct DefinitionBase1 : kgr::single_service<ServiceBase1>, kgr::polymorphic {
			DefinitionBase1() {
				base1_instanciated = true;
			}
			
			ServiceBase1 forward() {
				base1_forward_called++;
				return single_service::forward();
			}
		};
		
		struct DefinitionBase2 : kgr::single_service<ServiceBase2>, kgr::polymorphic {
			DefinitionBase2() {
				base2_instanciated = true;
			}
			
			ServiceBase2 forward() {
				base2_forward_called++;
				return single_service::forward();
			}
		};
		
		struct DefinitionDerived : kgr::single_service<ServiceDerived>, kgr::overrides<DefinitionBase1, DefinitionBase2> {
			ServiceDerived forward() {
				derived_forward_called++;
				return single_service::forward();
			}
		};
		
		kgr::container c;
		
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
		
		struct DefinitionBase1 : kgr::single_service<ServiceBase1>, kgr::polymorphic {
			DefinitionBase1() {
				base1_instanciated = true;
			}
		};
		
		
		struct DefinitionBase2 : kgr::single_service<ServiceBase2>, kgr::polymorphic {
			DefinitionBase2() {
				base2_instanciated = true;
			}
		};
		
		struct DefinitionDerived : kgr::single_service<ServiceDerived>, kgr::overrides<DefinitionBase1, DefinitionBase2> {};
		
		kgr::container c;
		
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
		
		struct DefinitionBase1 : kgr::single_service<ServiceBase1>, kgr::polymorphic {
			DefinitionBase1() {
				base1_instanciated = true;
			}
		};
		
		
		struct DefinitionBase2 : kgr::single_service<ServiceBase2>, kgr::polymorphic {
			DefinitionBase2() {
				base2_instanciated = true;
			}
		};
		
		struct DefinitionDerived : kgr::single_service<ServiceDerived>, kgr::overrides<DefinitionBase1, DefinitionBase2> {};
		
		kgr::container c;
		
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
	
	struct DefinitionBase : kgr::single_service<ServiceBase>, kgr::polymorphic {
		DefinitionBase() {
			base_instanciated = true;
		}
		
		ServiceBase& forward() {
			base_forward_called++;
			return single_service::forward();
		}
	};
	
	struct DefinitionMiddle1 : kgr::single_service<ServiceMiddle1>, kgr::overrides<DefinitionBase> {
		DefinitionMiddle1() {
			middle1_instanciated = true;
		}
		
		ServiceMiddle1& forward() {
			middle1_forward_called++;
			return single_service::forward();
		}
	};
	
	struct DefinitionMiddle2 : kgr::single_service<ServiceMiddle2>, kgr::overrides<DefinitionBase> {
		DefinitionMiddle2() {
			middle2_instanciated = true;
		}
		
		ServiceMiddle2& forward() {
			middle2_forward_called++;
			return single_service::forward();
		}
	};
	
	struct DefinitionDerived : kgr::single_service<ServiceDerived>, kgr::overrides<DefinitionMiddle1, DefinitionMiddle2> {
		DefinitionDerived() {
			derived_instanciated = true;
		}
		
		ServiceDerived& forward() {
			derived_forward_called++;
			return single_service::forward();
		}
	};
	
	kgr::container c;
	
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

TEST_CASE("Abtract Services Are virtual", "[virtual]") {
	kgr::container container;
	struct A {};
	
	struct AbstractDefinition : kgr::abstract_service<A> {};
	
	REQUIRE(kgr::detail::is_polymorphic<AbstractDefinition>::value);
	
	CHECK_THROWS_AS(container.service<AbstractDefinition>(), kgr::abstract_not_found);
	struct Override1Definition : kgr::single_service<A>, kgr::overrides<AbstractDefinition> {};
	
	container.emplace<Override1Definition>();
	
	REQUIRE(&container.service<AbstractDefinition>() == &container.service<AbstractDefinition>());
}

TEST_CASE("A service can depend on a abstract service", "[virtual]") {
	kgr::container container;
	struct A {
		virtual ~A() = default;
		virtual void a() = 0;
	};
	
	struct Impl : A { void a() override {} };
	
	struct Use {
		explicit Use(A const& a_) : a(a_) {}
		A const& a;
	};
	
	struct AbstractService : kgr::abstract_service<A> {};
	struct ImplService : kgr::single_service<Impl>, kgr::overrides<AbstractService> {};
	struct UseService : kgr::service<Use, kgr::dependency<AbstractService>> {};
	
	REQUIRE(kgr::detail::is_polymorphic<AbstractService>::value);
	REQUIRE(kgr::detail::is_polymorphic<ImplService>::value);
	REQUIRE(!kgr::detail::is_polymorphic<UseService>::value);
	REQUIRE(kgr::detail::is_service_valid<UseService>::value);
	
	container.emplace<ImplService>();
	
	REQUIRE(&container.service<UseService>().a == &container.service<AbstractService>());
}
