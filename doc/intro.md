Intro
=====

Welcome to our documentation!

#### First, what is kangaru?

Kangaru is a simple inversion of control container for C++11 and later. Our goal is to create a container capable of automatic dependency injection that do most diagnostics at compile time, while keeping the simplest interface possible, and all that without modifying existing classes. Kangaru is a header only library because of it's extensive use of templates. The name kangaru comes from the container's feature that consists in injecting itself into a service as a dependency.

#### Inversion of control that puts you back in control

Indeed, the container does not impose a way to construct, allocate memory, contain or inject your classes. You are in full control of everything related to the classes of your project.

Getting Started
---------------

Getting started with Kangaru is easy. First of all, you need to include the library:

    #include "kangaru.hpp"

Take note that you will need to add the header to your include paths.

All declarations are made in the namespace `kgr`. Additionnaly, the namespace `kgr` contains the namespace `detail`, which itself contains implementation details.
Note that the `detail` namespace is not considered as a part of the API and its content might be subject to changes.

### Wording

In this documentation, many classes will be refered as services or service definitions.

_Services_ are classes that are either injected by the container or have other classes as dependencies.
_Service Definitions_ are classes that contain a service and tell the container how this particular service should behave within the container.

### Macros

This library does not make use of macros and does not declare any.
Note that some features of this library are easier to use with macros, and we recommend you to use those that are defined in the documentation.

Index
-----
 * [Services](section1_services.md)
 * [Basic usage of the container](section2_container.md)
 * [Override](section3_override.md)
 * [Invoke](section4_invoke.md)
 * [Injection with setters](section5_setters.md)
 * [Writing service definitions from scratch](section6_definitions.md)
 * [Generic Services](section7_generic.md)
 * [Structure for large projects](section8_structure.md)
