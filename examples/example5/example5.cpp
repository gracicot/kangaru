#include <iostream>
#include <string>
#include <memory>
#include <tuple>

#include <kangaru/kangaru.hpp>

/**
 * This example explains advanced use of kangaru and it's components.
 * It covers custom service definition
 */

struct Camera {
	int position;
};

struct Scene {
	Scene(Camera c, int w, int h) :
		camera{c}, width{w}, height{h} {}
	
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
		
		inserted = container.emplace<SceneService>(1920, 1080); // contruct a scene in the container.
		std::cout << std::boolalpha << "Is inserted? " << inserted << '\n';
		
		inserted = container.emplace<SceneService>(1024, 768); // contruct a scene in the container.
		std::cout << std::boolalpha << "Is inserted a second time? " << inserted << '\n';

		Scene& scene = container.service<SceneService>(); // works, won't try to construct it.
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
