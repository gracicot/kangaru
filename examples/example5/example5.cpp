#include <iostream>
#include <string>
#include <memory>

#include "kangaru.hpp"

/**
 * This example explains advanced use of kangaru and it's components.
 * It covers pointer types
 */

using namespace std;
using namespace kgr;

struct ScrewDriver {
	double length = 0;
};

struct Hammer {
	double weight = 0;
};

struct Plumber {
	// Here we receive a unique_ptr and a raw pointer as dependency.
	Plumber(unique_ptr<ScrewDriver> _sd, Hammer* _h) : sd{move(_sd)}, h{_h} {
		sd->length = 6;
		h->weight = 3;
	}
	
	~Plumber() {
		// The container assumes that the plumber is responsible for
		// the allocation of his hammer, so we must delete it ourself.
		delete h;
	}
	
	void doJob() {
		cout << "I got a unique screw driver with a length of " << sd->length;
		cout << " and a hammer with a weight of " << h->weight << endl;
	}
	
private:
	unique_ptr<ScrewDriver> sd;
	Hammer* h;
};

namespace kgr {

// ScrewDriver will now be used as unique pointer
template<> struct Service<ScrewDriver> : NoDependencies, Unique {};

// We are using Raw pointer for the plumber.
// The container will manage the instance of the plumber because it's a single service.
template<> struct Service<Plumber> : Dependency<ScrewDriver, Hammer>, Raw, Single {};

// Hammer will be injected as Raw pointer.
template<> struct Service<Hammer> : NoDependencies, Raw {};

}

int main()
{
	auto container = make_container();
	
	Plumber* plumber = container->service<Plumber>();
	
	plumber->doJob();
	
	// We do not need to delete single services.
	// The container still tracking and managing instances of single services, even when they are raw pointers.
	return 0;
}
