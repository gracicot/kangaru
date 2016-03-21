#include <iostream>
#include <string>

#include "kangaru.hpp"

/**
 * This example explains advanced use of kangaru and it's components.
 * It covers invoke and autocall (injection by setters) through service map.
 */

using std::cout;
using std::endl;

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

struct CaramelService : kgr::Service<Caramel> {};
struct GummyBearService : kgr::Service<GummyBear> {};

template<typename>
struct ServiceMap;

template<> struct ServiceMap<Caramel> : kgr::Map<CaramelService> {};
template<> struct ServiceMap<GummyBear> : kgr::Map<GummyBearService> {};

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

void recepie(Caramel, GummyBear) {
	cout << "A sweet recepie mixing Caramel and GummyBear completed" << endl;
}

int main() {
	kgr::Container container;
	auto candyFactory = container.service<CandyFactoryService>();
	
	auto lazyGummyBear = candyFactory.makeGummyBear();
	auto caramel = candyFactory.makeCaramel();
	
	lazyGummyBear->eat();
	caramel.eat();
	
	cout << endl << "== Let's make a recepie ==" << endl;
	
	candyFactory.mix(recepie);
	
	return 0;
}
