#include <iostream>
#include <string>

#include <kangaru/kangaru.hpp>

/**
 * This example refect snippets of code found in the documentation section 8: Custom Definitions
 * It explains how to make your own service definition without using generic ones from kangaru.
 */

// User classes.
struct Window {};
struct Camera {};
struct BasicFileManager {
	BasicFileManager() {
		std::cout << "BasicFileManager constructed\n";
	}
};
struct FileManager {
	FileManager(Window&, Camera, int parameter = 0) {
		std::cout << "Parameter: " << parameter << '\n';
	}
};

// Service definitions
struct WindowService : kgr::single_service<Window> {};
struct CameraService : kgr::service<Camera> {};


// Custom defintion. We kind of reinplement what `kgr::service` is doing
struct BasicFileManagerService {
	BasicFileManagerService(kgr::in_place_t) : instance{} {
		std::cout << "BasicFileManagerService::BasicFileManagerService called\n";
	}
	
	static auto construct() -> kgr::inject_result<> {
		std::cout << "BasicFileManagerService::construct called\n";
		return kgr::inject();
	}
	
	// return as move, invalidating the service definition is okay since it's not single and won't be reused.
	BasicFileManager forward() {
		return std::move(instance);
	}
	
private:
	BasicFileManager instance;
};

// Custom defintion. We kind of reinplement what `kgr::single_service` is doing,
// and we defined injected parameter in construct.
// Parameter returned from construct are sent to the constructor
struct FileManagerService : kgr::single {
	FileManagerService(kgr::in_place_t, Window& w, Camera c) : instance{w, std::move(c)} {}
	
	static auto construct(kgr::inject_t<WindowService> ws, kgr::inject_t<CameraService> cs)
		-> kgr::inject_result<kgr::service_type<WindowService>, kgr::service_type<CameraService>>
	{
		std::cout << "FileManagerService::construct called\n";
		return kgr::inject(ws.forward(), cs.forward());
	}
	
	FileManager forward() {
		return std::move(instance);
	}
	
private:
	FileManager instance;
};

// and we defined injected parameter in construct.
// Parameter returned from construct are sent to the constructor
// We are recieving a int parameter, which must be sent from the caller.
struct FileManagerParamService {
	FileManagerParamService(kgr::in_place_t, Window& w, Camera c, int p) : instance{w, std::move(c), p} {}
	
	static auto construct(kgr::inject_t<WindowService> ws, kgr::inject_t<CameraService> cs, int p)
		-> kgr::inject_result<kgr::service_type<WindowService>, kgr::service_type<CameraService>, int>
	{
		std::cout << "FileManagerParamService::construct called\n";
		return kgr::inject(ws.forward(), cs.forward(), p);
	}
	
	FileManager forward() {
		return std::move(instance);
	}
	
private:
	FileManager instance;
};

// and we defined injected parameter in construct.
// Parameter returned from construct are sent to the constructor
// We are recieving a parameter pack, which can be empty or parameters.
struct FileManagerParamTemplService {
	template<typename... Args>
	FileManagerParamTemplService(kgr::in_place_t, Args&&... args) : instance{std::forward<Args>(args)...} {}
	
	template<typename... Args>
	static auto construct(kgr::inject_t<WindowService> ws, kgr::inject_t<CameraService> cs, Args&&... args)
		-> kgr::inject_result<kgr::service_type<WindowService>, kgr::service_type<CameraService>, Args...>
	{
		std::cout << "FileManagerParamTemplService::construct called\n";
		return kgr::inject(ws.forward(), cs.forward(), std::forward<Args>(args)...);
	}
	
	FileManager forward() {
		return std::move(instance);
	}
	
private:
	FileManager instance;
};

int main() {
	kgr::container container;
	
	// BasicFileManagerService::construct called, then the FileManager constructor
	BasicFileManager fm1 = container.service<BasicFileManagerService>();
	
	std::cout << '\n';
	
	// BasicFileManagerService::construct called, then the FileManager constructor
	FileManager fm2 = container.service<FileManagerService>();
	
	std::cout << '\n';
	
	// BasicFileManagerService::construct called, then the FileManager constructor
	FileManager fm3 = container.service<FileManagerParamService>(2);
	
	std::cout << '\n';
	
	// BasicFileManagerService::construct called, then the FileManager constructor
	FileManager fm4 = container.service<FileManagerParamTemplService>(2); // Args is int
	FileManager fm5 = container.service<FileManagerParamTemplService>(); // Args is empty
}
