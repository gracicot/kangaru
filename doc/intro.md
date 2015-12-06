Intro
=====

Welcome to our documentation!

#### First, what is kangaru?

Kangaru is a simple inversion of control container for C++11 and later. Our goal is to create a container capable of automatic dependency injection that do most diagnostics at compile time, while keeping the simplest interface possible without modifying existing classes. Kangaru is a header only library because of it's extensive use of templates. The name kangaru come from the feature of the container injecting itself into a service as a dependency.

#### Inversion of control that put you back in control

Indeed, the container does not force a way to construct, allocate memory, containing or inject your classes. You are in control of everything related to the classes of your project.

Getting Started
---------------

Getting started with Kangaru is easy. First of all, you need to include the library:

    #include "kangaru.hpp"

Take note that you will need to add the header to your include paths.

Every declarations are made in the namespace `kgr`. Additionnaly, the namespace `kgr` is containing the namespace `detail` which contains implementation detail.
Note that the `detail` namespace is not considered part of the API and its content might be subject to changes.

### Wording

In this documentation, many classes will be reffered as services and service definitions.

_Service_ classes are classes that either is injected by the container or has other classes as dependency.
_Service Definition_ classes are a class that contain a service and tell the container how to related service should behave within the container.

### Macros

This library do not make use of macros and do not declare any.
Note that some features of this library are easier to use with macros and we recomend you to follow how this documentation define them.

Index
-----
 * [Services](section1_services.md)
 * [Basic usage of the container](section2_container.md)
 * [Override](section3_override.md)
 * [Invoke](section4_invoke.md)
 * [Injection with setters](section5_setters.md)
 * [Writing service definitions from scratch](section6_definition.md)
 * [Generic Services](section7_generic.md)
 * [Structure for large projects](section8_structure.md)
