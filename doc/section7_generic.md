Generic Services
================

This section shows you how to create your own generic service class just like `kgr::Service` or `kgr::SingleService`. It's the most advanced part of this documentation. Luckily, you won't have to do it very often.

Well... Let's dive into it, shall we?

First, our generic service usually have two template parameter: The sevice class and a dependency class.

    template<typename Type, typename Deps = Dependency<>>

## Option 1: 100% custom implementation

There's nothing that prevents you from reinventing the wheel, as long as the generic definition implements `forward` and `construct` correctly. Good luck.
Please note that some feature like `autocall` will not work with custom implementation unless implemented in a similar way.

## Option 2: Extend `kgr::GenericService`

How to use `kgr::GenericService`?
We need to pass it three parameters:

 * The first one is the struct itself. Yes, it's the [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern).
 * The second one is what type the generic service should contain. It can be the service, or a pointer to the service or anything you want.
 * The third one are the dependencies.
 
    template<typename Type, typename Deps = kgr::Dependency<>>
    struct MyGeneric : kgr::GenericService<Service<Type, Deps>, Type, Deps> {
        
    };

Next, we need to use the alias `Self` inside the `GenericService` and use the constructor: 
 
    template<typename Type, typename Deps = kgr::Dependency<>>
    struct MyGeneric : GenericService<Service<Type, Deps>, std::unique_ptr<Type>, Deps> {
        private: using Parent = GenericService<Service<Type, Deps>, std::unique_ptr<Type>, Deps>;
    
    public:
        using typename Parent::Self;
        using Parent::Parent;
    };
    
Then, we must implement three methods: `makeService`, `forward` and `call`.
Forward is the same forward as we know. However, `makeService` is something new. What is it? It's a static function that takes anything as parameter and returns a fully constructed definition. The parent class `GenericService` will use this method to construct the service.
It usually looks like this:

    template<typename... Args>
    static Self makeService(Args&&... args) {
        return Self{ Type{std::forward<Args>(args)...} };
    }

There's Another function we haven't seen before. What is this mysterious `call` function? It's a function to call a method from a given instance.
It must receive for the first parameter is a reference to the instance as it is contained within the `GenericService`. The second parameter is a pointer to the method to be called. Then the other parameters are the parameter pack to forward to the method.
The most basic implementation:

    template<typename T, typename... Args>
    static decltype(auto) call(Type& instance, T method, Args&&... args) {
        return (instance.*method)(std::forward<Args>(args)...);
    }
    
_`decltype(auto)` is a c++14 feature even if only c++11 is required. It is used for the sake of simplicity and is not required._

Now let's see how they look like in `MyGeneric`!

### Full example
 
    template<typename Type, typename Deps = kgr::Dependency<>>
    struct MyGeneric : GenericService<Service<Type, Deps>, std::unique_ptr<Type>, Deps> {
        private: using Parent = GenericService<Service<Type, Deps>, std::unique_ptr<Type>, Deps>;
    
    public:
        using typename Parent::Self;
        using Parent::Parent;
        
        template<typename... Args>
        static Self makeService(Args&&... args) {
            return Self{ new Type{std::forward<Args>(args)...} };
        }

        std::unique_ptr<Type> forward() {
            return std::move(this->getInstance());
        }
        
        template<typename T, typename... Args>
        static decltype(auto) call(std::unique_ptr<Type>& instance, T method, Args&&... args) {
            return ((*instance).*method)(std::forward<Args>(args)...);
        }
    };
    
#### Singles

If the generic definition should be single, simply make it extend the `kgr::Single` struct and make `forward` virtual.

[Next chapter](section8_structure.md)
