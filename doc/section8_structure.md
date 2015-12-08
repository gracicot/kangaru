Structure for large projects
============================

Have a large codebase? Big plans? Here's some suggestion to keep your project well done!

## Definition completeness

When you use a service definition, it must be complete. And for a sevice definition to be valid, it's service must be complete too!

*How can we keep a fast compilation and minimize dependencies?*

For services in your own project, keep one definition in each header. Multiple definition per header file will prevent you from including only what you need.
Also, in a service definition, other definition used should be complete too.

Headers of your classes should not be significantly changed. The big change is in the cpp file. If you need to deal with the container, you must include the definition instead of the class you want to use.

## Including kangaru

I'd recommend not including `kangaru.hpp` directly and use a proxy header file instead. Why? Because you must define the service map, and maybe including your own generic service.
A "include kangaru" header file should look like this:

    #pragma once // or a header guard.

    #include "kangaru/kangaru.hpp" // include kangaru

    // Here you can put your generic service.
    #include "sharedservice.h"
    #include "uniqueservice.h"

    // declare some needed macros
    #define METHOD(...) decltype(__VA_ARGS__), __VA_ARGS__
    #define INVOKE(...) ::kgr::Method<decltype(__VA_ARGS__), __VA_ARGS__>

    // declare the service map
    template<typename>
    struct ServiceMap;

