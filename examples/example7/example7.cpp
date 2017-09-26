#include <iostream>
#include <string>

#include <kangaru/kangaru.hpp>

/**
 * This example explains advanced use of kangaru and it's components.
 * It covers invoke and autocall (injection by setters) through service map.
 */

// This is a utility macro to workaround the lack of type inference for non-type template parameter
// Will not be needed once this library upgrade to C++17
#define METHOD(...) KGR_KANGARU_METHOD(__VA_ARGS__)

using std::string;
using std::cout;
using std::endl;

struct Keyboard {
	string switchColor;
};

struct Monitor {
	double size;
};

struct Mouse {
	int nbButton;
};

struct Speakers {
	int maxDb;
};

struct Computer {
	Computer(Keyboard& myKeyboard) : keyboard{myKeyboard} {}
	
	void setAccessories(Mouse& aMouse, Speakers& someSpeakers) {
		mouse = &aMouse;
		speakers = &someSpeakers;
	}
	
	void setMonitor(Monitor& aMonitor) {
		monitor = &aMonitor;
	}
	
	void printGear() const {
		cout << "This computer has a keyboard with " << keyboard.switchColor << " switches. " << endl;
		
		if (monitor) {
			cout << "    It got a " << monitor->size << " inch monitor." << endl;
		}
		
		if (mouse) {
			cout << "    It got a mouse with " << mouse->nbButton << " buttons." << endl;
		}
		
		if (speakers) {
			cout << "    It got speakers with a maximum of " << speakers->maxDb << " decibels." << endl;
		}
		
		cout << endl;
	}
	
private:
	Keyboard& keyboard;
	Monitor* monitor = nullptr;
	Mouse* mouse = nullptr;
	Speakers* speakers = nullptr;
};

// service definitions
struct KeyboardService : kgr::SingleService<Keyboard> {};
struct MonitorService : kgr::SingleService<Monitor> {};
struct MouseService : kgr::SingleService<Mouse> {};
struct SpeakersService : kgr::SingleService<Speakers> {};

struct MinimalComputerService : kgr::Service<Computer, kgr::Dependency<KeyboardService>> {};

struct EquippedComputerService : kgr::Service<Computer, kgr::Dependency<KeyboardService>>, kgr::AutoCall<
	METHOD(&Computer::setMonitor),
	METHOD(&Computer::setAccessories)
> {};

// To which service definition do we refer when the function parameter 'Keyboard' is found?
auto service_map(Keyboard) -> KeyboardService;

// The same for other definitions
auto service_map(Monitor) -> MonitorService;
auto service_map(Mouse) -> MouseService;
auto service_map(Speakers) -> SpeakersService;

// A funtion to wash our favourite monitor and keyboard.
// A service will be needed to be used with invoke.
double washMonitorAndKeyboard(Monitor& monitor, Keyboard& keyboard) {
	cout << "Monitor of size of " << monitor.size 
		 << " inch and a keyboard with " << keyboard.switchColor
		 << " switches has been washed." << endl;
		 
	return 9.8;
}

int main()
{
	kgr::Container container;
	
	// getting our four pieces of hardware
	auto& keyboard = container.service<KeyboardService>();
	auto& monitor = container.service<MonitorService>();
	auto& mouse = container.service<MouseService>();
	auto& speakers = container.service<SpeakersService>();
	
	// and give them values.
	keyboard.switchColor = "blue";
	monitor.size = 19.5;
	mouse.nbButton = 3;
	speakers.maxDb = 80;
	
	// make two computers
	auto computer1 = container.service<EquippedComputerService>();
	auto computer2 = container.service<MinimalComputerService>();
	
	// computer 1 will print everything.
	computer1.printGear();
	// computer 2 will print only about the keyboard.
	computer2.printGear();
	
	// will call 'washMonitorAndKeyboard' with the right set of parameters.
	double result = container.invoke(washMonitorAndKeyboard);
	
	cout << "Result of washMonitorAndKeyboard is " << result << "!" << endl;
}
