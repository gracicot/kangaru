#include <iostream>
#include <string>
#include <memory>

#include "kangaru.hpp"

/**
 * This example explains advanced use of kangaru and it's components.
 * It covers invoke and autocall (injection by setters)
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

// this is a struct that associate a type with a service.
// It will tell the container which service definition to pick when a specific type is encountered.
template<typename>
struct ServiceMap;

// service definitions
struct KeyboardService : SingleService<Keyboard> {};
struct MonitorService : SingleService<Monitor> {};
struct MouseService : SingleService<Mouse> {};
struct SpeakersService : SingleService<Speakers> {};

struct MinimalComputerService : Service<Computer, Dependency<KeyboardService>> {};

struct EquippedComputer1Service : Service<Computer, Dependency<KeyboardService>> {
	using invoke = Invoke<
		INVOKE(&Self::autocall<METHOD(&Computer::setAccessories), MouseService, SpeakersService>),
		INVOKE(&Self::autocall<METHOD(&Computer::setMonitor), MonitorService>)
	>;
};

struct EquippedComputer2Service : Service<Computer, Dependency<KeyboardService>> {
	using invoke = Invoke<
		INVOKE(&Self::autocall<METHOD(&Computer::setAccessories), ServiceMap>),
		INVOKE(&Self::autocall<METHOD(&Computer::setMonitor), ServiceMap>)
	>;
};

// To which service definition do we refer when the function parameter 'Keyboard&' is found?
template<> struct ServiceMap<Keyboard&> {
	// It refers to the KeyboardService! 
	using Service = KeyboardService;
};

// Same for the following...

template<> struct ServiceMap<Monitor&> {
	using Service = MonitorService;
};

template<> struct ServiceMap<Mouse&> {
	using Service = MouseService;
};

template<> struct ServiceMap<Speakers&> {
	using Service = SpeakersService;
};

// A funtion to wash our favourite monitor and keyboard.
// Called with normal parameters. A service will be needed to be used with invoke.
void washMonitorAndKeyboard1(Monitor& monitor, Keyboard& keyboard) {
	
	cout << "Monitor of size of " << monitor.size 
		 << " inch and a keyboard with " << keyboard.switchColor
		 << " switches has been washed." << endl;
}

// A funtion to wash our favourite monitor and keyboard.
// A service map is not needed for invoke to work.
double washMonitorAndKeyboard2(MonitorService& monitorService, KeyboardService& keyboardService) {
	auto& monitor = monitorService.forward();
	auto& keyboard = keyboardService.forward();
	
	cout << "Monitor of size of " << monitor.size 
		 << " inch and a keyboard with " << keyboard.switchColor
		 << " switches has been washed." << endl;
		 
	return 3.2;
}

int main()
{
	auto container = make_container();
	
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
	
	// make three computers
	auto computer1 = container.service<EquippedComputer1Service>();
	auto computer2 = container.service<EquippedComputer2Service>();
	auto computer3 = container.service<MinimalComputerService>();
	
	// computer 1 and 2 will print the same thing.
	computer1.printGear();
	computer2.printGear();
	// computer 3 will print only about the keyboard.
	computer3.printGear();
	
	// will call 'washMonitorAndKeyboard1' with the right parameters.
	container.invoke<ServiceMap>(washMonitorAndKeyboard1);
	
	// will call 'washMonitorAndKeyboard2' with the right parameters.
	// return values are working too!
	double result = container.invoke(washMonitorAndKeyboard2);
	
	cout << "Result of washMonitorAndKeyboard2 is " << result << "!" << endl;
	
	return 0;
}
