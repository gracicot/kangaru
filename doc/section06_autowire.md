Autowire Services
=================

As from now we configured our services definition with a specific set of dependencies.

kangaru offers a way to automatically inject dependencies into a service.

We will start with this simple example:

```c++
struct Camera {
    int position;
};

struct Scene {
	// We contain a reference to a camera
    Camera& camera;
    int width = 800;
    int height = 600;
};
```

And we will see how to autowire these services together!

## From Dependencies to Autowiring

With fixed dependencies, our service definitions are specefied like this:

```c++
struct CameraService : kgr::single_service<Camera> {};
struct SceneService : kgr::service<Scene, kgr::dependency<CameraService>> {};
```

To enable autowiring, we must map constructor parameters to services.
Luckily, just like with invoke, we can define the service map:

```c++
auto service_map(Camera const&) -> CameraService;
auto service_map(Scene const&) -> SceneService;
```

The container will use these functions to get which service definition will yield the requested parameter.

Then, instead of using `kgr::dependency`, we will use `kgr::autowire` just like that:

```c++
struct CameraService : kgr::single_service<Camera> {};
struct SceneService : kgr::service<Scene, kgr::autowire> {};
```

Done! The container will now construct the scene with a camera.

## Some Aliases!

The autowiring example above is great, but could be simpler. Kangaru defines service types that are automatically autowired.
Here's a list of them:

 * `kgr::autowire_service<T>`, an alias to `kgr::service<T, kgr::autowire>`
 * `kgr::autowire_single_service<T>`, an alias to `kgr::single_service<T, kgr::autowire>`
 * `kgr::autowire_unique_service<T>`, an alias to `kgr::unique_service<T, kgr::autowire>`
 * `kgr::autowire_shared_service<T>`, an alias to `kgr::shared_service<T, kgr::autowire>`

This might simplify some service declaration and help readability.

## Some... Magic?

We wanted to make autowiring the simplest possible.
This is why we also added a way to generate service definition through the service map.

If a mapping yields `kgr::autowire`, the container will generate a service definition for us that uses autowire.

Here's the example using this feature:

```c++
struct Camera {
    int position = 0;
};

struct Scene {
	// We contain a reference to a camera
    Camera& camera;
    int width = 800;
    int height = 600;
};

auto service_map(Camera const&) -> kgr::autowire_single;
auto service_map(Scene const&) -> kgr::autowire;

int main()
{
	kgr::container container;
	
	container.invoke([](Camera& c, Scene s) {
		// Works!
	});
}
```

Here we cna see the absence of service definition.
This is because while invoking the lambda, the container uses the mapping above,
generate definitions and autowire these services!

As you can notice, the mapping for `Camera` yields a `kgr::autowire_single`. This is because we want the container
to generate a definition of a single service for our camera.
There is `kgr::autowire_unique` and `kgr::autowire_shared` of you want those kind if service to be generated.

If you want to get the generated service definition type, you can use the mapping:

```c++
using CameraService = kgr::mapped_service_t<Camera>;

// Or even
Camera& camera = container.service<kgr::mapped_service_t<Camera>>();
```

Also, this is possible to fill the service map with friend functions:

```c++
struct Camera {
    int position;
    
    friend auto service_map(Camera const&) -> kgr::autowire_single;
};

struct Scene {
	// We contain a reference to a camera
    Camera& camera;
    int width = 800;
    int height = 600;
    
    friend auto service_map(Scene const&) -> kgr::autowire;
};
```

Just with these two lines, both classes can now be used with the container just as any other services.

## Mapping

The service map allows you to have multiple named maps. And sometimes, a service is only mapped in one specific map.

To tell autowire which map to read, we added alternative types that allows you to specify the map:

```c++
struct CameraService : kgr::single_service<Camera, kgr::mapped_autowire<kgr::map<MyMap1, MyMap2>>> {};
struct SceneService : kgr::autowire_service<Scene, kgr::map<MyMap1>> {};

auto service_map(Window const&) -> kgr::mapped_autowire<kgr::map<MyMap2>> {};
```

Of course, all those aliases are also defined for unique and shared services.

## Not Almighty

The autowire feature had to be implemented without reflection. Because we cannot know the amount of parameters a service
need to take, we had to define a maximum. With the current implementation, this maximum is defined to be `8`.

However, that maximum is not fixed. With both `kgr::autowire_service` and `kgr::mapped_autowire` have a supplementary parameter
that allow setting the maximum amount of deduced services:


```c++
struct CameraService : kgr::single_service<Camera, kgr::mapped_autowire<kgr::map<>, 10>> {};
struct SceneService : kgr::autowire_service<Scene, kgr::map<>, 12> {};

auto service_map(Window const&) -> kgr::mapped_autowire<kgr::map<>, 3> {};
```

Autowiring is a really powerful tool that can simplify both your code, but your interaction with kangaru in general.
It enable you to change constructors as needed without any refactor. As long as your services are mapped, the container
will be able to autowire any services!

To see more about autowired services, please see [example6](../examples/example6/example6.cpp).

[Next chapter: Autocall](section07_autocall.md)
