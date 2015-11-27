Generic Services
================

This section shows you how to create your own generic service class just like `kgr::Service` or `kgr::SingleService`. It's the most advanced part of this documentation. Luckily, you won't have to do it very often.

Well... Let's dive into it shall we?

First, our generic service usually have two template parameter: The sevice class and a dependency class.

    template<typename Type, typename Deps = Dependency<>>

## Option 1: 100% custom implementation

There's nothing to prevent you from reinventing the wheel, as long as the generic definition implement `forward` and `construct` correctly. Good luck.

## Option 2: Extend `kgr::GenericService`

How to use `kgr::GenericService`?
We need to pass it three parameters:

 * The first one is the struct itself. Yes, it's the [CRTP](https://en.wikipedia.org/wiki/Curiously_recurring_template_pattern).
 * The second one is what type the generic service should contain. It can be the service, or a pointer to the service or whatever you want.
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
    
Then, we must implement two method: `makeService` and `forward`.
Forward is the same forward as we know. However, `makeService` is something new. What is it? It's a static function that takes anything as parameter and return a fully constructed definition.

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
    };
    
#### Singles

If the generic definition should be single, simply make it extending from `kgr::Single` and making `forward` virtual.
