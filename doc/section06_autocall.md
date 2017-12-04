Autocall
========

Sometime only constructing a service is not enough. Some classes needs configuration, or some function to be called initially, or even call some setters.

The autocall feature is doing exacly that. It's a list of function to call upon the construction of a service.
The container will invoke all function in that list. You can use this feature to perform injection through setters.


First of all, we recommend defining a shortcut for using `kgr::invoke`. Of you have C++17 enabled, you can define that shortcut like that:
```c++
template<auto m>
using method = kgr::method<decltype(m), m>;
```

Alternatively, if you only have C++11 or C++14 on hand, you can use a macro:
```c++
#define METHOD(...) ::kgr::Method<decltype(__VA_ARGS__), __VA_ARGS__>
```

Of course, you are free to name them as you want.
One your shotcut is declared, let's get started!

## Enabling Autocall

Any service can extends the class `kgr::autocall`. This class enable the needed metadata for the container to call the methods you want to be called.

The `kgr::autocall` type is a list of method to call in your class. More specifically, a list of `kgr::method`.

Let's see an example of it's usage. So we have this class:

```c++
struct MessageBus {
    void init() {
        max_delay = 42;
    }
    
private:
    int max_delay;
};
```
    
If we want `init()` to be called at the service's construction, we need our definition to extends `kgr::autocall`:

```c++
struct MessageBusService : kgr::service<MessageBus>, kgr::AutoCall<METHOD(&MessageBus::init)> {};
```

Great! Now creating the service will call that function:

```c++
// MessageBus::init is called before returning
MessageBus md = container.service<MessageBusService>();
```

## Parameters

Here comes the fancy thing. Functions listed in `kgr::autocall` can receive other services.
In fact, the container will call these functions using the `invoke` function. So you can receive any other services.

For example, we need `max_delay` to be calculated with values that comes from other service.
So here's our class according to the new need:

```c++
struct MessageBus {
    void init(Window& window) {
        max_delay = 3 * window.get_framerate();
    }
    
private:
    int max_delay;
};
```

That's it! You can add any number of parameter as you wish, the definition will stay the same and the method will receive what you ask for.

## Multiple methods

As said before, `kgr::autocall` is a list of method to call. You can have as many method to call as you wish
```c++
struct MessageBus {
    void init(Window& window, Camera& camera) {
         max_delay = 3 * window.get_framerate();
    }
    
    void set_scene(Scene& scene) {
        this->scene = &scene;
    }
    
private:
    Scene* scene;
    int max_delay;
};

struct MessageBusService : kgr::service<MessageBus>, kgr::autocall<
    METHOD(&MessageBus::init),
    METHOD(&MessageBus::set_scene)
> {};
```

The functions are called in the order that are listed in `kgr::autocall`.

## Specifying The Service Map

In previous examples, we used the default service map. If you deal with advanced mapping, you might want to specity which maps to use.
You can set the default map to use in the first parameter of autocall:

```c++
struct MyMap;

struct MessageBusService : kgr::service<MessageBus>, kgr::autocall<
    kgr::map<MyMap1, MyMap2>,
    METHOD(&MessageBus::init),
    METHOD(&MessageBus::set_scene)
> {};
```

## Specifying Services

Alternatively, you can list needed sevices for every methods. Parameters are grouped within the `kgr::invoke` class:

```c++
struct MessageBusService : kgr::service<MessageBus>, kgr::autocall<
    kgr::invoke<METHOD(&MessageBus::init), WindowService, CameraService>,
    METHOD(&MessageBus::set_scene)
> {};
```

[Next chapter: Operator Services](section07_operator.md)
