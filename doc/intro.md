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

You can then use cmake to find the package and add include paths: 

    find_package(kangaru REQUIRED)
    target_link_libraries(<YOUR TARGET> PUBLIC kangaru)

Then, you can simply include the library:

    #include <kangaru/kangaru.hpp>

Take note that you will need to add the library to your include paths.

All declarations are made in the namespace `kgr`. Additionnaly, the namespace `kgr` contains the namespace `detail`, which itself contains implementation details.
Note that the `detail` namespace is not considered as a part of the API and its content might be subject to changes.

### Services, Definitions, Oh My!

In this documentation, many classes will be refered as services or service definitions.

_Services_ are classes that are either injected by the container or have other classes as dependencies.

_Service Definitions_ are classes that contain a service and tell the container how this particular service should behave within the container.

### About Macros

This library make use of macros to prevent multiple inclusion.
Every macros that starts with `KGR_KANGARU_` is considered reserved for implementation.
Macros defined by the library are not part of it's interface.

Note that [some features](section07_autocall.md) of this library may be easier to use with macros, especially before C++17. We recommend you to define some for your own usage if you feel the need.

### Compiler Requirement

 - MSVC: 2015 update 3 or better
 - GCC: 4.8.5 or better
 - Clang: 3.6 or better

### Backward Compatibility

In between minor versions, we make our best to remain source compatible and not break any code using kangaru. Our continuous integration tests is our primary mean to guarantee this statement. If a breaking change is introduced in a non-breaking release, please submit an issue and we'll discuss how we can solve this.

Note that we don't recommend forward declaring types from the `kgr` namespace, as we reserve the right to change a type to an alias in minor vesions. Since forward declaring an alias is not a thing in C++, this may create some incompatibility. Some details in the build system might also change between minor version, but we will not break documented usage of the kangaru cmake package or it's generation. 

In between major versions, we still try our best to make the transition to future version as smooth as possible. To do this, we provide a migration guide to help developpers and we make our best to not break basic usage of the library.

Breaking versions might bump compiler requirements and might also bump language version requirement. Note that when this happen, we are willing to offer support for older versions if there's a demand for it.

### Migration Guide

Since many breaking changes has been introduced in `v4.0.0`, we decided to write a [migration guide from the `3.x.y` series to `v4.0.0`](migration_guide_3xy.md).
We also added a compatibility header to make the transition smoother between the two versions.

Index
-----
 * [Services](section01_services.md)
 * [Invoke](section02_invoke.md)
 * [Polymorphic Services](section03_polymorphic.md)
 * [Managing Containers](section04_container.md)
 * [Supplied services](section05_supplied.md)
 * [Autowire](section06_autowire.md)
 * [Autocall](section07_autocall.md)
 * [Operator services](section08_operator.md)
 * [Custom service definitions](section09_definitions.md)
 * [Advanced Mapping](section10_mapping.md)
 * [Debugging](section11_debug.md)
 * [Generic Services](section12_generic.md)
 * [Structuring projects](section13_structure.md)
 * [API Reference](section14_api_reference.md)
