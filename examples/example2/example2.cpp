#include <iostream>
#include <string>
#include <memory>
#include <vector>

#include "kangaru.hpp"

/**
 * This example explains moderate use of kangaru and it's components.
 * It covers providing instances to the container, self-injection
 */

using namespace std;
using namespace kgr;

struct WoodStack {
	int planks;
};

struct Product {
	Product(WoodStack& stack) {
		stack.planks--;
	}
	
	string name;
};

// This is our wood stack service definition
struct WoodStackService : SingleService<WoodStack> {
	// We need constructors for our use case.
	using Self::Self;
};

// This is our product service definition
struct ProductService : Service<Product, Dependency<WoodStackService>> {};

struct Carpenter {
	Carpenter(Container& _container, WoodStack& _stack) : container{_container}, stack{_stack} {}
	
	// We are using ServiceType, which in this case is an alias to unique_ptr<Product>.
	void makeProduct(string name) {
		if (stack.planks > 0) {
			cout << "Another " << name << " made, but only " << stack.planks << " planks left!" << endl;
		
			auto product = container.service<ProductService>();
			product.name = name;
		
			products.emplace_back(move(product));
		} else {
			cout << "No planks left, no product made." << endl;
		}
	}
	
private:
	vector<Product> products;
	Container& container;
	WoodStack& stack;
};

// This is our carpenter service definition
struct CarpenterService : Service<Carpenter, Dependency<ContainerService, WoodStackService>> {};

int main()
{
	Container container;
	
	// We made the stack ourself and set the number of planks to 2
	WoodStack stack{2};
	
	// We are providing our stack instance to the container.
	container.instance(WoodStackService{stack});
	
	
	// It has the Container and the WoodStack injected.
	auto gerald = container.service<CarpenterService>();
	
	// Will print: Another computer desk made, but only 1 planks left!
	gerald.makeProduct("computer desk");
	
	// Will print: Another chair made, but only 0 planks left!
	gerald.makeProduct("chair");
	
	// Will print: No planks left, no product made.
	// As a result, product3 is not made.
	gerald.makeProduct("table");
	
	return 0;
}
