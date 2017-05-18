Invoke
======

Invoke is one handy feature. It calls a function that receives services with the right set of parameters.

Let's say we have this function:

```c++
int doThings(Notification n, FileManager& fm);
```
    
Then you can use the container to call the function:

```c++
int result = container.invoke<NotificationService, FileManagerService>(doThings);
```

This is good, it will work but we can make it better.

The `invoke` function can found what parameters the function needs.
But we need a way to associate parameters to services. Introducing the service map:

```c++
auto service_map(Argument) -> Service;
```
    
The service map is a function that takes a parameter to be mapped, and has the associated service as return type.
Each entry must be in the same namespace as the argument it receives. The container will search the right `service_map` function through ADL.

For each service in which you want to be able work with `invoke` without explicitely listing every services, declare the following entries:

```c++
auto service_map(const Notification&) -> NotificationService;
auto service_map(const FileManager&) -> FileManagerService;
```

If you want, you can also receive the parameter `kgr::Map<>` to desambiguate the function if you already had one named like this:

```c++
auto service_map(const Notification&, kgr::Map<>) -> NotificationService;
auto service_map(const FileManager&, kgr::Map<>) -> FileManagerService;
```

> Note that you wont need to define the function themselves, as the container only use them to check the return type.

Now that our service map is defined, we can use invoke like this:

```c++
int doThings(Notification n, FileManager& fm);

// ...

int result = container.invoke(doThings);
```

> Note that for version `v3.2.1`, abstract services are abstract classes. Since you cannot have a function signature that has an abstract type as return type, mapping your service to `kgr::Map<your-abstract-service>`. It will be fixed in `v3.3.0`. For more detail, see [#35](https://github.com/gracicot/kangaru/issues/35) and [#37](https://github.com/gracicot/kangaru/issues/37)

---

Alternatively, if you don't want to use ADL to map arguments to their services, you can define your own service map using a template struct and specialise it for every service:

```c++
template<typename>
struct MyMap;

template<> struct MyMap<Notification> : kgr::Map<NotificationService> {};
template<> struct MyMap<FileManager&> : kgr::Map<FileManagerService> {};
```

> Note that this way of defining the service map is deprecated, and likely removed in future major releases.

> Note the presence of the `&` after `FileManager`. This is because `FileManager`, `FileManager&` and `const FileManager*` could all be bound to different service definitions.
> Take note that you must declare the service map yourself.

Using this syntax, you must send your service map when invoking:

```c++
int doThings(Notification n, FileManager& fm);

// ...

int result = container.invoke<MyMap>(doThings);
```

With the service map, the container can even call function from third party libraries!

## Additional parameters

Just like the `service()` function, `invoke()` can receive additional parameters to be sent after dependencies.
Here's an emaxple:

```c++
int doThings(Notification n, FileManager& fm, int a, double b);

int result = container.invoke(doThings, 7, 8.9);
```

C++14 generic lambda are also supported. The only restriction is that all `auto` must be at the end. Just like this:
```c++
auto doThings = [](Notification n, FileManager& fm, int a, auto b){};

int result = container.invoke(doThings, 7, 8.9); // auto as been deduced to be `double`
```

This however, is not supported:
```c++
auto doThings = [](Notification n, FileManager& fm, auto a, double b){};

int result = container.invoke(doThings, 7, 8.9); // error, not invokable
```

[Next chapter: Operator services](section5_operator.md)
