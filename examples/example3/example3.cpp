#include <iostream>
#include <string>
#include <memory>
#include <tuple>

#include "kangaru.hpp"

/**
 * This example explains moderate use of kangaru and it's components.
 * It covers overriding the construct method
 */

using namespace std;
using namespace kgr;

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
struct AmpService : Service<Amp> {
	static auto construct() {
		static int watts = 0;
		return std::forward_as_tuple(watts += 48);
	}
};

struct GuitarService : Service<Guitar, Dependency<AmpService>> {};
struct StudioService : SingleService<Studio> {};

int main()
{
	Container container;
	
	container.service<StudioService>().name = "The Music Box";
	
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
