kangaru
=======

kangaru is a dependency injection container library for C++11.
It manages multiple level of dependency. The name Kangaru came from the
feature of injecting itself as a dependency into a service.

[Documentation and tutorial](https://github.com/gracicot/kangaru/wiki) is in the wiki and the `doc` folder!

Installation
------------
To install kangaru on a machine, you must create a build directory:

    mkdir build
    cd build
Then use cmake to generate the makefile:

    cmake ..
    sudo make install

Adding Include Path
-------------------
You must use the `find_package` function: 

    find_package(kangaru REQUIRED)
    
And then add the include dirs to your target:

    target_link_libraries(<YOUR TARGET> PUBLIC kangaru)
    
Then you can include the library as follow:

    #include <kangaru/kangaru.hpp>

Features
--------

 * Recursive dependency resolution
 * Does not need to modify existing classes
 * Instances shared across every services (Single Services)
 * You tell the container how to construct your types
 * You tell the container how to store them
 * You tell the container how they are injected
 * Injection by setters
 * Clean and simple API
 * Low runtime overhead
 * Header only library

What's next?
------------

There is some feature I would like to see become real. Here's a list of those,
feel free to contribute!

 * Testing with multiple / virtual inheritance
 * Unit tests
 * Better messages for compile-time errors
 * Better resource management
 * Operation on containers
