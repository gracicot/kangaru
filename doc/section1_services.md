Services
========

A service is simply a fancy name for one of your class that can be used by the container.
Service are the building block of injection with this library. Every injectable type is a service.

The container won't use your classes directly. It instead uses a proxy called the definition of a service.
A definition contains the config and metadata for the container to use your class the desired way.
For example, of you want your class to have a shared instance between injections, you simply opt-in for it in the definition.

tl;dr:
The definition of a service is the place where we define how the service is contained in the container, how it's constructed and how it's injected into other services.

## Your first service

In order to transform a class into a service, you wont need to modify your class. All you have to do, is to make a new definition, or new config for your class.
Let's start with a simple case. Let's say we have a camera class we want to turn into a service:

```c++
struct Camera {
    int position = 0;
};
```

All we have to do is declare the following service definition:

```c++
struct CameraService : kgr::service<Camera> {};
```

We made it! Now we can use the container to create a camera. The container has a function called `service`.
That function will return an instance of the class specified in `CameraService`, thus `Camera` in our case:

```c++
kgr::container container;
Camera camera = container.service<CameraService>();
```

Yay! The container created an instance of `Camera` through our service definition.

Also, arguments sent to the `service` function are forwarded to the service's constructor:

```c++
Camera furtherCamera = container.service<CameraService>(14);

// furtherCamera.position == 14
```

You might wonder why we are doing this. Why using an indirect way to construct our simple classes?
As we are using other functionalities of kangaru, we'll quickly see why.

## Dependencies

The above example weren't so useful by itself. Let's add another service. Now, we want to make a `Scene` class that uses a camera.

```c++
struct Scene {
    Camera camera;
    int width = 800;
    int height = 600;
};
```

Now instead of sending an instance of a camera into our scene, let's express this as a dependency between our services.
This will make the container aware of the link between our classes so the container will create one instance of camera, and inject it into the scene.

Let's make our definition:

```c++
struct SceneService : kgr::service<Scene, kgr::dependency<CameraService>> {};
```

The second argument to the `kgr::service` class is the dependencies of the service.

Now, since we expressed our dependency there, we don't need to explicitly create a camera and send it;
instead, the container will take of the depdency without the construction site to be aware:

```c++
// A camera is created and sent into the scene.
Scene scene = container.service<SceneService>();
```

As before, we can forward arguments to the constructor of `Scene` note that we still don't need to send the camera there.

```c++
// A camera is created and sent into the scene.
// The scene has a size of 1920x1080
Scene scene = container.service<SceneService>(1920, 1080);
```

Now we can clearly see the point of using the container to create classes that have dependencies.
The usage site of the scene class don't need an instance of a camera in hand to create a scene.
Also, the day you need additional dependencies, you won't need to refactor every place we construct a scene!

## Encapsulation

In the previous example, we did not bother making our members private.
Making them so is not a limitation. We can define our class like we're used to:

```c++
struct Scene {
    Scene(Camera c, int w = 800, int h = 600) :
        camera{c}, width{w}, height{h} {}
    
private:
    Camera camera;
    int width;
    int height;
};
```

Note that this change in our class have no impact on the definition.
Since our class has the exact same semantics as before, the container will continue to call `Scene{camera}` without it being aware of the change.

## Single Services

Single services are kind the point why we need a container to resolve dependencies, and not just free functions.

There are few key difference with single services as opposed to regular services.
They are created one time at the first call to `service()`, and then contained inside the container.
The container will then reuse the instance for all the next injections.

Also, since the constructor will only be called at the first injection, or the first `service()` call, argument forwading is disabled.

Now, let's say we want only one scene in our application. We want the same scene to be injected and returned by the container.
We will do that by inheriting from the `kgr::single_service` definition.
That definition tells the container to reuse the instance and inject as a reference.

```c++
struct SceneService : kgr::single_service<Scene, kgr::dependency<CameraService>> {};
```

That's it! A reference is now returned by the container:

```c++
Scene& scene1 = c.service<SceneService>();
Scene& scene2 = c.service<SceneService>();

assert(&scene1 == &scene2); // passes! Both scenes are the same object.
```
## Single as Dependency

Single can be injected as dependencies too. Consider this screen class and definition:

```c++
struct Screen {
    Scene& scene;
};

struct ScreenService : kgr::service<Screen, kgr::dependency<SceneService>> {};
```

Since a scene is injected by reference, every instance of `Screen` will hold to same scene:

```c++
Screen screen1 = container.service<ScreenService>();
Screen screen2 = container.service<ScreenService>();

assert(&screen1.scene == &screen2.scene); // Passes! Same scene injected into both screens!
```

In that code, maky things happened. At the first call of `service()`, a screen must be created.
The container will first need to create a scene, since no scene has been created yet.
But for a scene to be created, the container will have to create a camera. So in the end, a camera is first created, then a scene is created with that camera and we save the scene into the container, and then our screen is created with that scene.

For the second call, the service finds the saved camera, so it simply create a screen with that camera.

---

This is the most basic usage of kangaru. We achieved recursive dependency resolution and single instances.
With only that, many use cases are covered and may already be useful. But don't stop there! The fun has just begun!
In the next chapter, we'll see how to manage many containers and perform operation between them.

[Next chapter: Basic usage of the container](section2_container.md)
