#include <iostream>
#include <string>
#include <memory>

#include "kangaru.hpp"

/**
 * This example explains the very basic use of kangaru.
 * It covers dependencies, Single services and basic use of the container.
 */

using namespace std;
using namespace kgr;

struct PathProvider {
	string path;
};

struct PathPrinter {
	// For the sake of simplicity, we use a shared_ptr
	PathPrinter(shared_ptr<PathProvider> _pathProvider) : pathProvider{_pathProvider} {}
	
	void print() {
		cout << pathProvider->path << endl;
	}
	
private:
	shared_ptr<PathProvider> pathProvider;
};

// Service definitions must be in the kgr namespace
namespace kgr {

// This is our service definitions
template<> struct Service<PathProvider> : NoDependencies, Single {};
template<> struct Service<PathPrinter> : Dependency<PathProvider> {};

}

int main()
{
	auto container = make_container();
	
	// a PathProvider is provided for every printer
	auto printer1 = container->service<PathPrinter>();
	auto printer2 = container->service<PathPrinter>();
	auto printer3 = container->service<PathPrinter>();
	
	auto provider = container->service<PathProvider>();
	
	provider->path = "/home/test";
	
	// every printer will print /home/test, because every printer has the same instance of PathProvider
	printer1->print();
	printer2->print();
	printer3->print();
	
	return 0;
}
