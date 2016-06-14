Generic Services
================

This section shows you how to create your own generic service class just like `kgr::Service` or `kgr::SingleService`. It's the most advanced part of this documentation. Luckily, you won't have to do it very often.

Well... Let's dive into it, shall we?

First, our generic service usually have two template parameter: The sevice class and a dependency class.

```c++
template<typename, typename>
struct MyUniqueService;
```

## Option 1: 100% custom implementation

There's nothing that prevents you from reinventing the wheel, as long as the generic definition implements `forward` and `construct` correctly.
Please note that some feature like `autocall` will not work with custom implementation unless implemented in a similar way.

## Option 2: Extend `kgr::GenericService`

How to use `kgr::GenericService`?
We need to pass it two parameters:

 * The first one is the struct itself. Yes, it's the [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern).
 * The second one is what type the generic service should contain. It can be the service, or a pointer to the service or anything you want.
 
```c++
template<typename, typename>
struct MyUniqueService;

template<typename Type, typename... Deps>
struct MyUniqueService<Type, kgr::Dependency<Deps...>> : kgr::GenericService<MyUniqueService<Type, kgr::Dependency<Deps...>>, std::unique_ptr<Type>> {
    
};
```

Then, we must implement three methods: `construct`, `forward` and `call`.

There's a function we haven't seen before. What is this mysterious `call` function? It's a function to call a method from a given instance.
It must receive for the first parameter is a reference to the instance as it is contained within the `GenericService`. The second parameter is a pointer to the method to be called. Then the other parameters are the parameter pack to forward to the method.
The most basic implementation:

```c++
template<typename T, typename... Args>
static decltype(auto) call(Type& instance, T method, Args&&... args) {
    return (instance.*method)(std::forward<Args>(args)...);
}
```
    
_`decltype(auto)` is a c++14 feature even if only c++11 is required. It is used for the sake of simplicity and is not required._

Now let's see how they look like in `MyUniqueService`!

### Full example
 
```c++
template<typename, typename>
struct MyUniqueService;

template<typename Type, typename... Deps>
struct MyUniqueService<Type, kgr::Dependency<Deps...>> : kgr::GenericService<MyUniqueService<Type, kgr::Dependency<Deps...>>, std::unique_ptr<Type>> {
    template<typename... Args>
    static auto construct(kgr::Inject<Deps>... deps, Args&&... args)
            -> decltype(kgr::inject(std::make_unique<Type>(deps.forward()..., std::forward<Args>(args)...))) {
        return kgr::inject(std::make_unique<Type>(deps.forward()..., std::forward<Args>(args)...));
    }

    std::unique_ptr<Type> forward() {
        return std::move(this->getInstance());
    }
    
    template<typename T, typename... Args>
    static decltype(auto) call(std::unique_ptr<Type>& instance, T method, Args&&... args) {
        return ((*instance).*method)(std::forward<Args>(args)...);
    }
};
```

### Using the class

You can now use your class as the following:

```c++
struct MyClassService : MyUniqueService<MyClass, kgr::Dependency<MyDependencyService>> {};
```

#### Singles

If the generic definition should be single, simply make it extend from the `kgr::Single` struct.
Take note that you don't need any virtual function. The override behaviour is completely implemented inside the container.

[Next chapter](section9_structure.md)
