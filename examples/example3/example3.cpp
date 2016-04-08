#include <iostream>
#include <string>

#include "kangaru/kangaru.hpp"

/**
 * This example explains moderate use of kangaru and it's components.
 * It covers overriding the construct method and extending the container
 */

using std::string;
using std::cout;
using std::endl;

struct Amp {
	Amp(int myWatts = 0) : watts{myWatts} {};
	
	int watts;
};

struct Guitar {
	Guitar(Amp myAmp) : amp{myAmp} {};
	
	string model;
	Amp amp;
};

struct Studio {
	Studio(string myName = "") : name{myName} {};
	
	void record(const Guitar& guitar) {
		cout << "The studio \"" << name << "\" records a " << guitar.model << " with a " << guitar.amp.watts << " watt amp." << endl;
	}
	
	string name;
};

// This is our service definitions
struct AmpService : kgr::Service<Amp> {
	static Self construct() {
		static int watts = 0;
		return Amp{watts += 48};
	}
};

struct GuitarService : kgr::Service<Guitar, kgr::Dependency<AmpService>> {};
struct StudioService : kgr::SingleService<Studio> {};

struct MyContainer : kgr::Container {
    MyContainer() {
		// We are making our studio with a pretty name.
		service<StudioService>().name = "The Music Box";
    }
};

int main()
{
	// The container type will be MyContainer.
	MyContainer container;
	
	auto guitar1 = container.service<GuitarService>();
	auto guitar2 = container.service<GuitarService>();
	auto guitar3 = container.service<GuitarService>();
	
	guitar1.model = "Gibson";
	guitar2.model = "Fender";
	guitar3.model = "Ibanez";
	
	auto& studio = container.service<StudioService>();
	
	studio.record(guitar1);
	studio.record(guitar2);
	studio.record(guitar3);
	
	return 0;
}
