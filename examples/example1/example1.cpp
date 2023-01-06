#include <kangaru-prev/kangaru.hpp>
#include <iostream>

/**
 * This example refect snippets of code found in the documentation section 1: Services
 * It explains how to branch containers and operate between them.
 */


// Camera is a user class.
struct Camera {
	int position;
};

// Scene too. User class.
struct Scene {
	Scene(Camera c, int w = 800, int h = 600) :
		camera{c.position}, width{w}, height{h} {}
	
private:
	Camera camera;
	int width;
	int height;
};

struct Screen {
	Scene& scene;
	Camera camera;
};

// This is our service definitions
struct CameraService : kgr::service<Camera> {};

// SceneService is a single service of Scene, that depends on a camera
struct SceneService : kgr::single_service<Scene, kgr::dependency<CameraService>> {};

// ScreenService is a single service of Screen, that depends on a scene and camera
struct ScreenService : kgr::service<Screen, kgr::dependency<SceneService, CameraService>> {};

int main()
{
	kgr::container container;
	
	// We create two cameras.
	Camera camera = container.service<CameraService>();
	Camera furtherCamera = container.service<CameraService>(14);
	
	// prints 0
	std::cout << "Camera Position: " << camera.position << '\n';
	
	// prints 14
	std::cout << "Further Camera Position: " << furtherCamera.position << '\n';
	
	// A Screen has a Scene and a Camera injected in it.
	Screen screen1 = container.service<ScreenService>();
	Screen screen2 = container.service<ScreenService>();
	
	// Spoiler: yes they are the same
	std::cout << "Is both scene the same? " << (&screen1.scene == &screen2.scene ? "yes" : "no") << '\n';
}
