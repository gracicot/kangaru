sioc
=====
sioc, is a simple dependency injection container library for C++11. It manages multiple level of dependency, and can even inject himself into a service!
sioc stands for "Simple Inversion Of Control"

Getting Started
---------------
Getting started with sioc is easy. First of all, you need to include the library:

    #include <sioc.hpp>

Take note that you will need either to add the header to your include paths or to add it to your project.
Every declarations are made in the namespace sioc.
The namespace sioc is containing the namespace detail which contains implementation detail.

For more tutorial and more details, please visit the wiki!

Features
--------
 * Recursive dependency resolution
 * Does not need to modify existing classes
 * Instances shared accross every services (Single Services)
 * Providing your own instances
 * Providing callback to construct your services
 * Self-injection
 * Polymorphic resoltuion of services
 * Clean and simple API

What's next?
------------
There is some feature I would like to see become real. Here's a list of those, feel free to contribute!
 * Testing with mutliple / virtual inheritance
 * Have callback to initialize services
 * Cleanup the code
 * Unit tests
 * CMake
