#include <iostream>
#include <string>
#include <memory>

#include "kangaru.hpp"

/**
 * This example explains advanced use of kangaru and it's components.
 * It covers autocall (injection by setters)
 */

// These are some utility macros to workaround the lack of type inference for non-type template parameter
// Will not be needed when N4469 will be accepted
#define METHOD(...) decltype(__VA_ARGS__), __VA_ARGS__
#define INVOKE(...) ::kgr::Method<decltype(__VA_ARGS__), __VA_ARGS__>

using namespace std;
using namespace kgr;

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
struct KeyboardService : SingleService<Keyboard> {};
struct MonitorService : SingleService<Monitor> {};
struct MouseService : SingleService<Mouse> {};
struct SpeakersService : SingleService<Speakers> {};

struct MinimalComputerService : Service<Computer, Dependency<KeyboardService>> {};

struct EquippedComputerService : Service<Computer, Dependency<KeyboardService>> {
	using invoke = Invoke<
		INVOKE(&Self::autocall<METHOD(&Computer::setAccessories), MouseService, SpeakersService>),
		INVOKE(&Self::autocall<METHOD(&Computer::setMonitor), MonitorService>)
	>;
};

int main()
{
	Container container;
	
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
	
	return 0;
}
