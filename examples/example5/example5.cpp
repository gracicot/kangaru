#include <kangaru/kangaru.hpp>
#include <iostream>

/**
 * This example reflects snippets of code found in the documentation section 5: Supplied Services
 * It explains how to make single services not constructible implicitly by the container.
 */

struct Camera {
	int position;
};

struct Scene {
	Scene(Camera c, int w, int h) :
		camera{c.position}, width{w}, height{h} {}
	
private:
	Camera camera;
	int width;
	int height;
};

struct CameraService : kgr::service<Camera> {};
struct SceneService : kgr::single_service<Scene, kgr::dependency<CameraService>>, kgr::supplied {};

int main()
{
	{
		kgr::container container;
		bool inserted;
		
		inserted = container.emplace<SceneService>(1920, 1080); // construct a scene in the container.
		std::cout << std::boolalpha << "Is inserted? " << inserted << '\n';
		
		inserted = container.emplace<SceneService>(1024, 768); // construct a scene in the container.
		std::cout << std::boolalpha << "Is inserted a second time? " << inserted << '\n';

		Scene& scene = container.service<SceneService>(); // works, won't try to construct it.
		
		(void) scene;
	}
	
	try {
		kgr::container container;
		// The container cannot construct a scene and don't contain one.
		// Throws a supplied_not_found error.
		container.service<SceneService>();
	} catch(kgr::supplied_not_found const& error) {
		std::cout << "supplied_not_found thrown, what(): " << error.what() << std::endl;
	}
}
