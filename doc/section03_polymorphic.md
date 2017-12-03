Polymorphic Services
====================

Until now, we've seen how to inject classes into other one in many ways. However, every type were static.
For example, we did not have a subclass for `Camera` we can inject instead of a camera.

In this section, we will see how to override a single service to make the container call the subclass's service instead of the base class one.

## How Overriding Services Works

When querying for a service, the container will check in his collection of definitions.
After that, it simply ask the definition to extract the service from it to return it to the caller.

For a service that can be overriden, the container will do the same process, but the function that extract the data from the definition polymorphically using function pointers.
That way, when we save a derived service into the container, it will also save itself as a base. So when searching for the base, it will find derived instance.

## A Hierarchy of Cameras

Until now, our camera class was a regular object, not tied to a hierarchy. We will change that to include two other camera type.

```c++
struct Camera {
	virtual void projection() {
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
```
Now, we need to reflect that in kangaru service definitions. By default, the container won't assume you need polymorphic behaviour.
The container will simply get the service you asked for and get the data inside. This allows the container to optimize querying services.

To make the container aware of the polymorphic setting, you must tag the definition as polymorphic, as well as marking derived as overriders:

```c++
struct CameraService : kgr::single_service<Camera>, kgr::polymorphic {};

struct PerspectiveCameraService : kgr::single_service<PerspectiveCamera>, kgr::overrides<CameraService> {};
struct OrthogonalCameraService  : kgr::single_service<OrthogonalCamera>,  kgr::overrides<CameraService> {};
```

With these tags on, the container knows to treat the camera as polymorphic, and will properly override the `CameraService` when any of the other camera are saved into the container.

## Using Services Polymorphically

Services can be used polymorphically when instances are involved.
The mechanism is this: when the instance of an overrider is created, it will register itself as it's parent service.

Here's one example of polymorphic usage and the container:

```c++
kgr::container c;

// Camera is returned
Camera& camera1 = container.service<CameraService>();
camera1.projection(); // prints `default projection`

// PerspectiveCamera is registered as Camera
container.emplace<PerspectiveCameraService>();

// PerspectiveCamera is returned
Camera& camera2 = container.service<CameraService>();
camera2.projection(); // prints `perspective projection`
```

## Order matters

Let's take a look at the case where both `PerspectiveCamera` and `OrthogonalCamera` are used.
Since services that overrides simply register itself as the service it overrides, it may replace a service that was already overriden before.
So for example I have two services that overrides `CameraService`, the last one inserted into the container will effectively override `CameraService`

Look at these two cases:

```c++
{
    kgr::container container;
    
    container.service<PerspectiveCameraService>();
    container.service<OrthogonalCameraService>();
    
    // instance of OrthogonalCamera returned
    Camera& fm = container.service<CameraService>();
}

{
    kgr::container container;
    
    container.service<OrthogonalCameraService>();
    container.service<PerspectiveCameraService>();
    
    // instance of PerspectiveCamera returned
    Camera& fm = container.service<CameraService>();
}
```

As we can see, when using polymorphic service, the order of insertion into the container can change behavior.

## Abstract Service

If you want to make a service definition for an abstract type, or simply make a service only instanciable using derived services, you may inherit from `kgr::abstract_service<T>`:

```c++
struct IFileManagerService : kgr::abstract_service<IFileManager>;
```

Abstract services are service that cannot be constructed by themselves, to have in instance of it we must first create an instance of a service that overrides it.

If the container cannot find the instance of an abstract service, it will throw a `kgr::abstract_not_found`.

```c++
{
	kgr::container container;
	
	// throws kgr::abstract_not_found
	IFileManager& fm = container.service<IFileManagerService>();
}

{
	kgr::container container;

	container.emplace<TreeFileManagerService>();
	
	// returns the TreeFileManager instance
	IFileManager& fm = container.service<IFileManagerService>();
}
```

### Default Service Type

If we cannot afford to throw, and yet want to use abstract services, we can define a default service.
When asking the container for an abstract service and no instance is found, it will fallback to the default service instead of throwing.

```c++
struct IFileManagerService : kgr::abstract_service<IFileManager>, kgr::defaults_to<TreeFileManagerService> {};

kgr::container c;

// Will instantiate a TreeFileManager and return a IFileManager.
IFileManager& fileManager = c.service<IFileManagerService>();
```

[Next chapter: Managing Containers](section04_container.md)
