Custom Service Definitions
==========================

Previously, each time we did a service definition, we extended the `kgr::service` or the `kgr::single_service` classes.
But it is important to note that we are not limited to that.

Writing a service definition is pretty easy. We only need to define three functions for it to work with the container:

 * `construct`
 * `forward`
 * A constructor that takes at least `kgr::in_place_t` as parameter.

The `construct` function is static and returns the arguments that should be used to construct your service definition.
It can take any other service definition as a parameter.
This is where dependencies are resolved.
This function must return values using the `kgr::inject` function.
The return type is a special tuple made for the container to handle your arguments correctly.

The `forward` function takes no parameters and returns the service.
The return type of this function defines how your service should be injected.
Note that this function can invalidate the service definition in the case of a non single service.

The container will instanciate your definition given these tools.
However, it must call a constructor. The container will call a constructor that takes a `kgr::in_place_t`
and the variables returned by `construct`.

Normally, your service definition should contain your service.

A basic service definition looks like this:

```c++
struct FileManagerService {
    FileManagerService(kgr::in_place_t) : instance{} {}

    static auto construct() -> kgr::inject_result<> {
        return kgr::inject();
    }
    
    // return as move, invalidating the service definition is okay since it's not single and won't be reused.
    FileManager forward() {
        return std::move(instance);
    }
    
private:
    FileManager instance;
};
```

In this case, `construct` injects nothing to our constructor.

If the class `FileManager` has dependencies, you can add them in the `construct` function as parameters.
To add a parameter that must be injected, you must use the wrapper class `kgr::inject_t<Definition>`:

```c++
struct FileManagerService {
    FileManagerService(kgr::in_place_t, Window& w, Camera c) : instance{w, std::move(c)} {}
    
    static auto construct(kgr::inject_t<WindowService> ws, kgr::inject_t<CameraService> cs)
        -> kgr::inject_result<kgr::service_type<WindowService>, kgr::service_type<CameraService>>
    {
        return kgr::inject(ws.forward(), cs.forward());
    }
    
    FileManager forward() {
        return std::move(instance);
    }
    
private:
    FileManager instance;
};
```
    
The `kgr::inject_t<Definition>` will not only tell the container that this arguments must be injected,
but ensure that singles are not copied and overrides are working correctly.
The container will call `construct` with the right set of parameters, automatically.
The `kgr::inject` function will forward the arguments to be sent to your constructor that receive a `kgr::in_place_t`.

The return type `kgr::inject_result` is an alias to the return type of the `kgr::inject` function.
Note that if you're using C++14 and later, you can use return type deduction `auto`, since `kgr::inject` always return a value.

#### Additional parameters

You may have a service that requires a parameters to be sent fwom the call site.
The `construct` function can take as many additional parameters as you want.
In fact, the forwarded parameters sent to `container.service<T>(...)` are directly sent to the construct function.

To be able to forward any parameter from the `container.service<T>(...)` to our constructor, we can simply define the construct as a template function:

```c++
struct FileManagerService {
    template<typename... Args>
    FileManagerService(kgr::in_place_t, Args&&... args) : instance{std::forward<Args>(args)...} {}
    
    template<typename... Args>
    static construct(kgr::inject_t<WindowService> ws, kgr::inject_t<CameraService> cs, Args&&... args)
        -> kgr::inject_result<service_type<WindowService>, service_type<CameraService>, Args...>
    {
        return kgr::inject(ws.forward(), cs.forward(), std::forward<Args>(args)...);
    }
    
    FileManager forward() {
        return std::move(instance);
    }
    
private:
    FileManager instance;
};
```

Then, we can send additional parameters to the service function:

```c++
auto fm = container.service<FileManagerService>("another parameter", 34);
```

### Singles

There are two steps required in order to make `FileManagerService` single.

First, we need to make our struct inherit from `kgr::single`.
That class is simply a tag that tell the container that our definition is single.

Secondly, we also need to adapt the `forward` function by returning a reference or a copy
in order to not invalidate the contained service, since the definition may be reused.

So let's make our sevice a single:

```c++
struct FileManagerService : kgr::single {
    template<typename... Args>
    FileManagerService(kgr::in_place_t, Args&&... args) : instance{std::forward<Args>(args)...} {}
    
    template<typename... Args>
    static construct(kgr::inject_t<WindowService> ws, kgr::inject_t<CameraService> cs, Args&&... args)
        -> kgr::inject_result<service_type<WindowService>, service_type<CameraService>, Args...>
    {
        return kgr::inject(ws.forward(), cs.forward(), std::forward<Args>(args)...);
    }
    
    // return as pointer because we want our service to be injected as a pointer into other services
    FileManager* forward() {
        return &instance;
    }
    
private:
    FileManager instance;
};
```

### Abstract Services

Abstract services are the simplest ones to implement. They have only one method called `forward`, which is undefined, and they inherits from `kgr::abstract`:

```c++
struct IFileManagerService : kgr::abstract {
    IFileManager& forward();
}
```

Abstract services are implicitly single and polymorphic.

To see more implementation of custom services, check out [example8](../examples/example8/example8.cpp).

[Next chapter: Advanced Mapping](section09_mapping.md)
