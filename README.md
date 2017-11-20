kangaru
=======

[![Build status](https://ci.appveyor.com/api/projects/status/8gv9iapt3g7mgc4l?svg=true)](https://ci.appveyor.com/project/gracicot/kangaru)
[![Build Status](https://travis-ci.org/gracicot/kangaru.svg?branch=dev-4.0.x)](https://travis-ci.org/gracicot/kangaru)
[![BCH compliance](https://bettercodehub.com/edge/badge/gracicot/kangaru?branch=master)](https://bettercodehub.com/)

Kangaru is an inversion of control container for C++11 and later. We support features like operation between containers,
injection via function parameter, automatic call of member function on instance creation and much more!

Our goal is to create a container capable of automatic, recusive dependency injection that do most diagnostics at compile time,
while keeping the simplest interface possible, and all that without being intrusive into user/library code.

Kangaru is a header only library because of it's extensive use of templates.
The name kangaru comes from the container's feature to inject itself into a service as a dependency, and because kangaroos are awesome.


[Documentation and tutorial](https://github.com/gracicot/kangaru/wiki) is in the wiki and the `doc` folder!

```c++
#include <kangaru/kangaru.hpp>
#include <iostream>

struct Camera {};

struct Scene {
    Scene(Camera& c) : camera{c} {}
    
    Camera& camera;
};

struct CameraService : kgr::single_service<Camera> {};
struct SceneService : kgr::service<Scene, kgr::dependency<CameraService>> {};

int main()
{
    kgr::container container;
    
    auto scene = container.service<SceneService>();
    auto& camera = container.service<CameraService>();
    
    std::cout
        << std::boolalpha
        << (&scene.camera == &camera) << std::endl; // outputs true
}
```

Features
--------

 * Recursive dependency resolution
 * Non intrusive. No existing classes need modification.
 * You tell the container how to construct your types
 * You tell the container how to store them
 * You tell the container how they are injected
 * Injection by setters
 * Function parameter injection
 * Clean and simple API
 * Low runtime overhead
 * Header only library
 * Clean diagnostics at compile-time.

Installation
------------
To make kangaru available on your machine, you must clone the repository and create a build directory:

    $ git clone https://github.com/gracicot/kangaru.git && cd kangaru
    $ mkdir build && cd build

Then use cmake to generate the makefile and export the package informations:

    $ cmake ..

That's it! Link it to your project using cmake and you can already include and code!

Optionally, you can also install kangaru on your system:

    # make install

Adding Include Path
-------------------

You must use the `find_package` function: 

    find_package(kangaru REQUIRED)

And then add the include dirs to your target:

    target_link_libraries(<YOUR TARGET> PUBLIC kangaru)

Then you can include the library as follow:

    #include <kangaru/kangaru.hpp>

What's next?
------------

There is some feature I would like to see become real. Here's a list of those,
feel free to contribute!

 * Tests for compile-time errors
 * More test coverage
 * Better messages for compile-time errors (ongoing)
 * Service sources, more detail here: [#41](https://github.com/gracicot/kangaru/issues/41)

Got suggestions or questions? Discovered a bug? Please open an issue and we'll gladly respond!

Contributing
------------
To contribute, simply open a pull request or an issue and we'll discuss together about how to make this library even more awesome!

Want to help? Pick an issue on our [issue tracker](https://github.com/gracicot/kangaru/issues)!
