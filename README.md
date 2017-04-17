kangaru
=======

kangaru is a dependency injection container library for C++11.
It manages recursive dependency injection, injection into function parameter, and more.
The name Kangaru came from the feature of injecting itself as a dependency into a service.

[Documentation and tutorial](https://github.com/gracicot/kangaru/wiki) is in the wiki and the `doc` folder!

Installation
------------
To make kangaru available on a machine, you must create a build directory:

	cd kangaru
    mkdir build
    cd build

Then use cmake to generate the makefile and export the package:

    cmake ..
    
Optionally, you can also install kangaru on your system:
    
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
 * You tell the container how to construct your types
 * You tell the container how to store them
 * You tell the container how they are injected
 * Injection by setters
 * Clean and simple API
 * Low runtime overhead
 * Header only library
 * Clean diagnostics at compile-time.

What's next?
------------

There is some feature I would like to see become real. Here's a list of those,
feel free to contribute!

 * Testing with multiple / virtual inheritance
 * Unit tests
 * Better messages for compile-time errors
 * Service sources
