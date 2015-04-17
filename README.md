Kangaru
=======

Kangaru is a simple dependency injection container library for C++11.
It manages multiple level of dependency. The name Kangaru came from the
feature of injecting itself as a dependency into a service.

Getting Started
---------------

Getting started with Kangaru is easy. First of all, you need to include the
library:

    #include <kangaru.hpp>

Take note that you will need either to add the header to your include paths or
to add it to your project.

Everything is declared in the namespace `kgr`

Documentation and tutorial is in the wiki!

Features
--------

 * Recursive dependency resolution
 * Does not need to modify existing classes
 * Instances shared across every services (Single Services)
 * Providing your own instances
 * Providing callback to construct your services
 * Self-injection
 * Polymorphic resolution of services
 * Clean and simple API
 * Near zero runtime overhead

What's next?
------------

There is some feature I would like to see become real. Here's a list of those,
feel free to contribute!

 * Testing with multiple / virtual inheritance
 * Unit tests
 * CMake
