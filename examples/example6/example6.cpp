#include <kangaru-prev/kangaru.hpp>
#include <iostream>

/**
 * This example refect snippets of code found in the documentation section 6: Autowire
 * It explains how to autowire services using the service map.
 */

struct Camera {
	int position;
};

struct Scene {
	Scene(Camera& c, int w = 0, int h = 0) :
		camera(c), width{w}, height{h} {}
	
	Camera& camera;
	int width;
	int height;
};

auto service_map(Camera const&) -> kgr::autowire_single;
auto service_map(Scene const&) -> kgr::autowire;

int main()
{
	kgr::container container;
	
	container.invoke([](Camera& camera, Scene scene) {
		// Do stuff with the camera and the scene!
		(void) scene.camera.position;
	});
	
	// Optional, but can be useful
	using SceneService = kgr::mapped_service_t<Scene>;
	
	auto scene = container.service<SceneService>(1920, 1080);

	std::cout << "Scene created with size " << scene.width << "x" << scene.height << '\n';
}
