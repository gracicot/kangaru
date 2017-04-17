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
struct SugarService : kgr::SingleService<Sugar> {};
struct CaramelService : kgr::Service<Caramel, kgr::Dependency<SugarService>> {};
struct GummyBearService : kgr::Service<GummyBear> {};

// We map our services
auto service_map(Caramel) -> CaramelService;
auto service_map(GummyBear) -> GummyBearService;

// CandyFactory, making candies and recepies
struct CandyFactory {
	CandyFactory(
		kgr::Generator<CaramelService> myCaramelGenerator,
		kgr::Generator<kgr::LazyService<GummyBearService>> myGummyBearGenerator,
		kgr::DefaultInvoker myInvoker
	) : caramelGenerator{myCaramelGenerator},
		gummyBearGenerator{myGummyBearGenerator},
		invoker{myInvoker} {}
	
	kgr::Lazy<GummyBearService> makeGummyBear() {
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
	kgr::Generator<CaramelService> caramelGenerator;
	kgr::Generator<kgr::LazyService<GummyBearService>> gummyBearGenerator;
	kgr::DefaultInvoker invoker;
};

struct CandyFactoryService : kgr::SingleService<CandyFactory, kgr::Dependency<
	kgr::GeneratorService<CaramelService>,
	kgr::GeneratorService<kgr::LazyService<GummyBearService>>,
	kgr::DefaultInvokerService
>> {};

// a recepie
void recepie(Caramel, GummyBear) {
	cout << "A sweet recepie mixing Caramel and GummyBear completed" << endl;
}

int main() {
	kgr::Container container;
	
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
	
	return 0;
}
