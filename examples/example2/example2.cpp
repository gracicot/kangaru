#include <iostream>
#include <string>
#include <memory>

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
	Product(WoodStack* stack) {
		stack->planks--;
	}
	
	string name;
};

// This is our wood stack service definitions
struct WoodStackService : Type<WoodStack*>, Single {
	WoodStackService(shared_ptr<WoodStack> instance) : _instance{move(instance)} {}
	
	static WoodStackService construct() {
		return make_shared<WoodStack>();
	}
	
	ServiceType forward() {
		return _instance.get();
	}
	
private:
	shared_ptr<WoodStack> _instance;
};

// This is our product service definitions
struct ProductService : Type<unique_ptr<Product>> {
	ProductService(unique_ptr<Product> instance) : _instance{move(instance)} {}
	
	static ProductService construct(WoodStackService stack) {
		return unique_ptr<Product>(new Product{stack.forward()});
	}
	
	ServiceType forward() {
		return move(_instance);
	}
	
private:
	unique_ptr<Product> _instance;
};

struct Carpenter {
	Carpenter(Container* _container, WoodStack* _stack) : container{_container}, stack{_stack} {}
	
	// We are using ServiceType, which in this case is an alias to unique_ptr<Product>.
	// Since the pointer type can be changed, using only unique_ptr here may be wrong.
	ProductService::ServiceType makeProduct(string name) {
		if (stack->planks > 0) {
			cout << "Another " << name << " made, but only " << stack->planks << " planks left!" << endl;
		
			auto product = container->service<ProductService>();
			product->name = name;
		
			return product;
		}
		
		cout << "No planks left, no product made." << endl;	
		return nullptr;
	}
	
private:
	Container* container;
	WoodStack* stack;
};

// This is our carpenter service definitions
struct CarpenterService : Type<Carpenter> {
	CarpenterService(Carpenter instance) : _instance{move(instance)} {}
	
	static CarpenterService construct(ContainerService container, WoodStackService& stack) {
		return Carpenter{container.forward(), stack.forward()};
	}
	
	ServiceType forward() {
		return move(_instance);
	}
	
private:
	Carpenter _instance;
};

int main()
{
	auto container = make_container();
	
	// We made the stack ourself and set the number of planks to 2
	auto stack = make_shared<WoodStack>();
	
	// We are providing our stack instance to the container.
	container->instance(WoodStackService{stack});
	
	stack->planks = 2;
	
	// It has the Container and the WoodStack injected.
	auto gerald = container->service<CarpenterService>();
	
	// Will print: Another computer desk made, but only 1 planks left!
	auto product1 = gerald.makeProduct("computer desk");
	
	// Will print: Another chair made, but only 0 planks left!
	auto product2 = gerald.makeProduct("chair");
	
	// Will print: No planks left, no product made.
	// As a result, product3 is null.
	auto product3 = gerald.makeProduct("table");
	
	if (!product3) {
		cout << "There's definitely no product made!" << endl;
	}
	
	return 0;
}
