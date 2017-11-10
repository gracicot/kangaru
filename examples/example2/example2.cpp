#include <iostream>
#include <string>
#include <vector>

#include <kangaru/kangaru.hpp>

/**
 * This example explains moderate use of kangaru and it's components.
 * It covers providing instances to the container, self-injection
 */

using std::string;
using std::cout;
using std::endl;
using std::move;

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
struct WoodStackService : kgr::SingleService<WoodStack> {};

// This is our product service definition
struct ProductService : kgr::Service<Product, kgr::Dependency<WoodStackService>> {};

struct Carpenter {
	Carpenter(kgr::Container& _container, WoodStack& _stack) : container{_container}, stack{_stack} {}
	
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
	std::vector<Product> products;
	kgr::Container& container;
	WoodStack& stack;
};

// This is our carpenter service definition
struct CarpenterService : kgr::Service<Carpenter, kgr::Dependency<kgr::ContainerService, WoodStackService>> {};

int main()
{
	kgr::Container container;
	
	// We made the stack ourself and set the number of planks to 2
	container.emplace<WoodStackService>(2);
	
	
	// It has the Container and the WoodStack injected.
	auto gerald = container.service<CarpenterService>();
	
	// Will print: Another computer desk made, but only 1 planks left!
	gerald.makeProduct("computer desk");
	
	// Will print: Another chair made, but only 0 planks left!
	gerald.makeProduct("chair");
	
	// Will print: No planks left, no product made.
	// As a result, product3 is not made.
	gerald.makeProduct("table");
}
