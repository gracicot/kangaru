#include <iostream>
#include <string>
#include <memory>
#include <tuple>

#include "kangaru.hpp"

/**
 * This example explains advanced use of kangaru and it's components.
 * It covers custom service definition
 */

using std::string;
using std::cout;
using std::endl;
using std::move;

struct FlourBag {
	int quantity = 99;
};

struct Bakery;

struct Oven {
	void bake() {
		cout << "Bread baking..." << endl;
	}
};

struct Baker {
	Baker(std::unique_ptr<FlourBag> myFlour) : flour{move(myFlour)} {}
	void makeBread(Bakery& b, Oven* o);
	
private:
	std::unique_ptr<FlourBag> flour;
};

struct Bakery {
	Bakery(Oven* myOven) : oven{myOven} {}
	
	void start(std::shared_ptr<Baker> baker) {
		baker->makeBread(*this, oven);
	}
	
	string name;
	
private:
	Oven* oven;
};

void Baker::makeBread(Bakery& b, Oven* o) {
	flour->quantity -= 8;
	o->bake();
	cout << "Bread baked at the \"" << b.name << "\" bakery. " << flour->quantity << " cups of flour left." << endl;
}

/**
 * Service Definitions
 */

// FlourBag service definition. 
struct FlourBagService {
	// constructor. Receives a fully constructed flour bag to contain.
	template<typename... Args>
	FlourBagService(kgr::in_place_t, Args&&... args) : flour{std::forward<Args>(args)...} {}
	
	// construct method. Receives dependencies of a FlourBag. In this case none.
	static auto construct() -> decltype(kgr::inject(std::unique_ptr<FlourBag>{})) {
		return kgr::inject(std::unique_ptr<FlourBag>{new FlourBag});
	}
	
	// forward method. This method define how the service is injected.
	std::unique_ptr<FlourBag> forward() {
		return move(flour);
	}
	
private:
	std::unique_ptr<FlourBag> flour;
};

// Oven service definition. 
struct OvenService : kgr::Single {
	// constructor. Receives a fully constructed oven to contain.
	template<typename... Args>
	OvenService(kgr::in_place_t, Args&&... args) : oven{new Oven{std::forward<Args>(args)...}} {}
	
	// construct method. Receives dependencies of a Oven. In this case none.
	static auto construct() -> decltype(kgr::inject()) {
		return kgr::inject();
	}
	
	// forward method. This method define how the service is injected.
	// It's virtual because other services can override this one.
	// See example 4 for more detail.
	virtual Oven* forward() {
		return oven.get();
	}
	
private:
	std::unique_ptr<Oven> oven;
};

// Baker service definition. 
struct BakerService : kgr::Single {
	// constructor. Receives a fully constructed baker to contain.
	template<typename... Args>
	BakerService(kgr::in_place_t, Args&&... args) : baker{std::make_shared<Baker>(std::forward<Args>(args)...)} {}
	
	// construct method. Receives dependencies of a Baker.
	static auto construct(kgr::Inject<FlourBagService> flourBag) -> decltype(kgr::inject(flourBag.forward())) {
		// dependencies are injected with the forward method.
		return kgr::inject(flourBag.forward());
	}
	
	// forward method. This method define how the service is injected.
	// It's virtual because other services can override this one.
	// See example 4 for more detail.
	virtual std::shared_ptr<Baker> forward() {
		return baker;
	}
	
private:
	std::shared_ptr<Baker> baker;
};

// Bakery service definition. 
struct BakeryService {
	// constructor. Receives a fully constructed bakery to contain.
	template<typename... Args>
	BakeryService(kgr::in_place_t, Args&&... args) : bakery{std::forward<Args>(args)...} {}
	
	// construct method. Receives dependencies of a Baker.
	// We have to receive the OvenService has a reference because it's single.
	static auto construct(kgr::Inject<OvenService> oven) -> decltype(kgr::inject(oven.forward())) {
		// dependencies are injected with the forward method.
		return kgr::inject(oven.forward());
	}
	
	// forward method. This method define how the service is injected.
	Bakery forward() {
		return move(bakery);
	}
	
private:
	Bakery bakery;
};

int main()
{
	kgr::Container container;
	
	// The return type of 'service<BakeryService>' is the same as the BakeryService::forward return type.
	Bakery bakery = container.service<BakeryService>();
	bakery.name = "Le bon pain";
	
	// The return type of 'service<BakerService>' is the same as the BakerService::forward return type.
	std::shared_ptr<Baker> baker = container.service<BakerService>();
	
	bakery.start(baker);
	
	return 0;
}
