#include <iostream>
#include <string>

#include "kangaru.hpp"

/**
 * This example explains advanced use of kangaru and it's components.
 * It covers invoke and autocall (injection by setters) through service map.
 */

using std::cout;
using std::endl;

// Here we are declaring two candy types
struct Caramel {
	Caramel() {
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
struct CaramelService : kgr::Service<Caramel> {};
struct GummyBearService : kgr::Service<GummyBear> {};

// We will need the service map
template<typename>
struct ServiceMap;

template<> struct ServiceMap<Caramel> : kgr::Map<CaramelService> {};
template<> struct ServiceMap<GummyBear> : kgr::Map<GummyBearService> {};

// CandyFactory, making candies and recepies
struct CandyFactory {
	CandyFactory(
		kgr::Generator<CaramelService> myCaramelGenerator,
		kgr::Generator<kgr::LazyService<GummyBearService>> myGummyBearGenerator,
		kgr::Invoker<ServiceMap> myInvoker
	) : caramelGenerator{myCaramelGenerator},
		gummyBearGenerator{myGummyBearGenerator},
		invoker{myInvoker} {}
	
	kgr::Lazy<GummyBearService> makeGummyBear() {
		return gummyBearGenerator();
	}
	
	Caramel makeCaramel() {
		return caramelGenerator();
	}
	
	template<typename T>
	void mix(T function) {
		invoker(function);
	}
	
private:
	kgr::Generator<CaramelService> caramelGenerator;
	kgr::Generator<kgr::LazyService<GummyBearService>> gummyBearGenerator;
	kgr::Invoker<ServiceMap> invoker;
};

struct CandyFactoryService : kgr::SingleService<CandyFactory, kgr::Dependency<
	kgr::GeneratorService<CaramelService>,
	kgr::GeneratorService<kgr::LazyService<GummyBearService>>,
	kgr::InvokerService<ServiceMap>
>> {};

// a recepie
void recepie(Caramel, GummyBear) {
	cout << "A sweet recepie mixing Caramel and GummyBear completed" << endl;
}

int main() {
	kgr::Container container;
	
	// We are making our factory
	auto candyFactory = container.service<CandyFactoryService>();
	
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
