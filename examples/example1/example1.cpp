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
	PathPrinter(PathProvider* _pathProvider) : pathProvider{_pathProvider} {}
	
	void print() {
		cout << pathProvider->path << endl;
	}
	
private:
	PathProvider* pathProvider;
};

// This is our service definitions
struct PathProviderService : Type<PathProvider*>, Single {
	PathProviderService(shared_ptr<PathProvider> instance) : _instance{move(instance)} {}
	
	static PathProviderService construct() {
		return {make_shared<PathProvider>()};
	}
	
	ServiceType forward() {
		return _instance.get();
	}
	
private:
	shared_ptr<PathProvider> _instance;
};

struct PathPrinterService : Type<PathPrinter> {
	PathPrinterService(PathPrinter instance) : _instance{move(instance)} {}
	
	static PathPrinterService construct(PathProviderService provider) {
		return {provider.forward()};
	}
	
	ServiceType forward() {
		return move(_instance);
	}
	
private:
	PathPrinter _instance;
};

int main()
{
	auto container = make_container();
	
	// a PathProvider is provided for every printer
	auto printer1 = container->service<PathPrinterService>();
	auto printer2 = container->service<PathPrinterService>();
	auto printer3 = container->service<PathPrinterService>();
	
	auto provider = container->service<PathProviderService>();
	
	provider->path = "/home/test";
	
	// every printer will print /home/test, because every printer has the same instance of PathProvider
	printer1.print();
	printer2.print();
	printer3.print();
	
	return 0;
}
