#include <iostream>
#include <string>
#include <memory>

#include "kangaru.hpp"

/**
 * This example explains moderate use of kangaru and it's components.
 * It covers providing instances to the container and self-injection.
 */

using namespace std;
using namespace kgr;

struct Product {
	string name;
};

struct WoodStack {
	int planks;
};

struct Carpenter {
	Carpenter(shared_ptr<Container> _container, shared_ptr<WoodStack> _stack) : container{_container}, stack{_stack} {}
	
	shared_ptr<Product> makeProduct(string name) {
		if (stack->planks > 0) {
			cout << "Another " << name << " made, but only " << --stack->planks << " planks left!" << endl;
		
			auto product = container->service<Product>();
			product->name = name;
		
			return product;
		}
		
		cout << "No planks left, no product made." << endl;	
		return nullptr;
	}
	
private:
	shared_ptr<Container> container;
	shared_ptr<WoodStack> stack;
};

// Service definitions must be in the kgr namespace
namespace kgr {

// This is our service definitions
template<> struct Service<Product> : NoDependencies {};
template<> struct Service<Carpenter> : Dependency<Container, WoodStack> {};
template<> struct Service<WoodStack> : NoDependencies, Single {};

}

int main()
{
	auto container = make_container();
	
	// We made the stack ourself and set the number of planks to 2
	auto stack = make_shared<WoodStack>();
	stack->planks = 2;
	
	// We are providing our stack instance to the container.
	container->instance(stack);
	
	// It has the Container and the WoodStack injected.
	auto gerald = container->service<Carpenter>();
	
	// Will print: Another computer desk made, but only 1 planks left!
	auto product1 = gerald->makeProduct("computer desk");
	
	// Will print: Another chair made, but only 0 planks left!
	auto product2 = gerald->makeProduct("chair");
	
	// Will print: No planks left, no product made.
	// As a result, product3 is null.
	auto product3 = gerald->makeProduct("table");
	
	return 0;
}
