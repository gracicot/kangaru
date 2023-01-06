#include <iostream>
#include <string>

#include <kangaru-prev/kangaru.hpp>

/**
 * This example refect snippets of code found in the documentation section 7: Autocall
 * It explains how to call member functions of a service upen construction and injection by setters.
 */

// This is a utility macro to workaround the lack of type inference for non-type template parameter
// Will not be needed once this library upgrade to C++17
#define METHOD(...) ::kgr::method<decltype(__VA_ARGS__), __VA_ARGS__>

struct Scene {};
struct Camera {};
struct Window {
	int get_framerate() {
		return 60;
	}
};

struct MessageBus {
	MessageBus() = default;
	
	void init(Window& window, Camera& camera) {
		 max_delay = 3 * window.get_framerate();
		 std::cout << "max_delay set to: " << max_delay << '\n';
	}
	
	void set_scene(Scene& scene) {
		this->scene = &scene;
		std::cout << "Setting scene" << '\n';
	}
	
private:
	Scene* scene = nullptr;
	int max_delay = 0;
};

struct SceneService : kgr::single_service<Scene> {};
struct CameraService : kgr::single_service<Camera> {};
struct WindowService : kgr::single_service<Window> {};
struct MessageBusService : kgr::single_service<MessageBus>, kgr::autocall<
	METHOD(&MessageBus::init),
	METHOD(&MessageBus::set_scene)
> {};

struct MessageBusServiceInvoke : kgr::single_service<MessageBus>, kgr::autocall<
	kgr::invoke<METHOD(&MessageBus::init), WindowService, CameraService>,
	kgr::invoke<METHOD(&MessageBus::set_scene), SceneService>
> {};

auto service_map(Scene const&) -> SceneService;
auto service_map(Camera const&) -> CameraService;
auto service_map(Window const&) -> WindowService;

int main()
{
	kgr::container container;
	
	container.emplace<MessageBusService>();
	std::cout << '\n';
	container.emplace<MessageBusServiceInvoke>();
}
