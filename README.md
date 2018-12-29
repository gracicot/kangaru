# kangaru [![Build status](https://ci.appveyor.com/api/projects/status/8gv9iapt3g7mgc4l?svg=true)](https://ci.appveyor.com/project/gracicot/kangaru) [![Build Status](https://travis-ci.org/gracicot/kangaru.svg?branch=master)](https://travis-ci.org/gracicot/kangaru) [![BCH compliance](https://bettercodehub.com/edge/badge/gracicot/kangaru?branch=master)](https://bettercodehub.com/results/gracicot/kangaru) [![Join the chat at https://gitter.im/gracicot/kangaru](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/gracicot/kangaru?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge) [![Try online](https://img.shields.io/badge/try-online-blue.svg)](https://wandbox.org/permlink/SVZduLAhH0dACDlj)

Kangaru is an inversion of control container for C++11, C++14 and later. It supports features like operation between containers,
injection via function parameter, automatic call of member functions on instance creation, autowiring and much more!

Our goal is to create a DI container capable of automatic, recusive dependency injection. We also want to do most diagnostics at compile time, while keeping the simplest interface possible. On top of that, we don't want to be intrusive into user/library code.

Kangaru is a header only library because of it's extensive use of templates.
The name kangaru comes from the container's feature to inject itself into a service as a dependency, and because kangaroos are awesome.

----

[Documentation and tutorial](https://github.com/gracicot/kangaru/wiki) is in the wiki and the `doc` folder!

Looking for the latest stable version? Check out our [release page](https://github.com/gracicot/kangaru/releases).

Overview
--------

Here's a quick demo to show usage of this library:
```c++
#include <kangaru/kangaru.hpp>
#include <cassert>

// We define some normal classes with dependencies between them
struct Camera {};

struct Scene {
    Camera& camera;
};

// The following is the configuration of our user classes above.
// The structure and dependency graph is defined by these configs.

// Camera is a single service so the service has a shared instance.
// It will be injected and returned as a reference.
struct CameraService : kgr::single_service<Camera> {};

// Scene is not single, so the container return scenes by value.
// Also, we depends on a camera to be constructed.
struct SceneService : kgr::service<Scene, kgr::dependency<CameraService>> {};

int main()
{
    kgr::container container;
    
    // The service function return instances of the normal classes.
    Scene scene = container.service<SceneService>();
    Camera& camera = container.service<CameraService>();
    
    assert(&scene.camera == &camera); // passes, both cameras are the same instance.
}
```
[Try this example online](https://wandbox.org/permlink/3ekQZXqTFGRlj8ZG) to see how it runs.

Features
--------

 * Non intrusive. No existing classes need modification.
 * You tell the container how to construct your types, store and inject them
 * Injection by setters
 * Autowiring by class constructors
 * Function parameter injection
 * Clean and simple API for simple cases, flexible enough for complex cases
 * Low runtime overhead
 * Header only library
 * Clean diagnostics at compile-time.

Installation
------------
To make kangaru available on your machine, you must clone the repository and create a build directory:

```sh
$ git clone https://github.com/gracicot/kangaru.git && cd kangaru
$ mkdir build && cd build
```

Then use cmake to generate the makefile and export the package informations:

```sh
$ cmake ..
```

That's it! Link it to your project using cmake and you can already include and code!

Optionally, you can also install kangaru on your system:

```sh
$ sudo make install # optional step
```

Adding Include Path
-------------------
You must use the `find_package` function: 

    find_package(kangaru REQUIRED)

And then add the include dirs to your target:

    target_link_libraries(<YOUR TARGET> PUBLIC kangaru)

Then you can include the library as follow:

    #include <kangaru/kangaru.hpp>

Compiler Requirement
--------------------

Kangaru is tested by our continuous integration with all major compiler versions. The minimum required versions are:

 * MSVC: 2015 update 3 or better
 * GCC: 4.8.5 or better
 * Clang: 3.6 or better

What's next?
------------
There is some feature I would like to see become real. Here's a list of those,
feel free to contribute!

 * Tests for compile-time errors
 * Better messages for compile-time errors (ongoing)
 * Service sources, more detail here: [#41](https://github.com/gracicot/kangaru/issues/41)
 * Even better performance (ongoing)

Got suggestions or questions? Discovered a bug? Please open an issue and we'll gladly respond!

Contributing
------------
To contribute, simply open a pull request or an issue and we'll discuss together about how to make this library even more awesome! See our complete [contribution guideline](https://github.com/gracicot/kangaru/blob/master/CONTRIBUTING.md) for more details.

Want to help? Pick an issue on our [issue tracker](https://github.com/gracicot/kangaru/issues)!

Found an issue? Have an idea to make this library better? Please [submit an issue](https://github.com/gracicot/kangaru/issues/new) and we will respond within a few days, and commit to address the needs.

For security related problems, please submit us an issues with the _security_ tag. If any private code/information need to be transmitted, a main contributor will transmit the instruction in the issue.

Who's using kangaru?
--------------------
Here's a list of projets making use of kangaru
 - _\<to be filled>_
   
#### Using kangaru?

Let me know of your projects that uses kangaru! I'll be glad to fill the list above with your project's name.

Acknowledgements
----------------
A big thanks to [Louis-Alexandre Vallières-Lavoie](https://github.com/Louis-Alexandre) for reviewing and proposing various improvement to our documentation.
