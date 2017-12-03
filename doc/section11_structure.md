Structuring projects
====================

Have a large codebase? Want to structure your code correctly and scale better?
Here's some suggestions to keep usage of kangaru in your projects well organised!

## Definition completeness

When you use a service definition, it must be complete. For a sevice definition to be valid, it's service must be complete too!

*How can we keep a fast compilation and minimize dependencies?*

For services in your own project, keeping one definition in each header will make including only services you need will be easier.
Multiple unrelated definitions per header file will prevent you from including only what you need.
Also, in a service definition, other definitions used should be complete. Incomplete service may result in errors when using that service.

Headers of your classes should not be significantly changed when integrating kangaru to your project.
The big change is in the cpp file where you actually use the container.
If you need to deal with the container, you should include the definition instead of the class you want to use.

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
    | kangaru.hpp |<------- |
    ---------------       | |
                          | |
    ------------     --------------
    | ClassC.h |<----| ClassC.cpp |
    ------------     --------------
    
As we can see, the files of `ClassA` and `ClassB` are unchanged and are not dependent of kangaru. The only thing changed is the addition of `ClassAService.h` and `ClassBService.h`.
This has the effect of reducing coupling considerably: only code that uses kangaru will depend on it.
If we were to remove kangaru from this project, the only changed thing would be to reimplement the desired logic in `ClassC`.

## Fork the container

If you have services that are tied together in a logical unit, and you only need to access some of those elsewhere,
don't be afraid to fork the container!

Having only one huge for everything is considered harmful to some extent.
Having a container with thousands of services in it may slow your application.
If you want your application to scale, prefer separating containers to contain only services that are useful in their context.

## Minimize coupling to the container

It may sound funny, but if you can, try not using the container directly.
Having a lot of classes that uses the container will make coupling less controllable.
Kangaru offers operator services that are meant to express your intent about how you plan to use the container.
If you can drop usage of the container, then do it! This library is a great tool to minimize coupling,
but coupling with this library is still coupling.

If on the other hand using the container (or operator services) in a particular place
reduces unwanted coupling with other thing, then don't be afraid and use the container.

## Including kangaru

We would recommend to not include `kangaru.hpp` directly, and use a proxy header file instead. Why? Because right now `kgr::AutoCall` is easier to use with a macro, and you can also include your own generic services that you want to use throughout your application.
A "include kangaru" header file should look like this:

```c++
#pragma once // or a header guard.

#include <kangaru/kangaru.hpp> // include kangaru

// Here you can put your generic service.
// Example:
// #include "sharedservice.h"
// #include "uniqueservice.h"

// You can optionally include `compatibility.hpp`
// #include <kangaru/compatibility.hpp>

// declare some recommended shortcut macros
#define METHOD(...) ::kgr::Method<decltype(__VA_ARGS__), __VA_ARGS__>

// Or if you have C++17 available:
// template<auto F>
// using invoke = kgr::invoke<decltype(F), F>;
```

This will add a common point between your project and kangaru.
Feel free to copy this header into your project. Just replace the macro name to fit your needs and you're ready to hack!

[Next chapter: API Reference](section12_api_reference.md)
