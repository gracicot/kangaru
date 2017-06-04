#include <iostream>
#include <string>
#include <tuple>

#include <kangaru/kangaru.hpp>

/**
 * This example explains moderate use of kangaru and it's components.
 * It covers overriding the construct method
 */

using std::string;
using std::cout;
using std::endl;

struct Amp {
	Amp(int myWatts = 0) : watts{myWatts} {};
	
	int watts;
};

struct Guitar {
	Guitar(Amp myAmp, std::string myModel) : amp{myAmp}, model{std::move(myModel)} {};
	
	Amp amp;
	string model;
};

struct Studio {
	Studio(string myName = "") : name{myName} {};
	
	void record(const Guitar& guitar) {
		cout << "The studio \"" << name << "\" records a " << guitar.model << " with a " << guitar.amp.watts << " watt amp." << endl;
	}
	
	string name;
};

struct AmpService : kgr::Service<Amp> {
	// Here we override the construct function. We are injecting a int into our Amp type.
	static auto construct() -> decltype(kgr::inject(std::declval<int>())) {
		static int watts = 0;
		return kgr::inject(watts += 48);
	}
};

// Other service definitions
struct GuitarService : kgr::Service<Guitar, kgr::Dependency<AmpService>> {};
struct StudioService : kgr::SingleService<Studio> {};

int main()
{
	kgr::Container container;
	
	container.service<StudioService>().name = "The Music Box";
	
	// Here we are sending an additional argument to the constructor.
	// As you can see, the Guitar constructor takes a string as second argument.
	auto guitar1 = container.service<GuitarService>("Gibson");
	auto guitar2 = container.service<GuitarService>("Fender");
	auto guitar3 = container.service<GuitarService>("Ibanez");
	
	auto& studio = container.service<StudioService>();
	
	// Output:
	// The studio "The Music Box" records a Gibson with a 48 watt amp.
	studio.record(guitar1);
	
	// The studio "The Music Box" records a Fender with a 96 watt amp.
	studio.record(guitar2);
	
	// The studio "The Music Box" records a Ibanez with a 144 watt amp.
	studio.record(guitar3);
}
