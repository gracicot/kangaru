Migration Guide From 3.x.y series to v4.0.0
===========================================

A lot of changes occurred between `3.x.y` series and `4.0.0`.
Many breaking changes has been introduced, and we want the migration to be as smooth as possible.

## Class Names

In kangaru `4.0.0`, we decided to change the naming convention of classes and aliases.
In previous versions of kangaru, it was unclear where `PascalCase` or `snake_case` was used.

Now, every classes and aliases tries to follow the naming scheme of the STL and Boost, so classes and aliases are now in `snake_case`.

Here's a list of all classes and public aliases that changed:

    Abstract                    --> abstract
    AbstractNotFound            --> abstract_not_found
    AbstractService             --> abstract_service
    AbstractSharedService       --> abstract_shared_service
    AdlMap                      --> (deleted)
    All                         --> all
    AnyOf                       --> only
    AutoCall                    --> autocall
    AutoCallNoMap               --> autocall
    Container                   --> container
    ContainerService            --> container_service
    Default                     --> defaults_to
    DefaultForkedInvoker        --> forked_invoker
    DefaultForkedInvokerService --> forked_invoker_service
    DefaultInvoker              --> invoker
    DefaultInvokerService       --> invoker_service
    Dependency                  --> dependency
    FilteredForkService         --> filtered_fork_service
    ForkService                 --> fork_service
    ForkedGenerator             --> forked_generator
    ForkedGeneratorService      --> forked_generator_service
    ForkedInvoker               --> forked_mapped_invoker
    ForkedInvokerService        --> forked_mapped_invoker_service
    ForkedLazy                  --> forked_lazy
    ForkedLazyService           --> forked_lazy_service
    Generator                   --> generator
    GeneratorService            --> generator_service
    GenericService              --> generic_service
    Inject                      --> inject_t
    Invoke                      --> invoke
    Invoker                     --> mapped_invoker
    InvokerService              --> mapped_invoker_service
    Lazy                        --> lazy
    LazyService                 --> lazy_service
    Map                         --> map_t
    NoneOf                      --> except
    Overrides                   --> override
    ServiceType                 --> service_type
    Service                     --> service
    SharedSrevice               --> shared_service
    Single                      --> single
    SingleService               --> single_service
    UniqueService               --> unique_service

### Compatibility Header

If you want to use the newest version of kangaru yet smoothly make the transition from `3.x.y` to `v4.0.0` in your codebase, you can use the compatibility header.

That header will define a set of aliases with the old class names to the new class names.
Simply add `#include <kangaru/compatibility.hpp>` in your codebase, and that will make your old code still compile in most cases.

## Service Map

The service map system has changed drastically. In the `3.x.y` series, the service map was a template class, specialized for every function parameter types.
In version `v3.2.0`, we shipped a service map that looked through ADL to find what parameter corresponded to what service.
It worked really well and was much easier to use.

To make the ADL map more extensible and more powerful, we decided to make it the only way to map service to parameters.
Yet, we wanted to keep the possibility to have many maps to organize services.

So code that used multiple map will have to adapt the service map to use advanced mapping.

**Before:**
```c++
template<typename>
struct MyMap1;

template<typename>
struct MyMap2;

template<>
struct MyMap1<Class1> : kgr::Map<Class1Service> {};

template<>
struct MyMap2<Class2&> : kgr::Map<Class2Service> {};

container.invoke<MyMap1>(someFunction1);
container.invoke<MyMap2>(someFunction2);
```

**After:**
```c++
struct MyMap1;
struct MyMap2;

auto service_map(Class1,  kgr::map_t<MyMap1>) -> Class1Service;
auto service_map(Class2&, kgr::map_t<MyMap2>) -> Class2Service;

container.invoke<kgr::map<MyMap1>>(someFunction1);
container.invoke<kgr::map<MyMap2>>(someFunction2);
```

Also, note that there's the concept of a default map. So that also changed how global service map worked.


**Before:**
```c++
template<typename>
struct MyGlobalMap;

template<>
struct MyGlobalMap<Class1> : kgr::Map<Class1Service> {};

template<>
struct MyGlobalMap<Class2&> : kgr::Map<Class2Service> {};

container.invoke<MyGlobalMap>(someFunction1);
container.invoke<MyGlobalMap>(someFunction2);
```

**After:**
```c++
auto service_map(Class1)  -> Class1Service;
auto service_map(Class2&) -> Class2Service;

container.invoke(someFunction1);
container.invoke(someFunction2);
```

### Inspecting The Service Map

Since the service map was defined by the user, one could inspect the service map to know what parameter was mapped to what service.

We now obtain the mapping through ADL, so inspecting the map now require a tool provided by kangaru.

**Before:**
```c++
template<typename>
struct MyMap;

template<>
struct MyMap<Class> : kgr::Map<ClassService> {};

using MappedService = MyMap<Class>::Service;
```

**After:**
```c++
struct MyMap;

auto service_map(Class, kgr::map<MyMap>) -> ClassService;

using MappedService = kgr::mapped_service_t<Class, MyMap>;
```

Note that if the default map is used, we can omit the second parameter: `kgr::mapped_service_t<Class>`

## Generic Service

If you have defined generic services and used `kgr::GenericService`, you code will break.
This is because the generic service has became much simpler, and don't need CRTP anymore.

**Before:**
```c++
template<typename, typename = kgr::Dependency<>>
struct MyGenericService;

template<typename T, typename... Deps>
struct MyGenericService<T, kgr::Dependency<Deps...>> : kgr::GenericService<MyGenericService<T, kgr::Dependency<Deps...>>, T> {
    // ...
};
```

**After:**
```c++
template<typename, typename = kgr::dependency<>>
struct MyGenericService;

template<typename T, typename... Deps>
struct MyGenericService<T, kgr::dependency<Deps...>> : kgr::generic_service<T> {
    // ...
};
```

Note that this change is not needed when using the `kangaru/compatibility.hpp` header.

## Instance

The Instance functionality has been removed. Now, or has been replaced by `kgr::container::emplace` and `kgr::container::replace`.
Thier functionaly is much more clearly defined and prevent misusing the container.

In `v4.0.0`, you can no longer call the constructor directly, but you can send parameters to the single services construct function using `emplace` or `replace`.

If there's something that cannot be possibly done with only `emplace` or `replace`, please open an issue and we'll discuss a solution for your case.
