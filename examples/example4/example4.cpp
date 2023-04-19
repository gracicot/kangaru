#include <kangaru/kangaru.hpp>
#include <iostream>

/**
 * This example reflects snippets of code found in the documentation section 4: Managing Containers
 * It explains how to branch containers and operate between them.
 */

struct Service {};

struct SingleService1 : kgr::single_service<Service> {};
struct SingleService2 : kgr::single_service<Service> {};
struct SingleService3 : kgr::single_service<Service> {};

int main()
{
	{
		kgr::container container1;
		
		container1.emplace<SingleService1>();
		container1.emplace<SingleService2>();
		
		auto container2 = container1.fork();
		
		std::cout << "container2 has SingleService1, SingleService2? ";
		std::cout << std::boolalpha << container2.contains<SingleService1>() << ", ";
		std::cout << std::boolalpha << container2.contains<SingleService2>() << '\n';
		
		container1.emplace<SingleService3>();
		container2.emplace<SingleService3>();
		
		bool are_same = &container1.service<SingleService3>() == &container2.service<SingleService3>();
		
		std::cout << "container1 and container2 has the same SingleService3? ";
		std::cout << std::boolalpha << are_same << '\n';
		
		// container1  ---o---o---*---o
		//                         \
		// container2               ---o
	}
	
	std::cout << '\n';
	
	{
		kgr::container container1;
		
		container1.emplace<SingleService1>();
		
		{
			auto container2 = container1.fork();
			
			container1.emplace<SingleService2>();
			container2.emplace<SingleService2>();
			container2.emplace<SingleService3>();
			
			bool are_same2 = &container1.service<SingleService2>() == &container2.service<SingleService2>();
			
			std::cout << "container1 and container2 has the same SingleService2? ";
			std::cout << std::boolalpha << are_same2 << '\n';
			
			// At that point, both containers have their own SingleService2 instance.
			// Only container2 has SingleService3
			// container1 is owner of SingleService1, and container2 observes it.
			
			container1.merge(container2);
			
			// container2 is still valid, but is no longer owner of any services.
			// We can still use the container and will still observe every service it was owner before.
			are_same2 = &container1.service<SingleService2>() != &container2.service<SingleService2>();
			
			std::cout << "has container1 kept his own instance of SingleService2 after merge? ";
			std::cout << std::boolalpha << are_same2 << '\n';
			
			std::cout << "do container1 contains container2's SingleService3 instance? ";
			std::cout << std::boolalpha << container1.contains<SingleService3>() << '\n';
		}
		
		// container1  ---o---*---o----------*---
		//                     \            /
		// container2           ---o---o---
	}
	
	std::cout << '\n';
	
	{
		kgr::container container1;
		
		container1.emplace<SingleService1>();
		
		auto container2 = container1.fork();
		
		container2.contains<SingleService1>(); // true
		container2.contains<SingleService2>(); // false
			
		std::cout << "do container2 has instances of SingleService1, SingleService2? ";
		std::cout << std::boolalpha << container2.contains<SingleService1>() << ", ";
		std::cout << std::boolalpha << container2.contains<SingleService2>() << '\n';
		
		std::cout << "Creating SingleService2 in container1...\n";
		container1.emplace<SingleService2>();
		
		std::cout << "Rebasing container2 from container1...\n";
		container2.rebase(container1); // rebase from container1
		
		std::cout << "do container2 has instances of SingleService1, SingleService2? ";
		std::cout << std::boolalpha << container2.contains<SingleService1>() << ", ";
		std::cout << std::boolalpha << container2.contains<SingleService2>() << '\n';
		
		// container1  ---o---*---o---*---
		//                     \       \
		// container2           --------*---
	}
}
