#include <kangaru-prev/kangaru.hpp>
#include <iostream>

/**
 * This example refect snippets of code found in the documentation section 3: Polymorphic Services
 * It explains how to override services and use them polymorphically.
 */

struct AbstractCamera {
	virtual void projection() = 0;
};

struct Camera : AbstractCamera {
	void projection() override {
		std::cout << "default projection" << std::endl;
	}
};

struct PerspectiveCamera : Camera {
	void projection() override {
		std::cout << "perspective projection" << std::endl;
	}
};

struct OrthogonalCamera : Camera {
	void projection() override {
		std::cout << "orthogonal projection" << std::endl;
	}
};


struct AbstractCameraService :
	kgr::abstract_service<AbstractCamera> {};
	
struct AbstractCameraServiceDefault :
	kgr::abstract_service<AbstractCamera>,
	kgr::defaults_to<struct PerspectiveCameraService> {};
	
struct CameraService :
	kgr::single_service<Camera>,
	kgr::polymorphic {};

// Multiple overrides are supported
struct PerspectiveCameraService :
	kgr::single_service<PerspectiveCamera>,
	kgr::overrides<CameraService, AbstractCameraServiceDefault> {};
	
struct OrthogonalCameraService :
	kgr::single_service<OrthogonalCamera>,
	kgr::overrides<CameraService>,
	kgr::final {};

int main()
{
	{
		kgr::container container;
		
		// Camera is returned
		Camera& camera1 = container.service<CameraService>();
		camera1.projection(); // prints `default projection`
		
		// PerspectiveCamera is registered as Camera
		container.emplace<PerspectiveCameraService>();
		
		// PerspectiveCamera is returned
		Camera& camera2 = container.service<CameraService>();
		camera2.projection(); // prints `perspective projection`
	}
	
	std::cout << '\n';
	
	{
		kgr::container container;
		
		container.service<PerspectiveCameraService>();
		container.service<OrthogonalCameraService>();
		
		// instance of OrthogonalCamera returned
		Camera& camera = container.service<CameraService>();
		camera.projection();
	}
	
	std::cout << '\n';
	
	{
		kgr::container container;
		
		container.service<OrthogonalCameraService>();
		container.service<PerspectiveCameraService>();
		
		// instance of PerspectiveCamera returned
		Camera& camera = container.service<CameraService>();
		camera.projection();
	}
	
	std::cout << '\n';
	
	try {
		kgr::container container;
		
		container.service<AbstractCameraServiceDefault>();
		
		std::cout << "No exceptions thrown\n";
	} catch(kgr::abstract_not_found const& e) {
		std::cout << "abstract_not_found thrown, what(): " << e.what() << std::endl;
	}
	
	try {
		kgr::container container;
		
		container.service<AbstractCameraService>();
		
		std::cout << "No exceptions thrown\n";
	} catch(kgr::abstract_not_found const& e) {
		std::cout << "abstract_not_found thrown, what(): " << e.what() << std::endl;
	}
}
