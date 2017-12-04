Generic Services
================

This section shows you how to create your own generic service class just like `kgr::service` or `kgr::single_service`.
It's the most advanced part of this documentation, and optional for everyday use of this library.
You won't have to do this unless you have very specific needs.

Well... Let's dive into it, shall we?

First, our generic service usually have two template parameter: The service class and a dependency class.

```c++
template<typename, typename>
struct MyUniqueService;
```

## Extending kgr::generic_service

Generic service is a class that will be default constructible, even if the contained service type is not.
This will make user definition able to omit inheriting constructors.

To use generic service, we only need to pass one parameter, the contained service type. It can be the service, a pointer to the service or any other way you want to contain the service type.

```c++
template<typename, typename>
struct MyUniqueService;

template<typename Type, typename... Deps>
struct MyUniqueService<Type, kgr::dependency<Deps...>> : kgr::generic_service<std::unique_ptr<Type>> {
    
};
```

Then, we must implement three member functions: `construct`, `forward` and `call`.

There's a function we haven't seen before. What is this mysterious `call` function? It's a function to call a function on the contained instance. It is used by `kgr::autocall`.

The first parameter is a pointer to the method to be called.
Then it recieves a pack of parameters as argument to the function to be called.

Here's the typical implementation:

```c++
template<typename T, typename... Args>
static decltype(auto) call(Type& instance, T method, Args&&... args) {
    return (instance().*method)(std::forward<Args>(args)...);
}
```
    
_`decltype(auto)` is a c++14 feature even if only c++11 is required. It is used for the sake of simplicity and is not required._

Now let's see how they look like in `MyUniqueService`!

## Full example

```c++
template<typename, typename>
struct MyUniqueService;

template<typename Type, typename... Deps>
struct MyUniqueService<Type, kgr::dependency<Deps...>> : kgr::generic_service<std::unique_ptr<Type>> {
    template<typename... Args>
    static auto construct(kgr::inject_t<Deps>... deps, Args&&... args)
        -> kgr::inject_result<std::unique_ptr<Type>>
    {
        return kgr::inject(std::make_unique<Type>(deps.forward()..., std::forward<Args>(args)...));
    }

    std::unique_ptr<Type> forward() {
        return std::move(this->instance());
    }
    
    template<typename T, typename... Args>
    static decltype(auto) call(T method, Args&&... args) {
        return ((*this->instance()).*method)(std::forward<Args>(args)...);
    }
};
```

### Using The Generic Definition

You can now use your class as the following:

```c++
struct MyClassService : MyUniqueService<MyClass, kgr::dependency<MyDependencyService>> {};
```

## Singles

If the generic definition should be single, simply make it extend from the `kgr::single` tag class.

[Next chapter: Structuring projects](section12_structure.md)
