#include <iostream>
#include <string>

#include <kangaru/kangaru.hpp>

/**
 * This example explains advanced use of kangaru and it's components.
 * It covers invoker, lazy and generator.
 */

using std::cout;
using std::endl;

struct Sugar {
	int quantity = 100;
};

// Here we are declaring two candy types
struct Caramel {
	// we need sugar
	Caramel(Sugar& s) {
		s.quantity -= 10;
		cout << "Caramel made" << endl;
	}
	
	void eat() {
		cout << "yummy! Caramel!" << endl;
	}
};

struct GummyBear {
	GummyBear() {
		cout << "GummyBear made" << endl;
	}
	
	void eat() {
		cout << "yummy! GummyBear!" << endl;
	}
};

// Then, we are declaring two service definition for them
struct SugarService : kgr::single_service<Sugar> {};
struct CaramelService : kgr::service<Caramel, kgr::dependency<SugarService>> {};
struct GummyBearService : kgr::service<GummyBear> {};

// We map our services
auto service_map(Caramel) -> CaramelService;
auto service_map(GummyBear) -> GummyBearService;

// CandyFactory, making candies and recepies
struct CandyFactory {
	CandyFactory(
		kgr::generator<CaramelService> myCaramelGenerator,
		kgr::generator<kgr::lazy_service<GummyBearService>> myGummyBearGenerator,
		kgr::invoker myInvoker
	) : caramelGenerator{myCaramelGenerator},
		gummyBearGenerator{myGummyBearGenerator},
		invoker{myInvoker} {}
	
	kgr::lazy<GummyBearService> makeGummyBear() {
		// this line is making a new GummyBear with it's dependencies injected
		return gummyBearGenerator();
	}
	
	Caramel makeCaramel() {
		// this line is making a new Caramel with it's dependencies injected
		return caramelGenerator();
	}
	
	template<typename T>
	void mix(T function) {
		// calls the function sent as parameter
		invoker(function);
	}
	
private:
	kgr::generator<CaramelService> caramelGenerator;
	kgr::generator<kgr::lazy_service<GummyBearService>> gummyBearGenerator;
	kgr::invoker invoker;
};

struct CandyFactoryService : kgr::single_service<CandyFactory, kgr::dependency<
	kgr::generator_service<CaramelService>,
	kgr::generator_service<kgr::lazy_service<GummyBearService>>,
	kgr::invoker_service
>> {};

// a recepie
void recepie(Caramel, GummyBear) {
	cout << "A sweet recepie mixing Caramel and GummyBear completed" << endl;
}

int main() {
	kgr::container container;
	
	// We are making our factory
	auto& candyFactory = container.service<CandyFactoryService>();
	
	// this will print nothing, as there is no candy constructed
	auto lazyGummyBear = candyFactory.makeGummyBear();
	
	// this will print "Caramel made"
	auto caramel = candyFactory.makeCaramel();
	
	// This will print both "GummyBear made" and "yummy! GummyBear!"
	lazyGummyBear->eat();
	
	// This will print "yummy! Caramel!"
	caramel.eat();
	
	cout << endl << "== Let's make a recepie ==" << endl;
	
	// As there are new candy made, this will print:
	// > Caramel made
	// > GummyBear made
	// > A sweet recepie mixing Caramel and GummyBear completed
	candyFactory.mix(recepie);
}
