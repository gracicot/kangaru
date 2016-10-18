Custom Service Definitions
==========================

Previously, each time we did a service definition, we extended the `kgr::Service` or the `kgr::SingleService` structs. But it is important to note that we are not limited to that.

Writing a service definition is pretty easy. You only need to define three functions for it to work:

 * `construct`
 * `forward`
 * A constructor that takes at least `kgr::in_place_t` as parameter.

The `construct` function is static and returns the arguments that should be used to construct your service definition. It can take any other service definition as a parameter.
This is where dependencies are resolved. This function must return values using the `kgr::inject` function. The return type is a special tuple made for the container to handle your arguments correctly.

The `forward` function takes no parameters and returns the service. The return type of this function defines how your service should be injected. Note that this function can invalidate the service definition in the case of a non single service.

The container will instanciate your struct given these tools. However, it must call a constructor. The container will call a constructor that takes a `kgr::in_place_t` and the variables returned by `construct`.

Normally, your service definition should contain your service.

Please note that the following example will use C++14 for the sake of simplicity and readability. Kangaru only require C++11 to work.
A basic service definition looks like this:

```c++
struct FileManagerService {
    FileManagerService(kgr::in_place_t) : fm{} {}

    static decltype(auto) construct() {
        return kgr::inject();
    }
    
    FileManager forward() {
        return std::move(fm);
    }
    
private:
    FileManager fm;
};
```

In this case, `construct` injects nothing to our constructor.

If the class `FileManager` has dependencies, you can add them in the `construct` function as parameters. To add a parameter that must be injected, you must use the wrapper class `kgr::Inject<Definition>`:

```c++
struct FileManagerService {
    FileManagerService(kgr::in_place_t, Notification& n, ClownManster cm) : fm{n, std::move(cm)} {}
    
    static decltype(auto) construct(kgr::Inject<NotificationService> ns, kgr::Inject<ClownMasterService> cms) {
        return kgr::inject(ns.forward(), cms.forward());
    }
    
    // return as move, invalidating the service definition is okay.
    FileManager forward() {
        return std::move(fm);
    }
    
private:
    FileManager fm;
};
```
    
The `kgr::Inject<Definition>` will not only tell the container that this arguments must be injected, but ensure that singles are not copied and overrides are working correctly.
The container will call `construct` with the right set of parameters, automatically.
The `kgr::inject` function will forward the arguments to be sent to your constructor that receive a `kgr::in_place_t`.

#### Additional parameters

Sometimes, a type requires a parameter like a double or a string. The `construct` function can take as many additional parameters as you want. Just make the construct function a template instead:

For this service definition:

```c++
struct FileManagerService {
    template<typename... Args>
    FileManagerService(kgr::in_place_t, Args&&... args) : fm{std::forward<Args>(args)...} {}
    
    template<typename... Args>
    static decltype(auto) construct(kgr::Inject<NotificationService> ns, kgr::Inject<ClownMasterService> cms, Args&&.. args) {
        return kgr::inject(ns.forward(), cms.forward(), std::forward<Args>(args)...);
    }
    
    // return as move, invalidating the service definition is okay.
    FileManager forward() {
        return std::move(fm);
    }
    
private:
    FileManager fm;
};
```

You have to call it that way:

```c++
auto fm = container.service<FileManagerService>("potatos", 34);
```

### Singles

There are two steps required in order to make `FileManagerService` single. First, we need to make our struct inherit from `kgr::Single`. Secondly, we also need to adapt the `forward` function by returning a reference or a copy in order to not invalidate the contained service.

Note: single services can forward copies too, but you rarely want to do that. Returning a reference or pointer is a much better idea when it comes to single service, what would be the point of a single instance if you copy the service everywhere?

So let's make our sevice a Single:

```c++
struct FileManagerService : Single {
    template<typename... Args>
    FileManagerService(kgr::in_place_t, Args&&... args) : fm{std::forward<Args>(args)...} {}
    
    template<typename... Args>
    static decltype(auto) construct(kgr::Inject<NotificationService> ns, kgr::Inject<ClownMasterService> cms, Args&&.. args) {
        return kgr::inject(ns.forward(), cms.forward(), std::forward<Args>(args)...);
    }
    
    // return as pointer because we want. Must not invalidate the service because it's a single.
    FileManager* forward() {
        return &fm;
    }
    
private:
    FileManager fm;
};
```

### Abstract Services

Abstract services are the simplest ones to implement. They have only one pure virtual method called `forward`:

```c++
struct IFileManagerService {
    virtual IFileManager& forward() = 0;
}
```
    
Abstract services are implicitly single.

[Next chapter](section8_generic.md)
