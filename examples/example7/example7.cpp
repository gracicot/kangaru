#include <iostream>
#include <string>

#include <kangaru/kangaru.hpp>

/**
 * This example refect snippets of code found in the documentation section 7: Operator Services
 * It explains how to use services bundled with kangaru that helps making specific operation on the container.
 */

// User classes
struct Window {};
struct Type {
	kgr::container& container;
};

struct MessageBus {
	void process_messages() {
		std::cout << "Messages processed\n";
	}
};

struct Scene {
	explicit Scene(char const* n = "") noexcept : name{n} {
		std::cout << "Scene created\n";
	}
	
	char const* name;
};

// Services declarations
struct MessageBusService : kgr::single_service<MessageBus> {};
struct WindowService : kgr::single_service<Window> {};
struct SceneService : kgr::service<Scene> {};
struct TypeService : kgr::service<Type, kgr::dependency<kgr::container_service>> {};

// Service map
auto service_map(Type const&) -> TypeService;
auto service_map(Window const&) -> WindowService;
auto service_map(MessageBus const&) -> MessageBusService;


// A function to be invoked
void send_message(MessageBus&, Window&, double timeout) {
	std::cout << "Message sent with a timeout of " << timeout << '\n';
}

int main() {
	// Container
	{
		kgr::container container1;
		kgr::container& container2 = container1.service<kgr::container_service>();
		
		std::cout << std::boolalpha << "Both containers are the same? " << (&container1 == &container2) << '\n'; 
		
		container1.invoke([&](kgr::container& container3, Type type) {
			std::cout << std::boolalpha << "Both container1 and container3 are the same? ";
			std::cout << (&container1 == &container3) << '\n';
		
			std::cout << std::boolalpha << "Both container3 and type.container are the same? ";
			std::cout << (&container3 == &type.container) << '\n';
		});
	}
	
	std::cout << '\n';
	
	// Fork
	{
		kgr::container container1;
		kgr::container container2 = container1.service<kgr::fork_service>();
		
		std::cout << std::boolalpha << "Both containers are the same? " << (&container1 == &container2) << '\n'; 
		
		container1.invoke([&](kgr::container container3) {
			std::cout << std::boolalpha << "Is container3 a fork? ";
			std::cout << (&container1 != &container3) << '\n';
		});
	}
	
	std::cout << '\n';
	
	// Generator
	{
		kgr::container container;
		kgr::generator<SceneService> scene_generator = container.service<kgr::generator_service<SceneService>>();
		
		Scene scene1 = scene_generator();
		Scene scene2 = scene_generator();
		Scene scene3 = scene_generator("special parameter");
	}
	
	std::cout << '\n';
	
	// Invoker
	{
		kgr::container container;
		kgr::invoker invoker = container.service<kgr::invoker_service>();
		
		invoker(send_message, 10); // calls send_message with 10 as it's timeout
	}
	
	std::cout << '\n';
	
	// Lazy
	{
		kgr::container container;
		kgr::lazy<MessageBusService> lazy_message_bus = container.service<kgr::lazy_service<MessageBusService>>();
		
		// MessageBus constructed here at `->` usage
		lazy_message_bus->process_messages();
	}
}
