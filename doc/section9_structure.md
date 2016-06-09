Have a large codebase? Big plans? Here's some suggestions to keep your project well organised!

## Definition completeness

When you use a service definition, it must be complete. For a sevice definition to be valid, it's service must be complete too!

*How can we keep a fast compilation and minimize dependencies?*

For services in your own project, keep one definition in each header. Multiple definitions per header file will prevent you from including only what you need.
Also, in a service definition, other definitions used should be complete too.

Headers of your classes should not be significantly changed. The big change is in the cpp file. If you need to deal with the container, you must include the definition instead of the class you want to use.

Let's say we have a project that has the classes `ClassA` and `ClassB`. We want to add `ClassC` that will use kangaru.
Here's a include graph of this project:

    // ClassAService.h and ClassBService.h includes kangaru.hpp
    
    ------------     --------------
    | ClassA.h |<----| ClassA.cpp |
    ------------     --------------
         ^
         |
         |           -------------------
         |-----------| ClassAService.h |<------
         |           -------------------      |
         |                                    |
         |__________________                  |
                            |                 |
                            |                 |
    ------------     --------------           |
    | ClassB.h |<----| ClassB.cpp |           |
    ------------     --------------           |
         ^                                    |
         |                                    |
         |           -------------------      |
         ------------| ClassBService.h |-------
                     -------------------
                            ^
                            |
    ---------------         |
    | kangaru.hpp |<--------|
    ---------------         |
                            |
    ------------     --------------
    | ClassC.h |<----| ClassC.cpp |
    ------------     --------------
    
As we can see, the files of `ClassA` and `ClassB` are unchanged and are not dependent of kangaru. The only thing changed is the addition of `ClassAService.h` and `ClassBService.h`.
This has the effect of reducing coupling considerably: only code that uses kangaru will depend on it.
If we were to remove kangaru from this project, the only changed thing would be to reimplement the desired logic in `ClassC`.

## Including kangaru

We would recommend to not include `kangaru.hpp` directly, and use a proxy header file instead. Why? Because you must define the service map, and maybe include your own generic services.
A "include kangaru" header file should look like this:

```c++
#pragma once // or a header guard.

#include "kangaru/kangaru.hpp" // include kangaru

// Here you can put your generic service.
// Example:
// #include "sharedservice.h"
// #include "uniqueservice.h"

// declare some needed macros
#define METHOD(...) ::kgr::Method<decltype(__VA_ARGS__), __VA_ARGS__>

namespace <your-namespace> {

// declare the service map
template<typename>
struct ServiceMap;

// specializing the service map for container services.

template<> struct ServiceMap<kgr::Container&> : kgr::Map<kgr::ContainerService> {};
template<> struct ServiceMap<kgr::Container> : kgr::Map<kgr::DefaultForkService> {};

// specializing the service map for operator services.

template<template<typename> class Map>
struct ServiceMap<kgr::Invoker<Map>> : kgr::Map<kgr::InvokerService<Map>> {};

template<template<typename> class Map>
struct ServiceMap<kgr::ForkedInvoker<Map>> : kgr::Map<kgr::ForkedInvokerService<Map>> {};

template<typename T>
struct ServiceMap<kgr::Generator<T>> : kgr::Map<kgr::GeneratorService<T>> {};

template<typename T>
struct ServiceMap<kgr::ForkedGenerator<T>> : kgr::Map<kgr::ForkedGeneratorService<T>> {};

template<typename T>
struct ServiceMap<kgr::Lazy<T>> : kgr::Map<kgr::LazyService<T>> {};

template<typename T>
struct ServiceMap<kgr::ForkedLazy<T>> : kgr::Map<kgr::ForkedLazyService<T>> {};

} // <your-namespace>
```

This will add a common point between your project and kangaru.
Feel free to copy this header into your project. Just replace `<your-namespace>` by the name of your namespace and you are ready to hack!
