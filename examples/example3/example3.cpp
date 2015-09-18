#include <iostream>
#include <string>
#include <memory>

#include "kangaru.hpp"

/**
 * This example explains moderate use of kangaru and it's components.
 * It covers callbacks and extending the container
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

struct MyContainer : Container {
	// This is the init function, we are initiating what we need to make the main() work.
    virtual void init() {
		// We are making our studio with a pretty name.
		// We are using make_service to make the right type of pointer.
		// In this case this will be equivalent to make_shared().
// 		auto studio = make_service<Studio>("The Music Box");
		
		// We are registering the studio instance to the conatiner.
// 		instance(studio);
		
		// Here we are giving the container a callback used to make Amps.
		// The container knows this function returns an Amp, so it will be used to construct the Amp service.
		// The container will always use this callback everytime we need an Amp.
// 		callback([]{
// 			// We are making a new amp with some watts, incrementing each time.
// 			// We are using make_service to make the right type of pointer.
// 			// In this case this will be equivalent to make_shared().
// 			// The pointer type may change in the future.
// 			static int watts = 0;
// 			auto amp = make_service<Amp>(watts += 65);
// 			
// 			cout << "A new amp is made with it's power at " << amp->watts << endl;
// 			
// 			return amp;
// 		});
    }
};

// This is our service definitions
struct AmpService : Service<Amp> {};
struct GuitarService : Service<Guitar, Dependency<AmpService>> {};
struct StudioService : SingleService<Studio> {};

int main()
{
	// The container type will be MyContainer.
	auto container = make_container<MyContainer>();
	
	auto guitar1 = container->service<GuitarService>();
	auto guitar2 = container->service<GuitarService>();
	auto guitar3 = container->service<GuitarService>();
	
	guitar1.model = "Gibson";
	guitar2.model = "Fender";
	guitar3.model = "Ibanez";
	
	auto studio = container->service<StudioService>();
	
	studio->record(guitar1);
	studio->record(guitar2);
	studio->record(guitar3);
	
	return 0;
}
