#include <iostream>
#include <string>
#include <vector>

#include <kangaru/kangaru.hpp>

/**
 * This example explains moderate use of kangaru and it's components.
 * It covers providing instances to the container, self-injection
 */

// Uncomment this to reciece the scene in process_inputs_mod
// #define HAS_SCENE_PARAMETER

struct KeyboardState {};
struct MessageBus {};
struct Camera {};

// This is the definition for our classes
struct KeyboardStateService : kgr::single_service<KeyboardState> {};
struct MessageBusService    : kgr::single_service<MessageBus> {};
struct CameraService        : kgr::service<Camera> {};

// This is the service map. We map a parameter type to a service definition.
auto service_map(KeyboardState const&) -> KeyboardStateService;
auto service_map(MessageBus const&)    -> MessageBusService;
auto service_map(Camera const&)        -> CameraService;

// These are functions we will call with invoke
bool process_inputs(KeyboardState& ks, MessageBus& mb) {
	std::cout << "processing inputs...\n";
	return true;
}

#ifndef HAS_SCENE_PARAMETER

	bool process_inputs_mod(KeyboardState& ks, MessageBus& mb, bool check_modifiers) {
		std::cout << "process inputs " << (check_modifiers ? "with" : "without") << " modifiers\n";
		return !check_modifiers;
	}

#else

	bool process_inputs_mod(KeyboardState& ks, MessageBus& mb, Camera scene, bool check_modifiers) {
		std::cout << "process inputs " << (check_modifiers ? "with" : "without") << " modifiers\n";
		std::cout << "got the scene!\n";
		return !check_modifiers;
	}

#endif

int main()
{
	kgr::container container;
	
	// We invoke a function specifying services
	bool result1 = container.invoke<KeyboardStateService, MessageBusService>(process_inputs);
	
	// We invoke a function using the service map
	bool result2 = container.invoke(process_inputs);
	bool result3 = container.invoke(process_inputs_mod, true);
	
	std::cout << '\n';
	
	std::cout << "Invoke results: \n  1: " << result1;
	std::cout << "\n  2: " << result2;
	std::cout << "\n  3: " << result3 << '\n';
	
	container.invoke([](Camera camera) {
		std::cout << "Lambda called." << std::endl;
	});
}
