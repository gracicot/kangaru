#include <kangaru-prev/kangaru.hpp>
#include <iostream>
#include <unordered_map>
#include <functional>
#include <string>
#include <memory>

// Shape classes
// These are the classes we want to create with our factory.
struct Shape {
	virtual int area() = 0;
	virtual ~Shape() = default;
};

struct Square : Shape {
	explicit Square(int l) noexcept : lenght{l} {}
	int lenght;
	
	int area() override {
		std::cout << "Square!\n";
		return lenght * lenght;
	}
};

struct Rectangle : Shape {
	explicit Rectangle(int h, int w) noexcept : height{h}, width{w} {}
	int height, width;
	
	int area() override {
		std::cout << "Rectangle!\n";
		return height * width;
	}
};

struct Circle : Shape {
	explicit Circle(int r) noexcept : radius{r} {}
	int radius;
	
	int area() override {
		std::cout << "Circle!\n";
		return int(3.14 * radius * radius);
	}
};

// Other classes. Contains information we need to instanciate shape classes.
struct Window {
	int height, width;
};

struct CursorState {
	int dragging_distance;
};

// This is our factory.
// You can add a type associated with a lambda that constructs it.
// Since that lambda is called through the invoker, we can recieve any number of services.
struct Factory {
	explicit Factory(kgr::invoker i) noexcept : invoker{i} {}
	
	template<typename T>
	void add(std::string const& type, T maker) {
		makers[type] = [maker](kgr::invoker invoker) {
			return std::unique_ptr<Shape>{invoker(maker)};
		};
	}
	
	std::unique_ptr<Shape> make(std::string type) {
		return makers[type](invoker);
	}
	
private:
	// We have the invoker to call functions in the map.
	kgr::invoker invoker;
	
	// A map of string as key and functions that takes an invoker as value.
	std::unordered_map<std::string, std::function<std::unique_ptr<Shape>(kgr::invoker)>> makers;
};

// This is our services. Factory depends on the invoker.
struct WindowService : kgr::single_service<Window> {};
struct CursorStateService : kgr::single_service<CursorState> {};
struct FactoryService : kgr::service<Factory, kgr::dependency<kgr::invoker_service>> {};

// This is the mapping. We need it to make the lambda callable with injection.
auto service_map(Window const&) -> WindowService;
auto service_map(CursorState const&) -> CursorStateService;
auto service_map(Factory const&) -> FactoryService;

Factory setup_factory(Factory factory) {
	// To make a rectangle, we need the window only.
	factory.add("Rectangle", [](Window& window) {
		return new Rectangle{window.width / 10, window.height / 10};
	});
	
	// To make a square, we need the window and the cursor state.
	factory.add("Square", [](Window& window, CursorState& cusor_state) {
		return new Square{(window.height / 10 ) - cusor_state.dragging_distance};
	});
	
	// To make a circle, we only need the cursor state.
	factory.add("Circle", [](CursorState& cusor_state) {
		return new Circle{cusor_state.dragging_distance};
	});
	
	return factory;
}

int main() {
	kgr::container container;
	
	// We setup the factory.
	// Since FactoryService, we can simply ask the container to invoke it.
	Factory factory = container.invoke(setup_factory);
	
	// We instanciate the other services with parameters.
	container.emplace<WindowService>(640, 480);
	container.emplace<CursorStateService>(20);
	
	// area to display.
	int area;
	
	// Here we make many shapes.
	// Even though creating a specific state require
	// many specific parameters, we only need to give the string of the type.
	auto circle_shape = factory.make("Circle");
	area = circle_shape->area();
	std::cout << "Area of circle_shape is: " << area << " pixels squared.\n\n";
	
	auto square_shape = factory.make("Square");
	area = square_shape->area();
	std::cout << "Area of square_shape is: " << area << " pixels squared.\n\n";
	
	auto rectangle_shape = factory.make("Rectangle");
	area = rectangle_shape->area();
	std::cout << "Area of rectangle_shape is: " << area << " pixels squared.\n";
}
