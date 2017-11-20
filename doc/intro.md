Intro
=====

Welcome to our documentation!

#### First, what is kangaru?

Kangaru is an inversion of control container for C++11 and later. We support features like operation between containers,
injection via function parameter, automatic call of member function on instance creation and much more!

Our goal is to create a container capable of automatic, recusive dependency injection that do most diagnostics at compile time,
while keeping the simplest interface possible, and all that without being intrusive into user/library code.

Kangaru is a header only library because of it's extensive use of templates.
The name kangaru comes from the container's feature to inject itself into a service as a dependency, and because kangaroos are awesome.

#### Inversion of control that puts you back in control

Indeed, the container does not impose a way to construct, contain or inject your classes. You are in full control of everything related to the classes of your project.

Control over memory allocation in containers is not yet supported, but is planned: #41. We do use `std::unordered_map` and `std::vector` with the default allocators.

Getting Started
---------------

To make kangaru available on your machine, you must clone the repository and create a build directory:

    $ git clone https://github.com/gracicot/kangaru.git && cd kangaru
    $ mkdir build && cd build

Then use cmake to generate the makefile and export the package informations:

    $ cmake ..

Tou can then use cmake to find the package and add include paths: 

    find_package(kangaru REQUIRED)
    target_link_libraries(<YOUR TARGET> PUBLIC kangaru)

Then, you can simply include the library:

    #include <kangaru/kangaru.hpp>

Take note that you will need to add the library to your include paths.

All declarations are made in the namespace `kgr`. Additionnaly, the namespace `kgr` contains the namespace `detail`, which itself contains implementation details.
Note that the `detail` namespace is not considered as a part of the API and its content might be subject to changes.

### Concepts

In this documentation, many classes will be refered as services or service definitions.

_Services_ are classes that are either injected by the container or have other classes as dependencies.

_Service Definitions_ are classes that contain a service and tell the container how this particular service should behave within the container.

### Macros

This library does not make use of macros to prevent multiple inclusion.
Every macros that starts with `KGR_KANGARU_` is considered reserved.
Note that some features of this library are easier to use with macros, and we recommend you to use those that are defined in the documentation.

Macros defined by the library are not part of it's interface.

### Compiler Support

 - MSVC: 2015 update 3 or better
 - GCC: 5.4 or better
 - Clang: 3.6 or better

Index
-----
 * [Services](section1_services.md)
 * [Basic usage of the container](section2_container.md)
 * [Override Services](section3_override.md)
 * [Invoke](section4_invoke.md)
 * [Operator services](section5_operator.md)
 * [Autocall](section6_setters.md)
 * [Custom service definitions](section7_definitions.md)
 * [Debugging](section8_debug.md)
 * [Generic Services](section9_generic.md)
 * [Structure for large projects](section10_structure.md)
