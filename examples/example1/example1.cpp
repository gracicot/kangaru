#include <iostream>
#include <string>
#include <memory>

#include <kangaru/kangaru.hpp>

/**
 * This example explains the very basic use of kangaru.
 * It covers dependencies, Single services and basic use of the container.
 */

// Camera is a user class.
struct Camera {
	int position;
};

// Scene too. User class.
struct Scene {
	Scene(Camera c, int w = 800, int h = 600) :
		camera{c}, width{w}, height{h} {}
	
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
	
	Camera camera = container.service<CameraService>();
	Camera furtherCamera = container.service<CameraService>(14);
	
	std::cout << "Camera Position: " << camera.position << '\n';
	std::cout << "Further Camera Position: " << furtherCamera.position << '\n';
	
	Screen screen1 = container.service<ScreenService>();
	Screen screen2 = container.service<ScreenService>();
	
	std::cout << "Is both scene the same? " << (&screen1.scene == &screen2.scene ? "yes" : "no") << '\n';
}
