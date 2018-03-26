Supplied Services
=================

Indeed, when a service is single, `service()` won't accept argument to forward to the constructor.
This is because only the first call to `service()` creates an instance. Consider this code (bad example):

```c++
// Bad :(
Scene& scene1 = c.service<SceneService>(1024, 768);
Scene& scene2 = c.service<SceneService>(1920, 1080); // Oops! Constructor not called! The instance is reused.

// The scene still have a resolution of 1024x768
```

As a matter of fact, the `service` function has no mean to tell if the service has actually been created here.
To resolve this, you can instead request the constuction of a service with the `emplace(...)` function.
That function will perform injection like `service()`, but serves only to save a single into the container.
Just like `std::map`, emplace only perform initialization if the container don't contain and instance yet.
It returns `true` if the service is created, and `false` if there's already an instance in the container:

```c++
bool inserted = c.emplace<SceneService>(1920, 1080); // (1)
assert(inserted); // Passes.

inserted = c.emplace<SceneService>(1024, 768);
assert(inserted); // Fires, not created two times.

Scene& scene = container.service<SceneService>(); // Returns the instance created at (1)
```

As we can see, `emplace` only constructs if the element is not found.
But as opposed to `service`, it can reports if it has already been inserted before.
This gives us the opportunity to handle those case correctly.

## Supplied Services

As `container.service<SceneService>()` requires the `SceneService` to be constructible using only it's dependencies, the following `Scene` declaration won't work:

```c++
struct Scene {
    Scene(Camera c, int w, int h) :
        camera{c}, width{w}, height{h} {}
    
private:
    Camera camera;
    int width;
    int height;
};

Scene& scene = container.service<SceneService>(); // fails!
```

It's constructor must receive two additional integers after it's dependencies.
Since `container.service<SceneService>()` must create a scene and does not forward any arguments to single services,
there are no way to obtain a scene from the container!

In these cases, we must tell the container that it's normal it cannot construct it without arguments,
and must be provided to the container before usage.

```c++
struct SceneService : kgr::single_service<Scene, kgr::dependency<CameraService>>, kgr::supplied {};
```

Now, we can use the service like this:

```c++
container.emplace<SceneService>(1920, 1080); // contruct a scene in the container.

Scene& scene = container.service<SceneService>(); // works, won't try to construct it.
```

Note that if the instance is not found, the container won't be able to construct it and will throw a `kgr::supplied_not_found` instead.

## Replace Services

The `emplace` function will only construct a service is it's not in the container yet. But what if you wanted to replace an existing service?

The container give you a way to explicitly replace a single service to it would be used in future injection. It's usage is similar to emplace:

```c++
kgr::container container;

container.emplace<SceneService>(640, 480);
Scene& scene1 = container.service<SceneService>();
container.replace<SceneService>(1920, 1080);
Scene& scene2 = container.service<SceneService>();

assert(&scene1 != &scene2); // passes, these are different instances of scenes
```

Note that when replacing, the container will not destroy the old service. In our example, `scene1` is still valid after `replace` has been called.

To see more about supplied services, please see [example5](../examples/example5/example5.cpp).

[Next chapter: Autocall](section06_autocall.md)
