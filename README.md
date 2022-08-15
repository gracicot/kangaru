# kangaru 🦘

[![Build status](https://ci.appveyor.com/api/projects/status/8gv9iapt3g7mgc4l?svg=true)](https://ci.appveyor.com/project/gracicot/kangaru) [![All OS](https://github.com/gracicot/kangaru/actions/workflows/all-os.yml/badge.svg)](https://github.com/gracicot/kangaru/actions/workflows/all-os.yml) [![BCH compliance](https://bettercodehub.com/edge/badge/gracicot/kangaru?branch=master)](https://bettercodehub.com/results/gracicot/kangaru) [![Codacy Badge](https://app.codacy.com/project/badge/Grade/f50382d94f894dc981334308a5e709f6)](https://www.codacy.com/gh/gracicot/kangaru/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=gracicot/kangaru&amp;utm_campaign=Badge_Grade) [![Language grade: C/C++](https://img.shields.io/lgtm/grade/cpp/g/gracicot/kangaru.svg?logo=lgtm&logoWidth=18)](https://lgtm.com/projects/g/gracicot/kangaru/context:cpp) [![CII Best Practices](https://bestpractices.coreinfrastructure.org/projects/1401/badge)](https://bestpractices.coreinfrastructure.org/projects/1401) [![Join the chat at https://gitter.im/gracicot/kangaru](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/gracicot/kangaru?utm_source=badge&utm_medium=badge&utm_campaign=pr-badge&utm_content=badge) [![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/gracicot/kangaru/master/LICENSE) [![GitHub Releases](https://img.shields.io/github/release/gracicot/kangaru.svg)](https://github.com/gracicot/kangaru/releases) [![GitHub Issues](https://img.shields.io/github/issues/gracicot/kangaru.svg)](http://github.com/gracicot/kangaru/issues) [![Try online](https://img.shields.io/badge/try-online-blue.svg)](https://wandbox.org/permlink/SVZduLAhH0dACDlj)

Kangaru is an inversion of control container for C++11, C++14 and later. It provides many features to automate dependency injection and reduce the amount of wiring boilerplate in your code. We are achieving that by exposing in code configuration for autowiring, constructor and function parameters injection. We aim to keep the simplest interface possible and keep boilerplate to a minimum. On top of that, we don't want to be intrusive into user/library code.

Kangaru is a header only library because of its extensive use of templates.
The name kangaru comes from the container's feature to inject itself into a service as a dependency, and because kangaroos are awesome.

----

[Documentation and tutorial](https://github.com/gracicot/kangaru/wiki) is in the wiki and the `doc` folder!

Looking for the latest stable version? Check out our [release page](https://github.com/gracicot/kangaru/releases).

## Overview

Here's a quick demo to show usage of this library. This is some basic usage of the library with two user classes.

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

// Scene is not single, so the container returns scenes by value.
// Also, we depend on a camera to be constructed.
struct SceneService : kgr::service<Scene, kgr::dependency<CameraService>> {};

int main()
{
    kgr::container container;
    
    // The service function returns instances of the normal classes.
    Scene scene = container.service<SceneService>();
    Camera& camera = container.service<CameraService>();
    
    assert(&scene.camera == &camera); // passes, both cameras are the same instance.
}
```

[Try this example online](https://wandbox.org/permlink/3ekQZXqTFGRlj8ZG) to see how it runs.

## Autowire API

Since recent versions of kangaru, we support autowire api. The following is the same example as above, using autowire.

```c++
#include <kangaru/kangaru.hpp>
#include <cassert>

// We define some normal classes with dependencies between them
// And we added the autowire configuration
struct Camera {
    // friend functions are faster to lookup than plain free functions
    friend auto service_map(Camera const&) -> kgr::autowire_single;
};

struct Scene {
    Camera& camera;
    
    friend auto service_map(Scene const&) -> kgr::autowire;
};

// No need for service definitions

int main()
{
    kgr::container container;
    
    // We invoke a lambda that receives injected parameters.
    // The container will figure how to wire the classes using
    // either the constructor parameters or aggregate initialization
    container.invoke([](Scene scene, Camera& camera) {
        assert(&scene.camera == &camera); // passes, both cameras are the same instance.
    });
}
```

## Features

* Non intrusive, no existing classes need modification
* You tell the container how to construct your types, store and inject them
* Injection by setters
* Autowiring by class constructors
* Function parameter injection
* Clean and simple API for simple cases, flexible enough for complex cases
* Low runtime overhead
* Header only library
* Clean diagnostics at compile-time

## Installation

### Build kangaru yourself

To make kangaru available on your machine, you must first clone the repository:

```sh
$ git clone https://github.com/gracicot/kangaru.git && cd kangaru
```

Then use cmake to generate the project and export the package information:

```sh
$ cmake --preset export # -DKANGARU_HASH_TYPE_ID=OFF # uncomment for older compiler support
```

That's it! Link it to your project using cmake and you can already include and code!

Optionally, you can also install kangaru on your system:

```sh
$ sudo cmake --build --preset export --target install # optional step
```

### Install with vcpkg

To make kangaru available on your machine, [install vcpkg](https://vcpkg.io/en/getting-started.html). Then install the appropriate architecture. For the default, enter the following:

```cmd
vcpkg install kangaru
```

or if you want 64-bit Windows, for example, enter:

```cmd
vcpkg install kangaru:x64-windows
```

## Adding Include Path

You must use the `find_package` function:

```cmake
find_package(kangaru 4.3 REQUIRED)
```

And then add the include dirs to your target:

```cmake
target_link_libraries(<YOUR TARGET> PUBLIC kangaru::kangaru)
```

Then you can include the library as follows:

```c++
#include <kangaru/kangaru.hpp>
```

If you skip the installation, simply tell CMake where to find kangaru:

```sh
# in your project build directory
$ cmake .. -DCMAKE_PREFIX_PATH=../../path/to/kangaru/build
```

## Compiler Requirement

Kangaru is tested by our continuous integration with all major compiler versions. The minimum required versions are:

* MSVC: 2015 update 3 or better
* GCC: 4.8.5 or better
* Clang: 3.6 or better
* AppleClang: 7.0 or better

## What's Next

There is some feature I would like to see become real. Here's a list of those,
feel free to contribute!

* Tests for compile-time errors
* Better messages for compile-time errors (ongoing)
* Service sources, more detail here: [#41](https://github.com/gracicot/kangaru/issues/41)
* Even better performance (ongoing)
* Expose a zero-overhead interface for cases it can apply
* Move service definitions to service map.

Got suggestions or questions? Discovered a bug? Please open an issue and we'll gladly respond!

## Contributing

To contribute, simply open a pull request or an issue and we'll discuss together about how to make this library even more awesome! See our complete [contribution guideline](https://github.com/gracicot/kangaru/blob/master/CONTRIBUTING.md) for more details.

Want to help? Pick an issue on our [issue tracker](https://github.com/gracicot/kangaru/issues)!

Found an issue? Have an idea to make this library better? Please [submit an issue](https://github.com/gracicot/kangaru/issues/new) and we will respond within a few days, and commit to address the needs.

### Running Tests

Tests are enabled using the cmake profile `dev`. Enabling this will make our CMake scripts to try finding the Catch2 library. We also contain a submodule for this library in our git repository in case you don't have it available in a prefix directory. You can also enable vcpkg to download the dependencies.

Using this option adds the the `test` target.

You can run the tests like this:

```cmake
cmake --preset dev
cmake --build --preset debug
ctest --preset debug
```

## Who's Using Kangaru

Here's a list of projects making use of kangaru:

* Our team's game engine
* The people I helped integrating this library into their projects
* Surely more!

### Using kangaru?

Let me know of your projects using kangaru! I'll be glad to fill the list above with your project's name.

## Acknowledgements

A big thanks to [Louis-Alexandre Vallières-Lavoie](https://github.com/Louis-Alexandre) for reviewing and proposing various improvements to our documentation.
