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

// This macro is used as a shortcut to use kgr::Method. Won't be needed in C++17
#define METHOD(...) ::kgr::method<decltype(__VA_ARGS__), __VA_ARGS__>

// The following classes are user classes.
// As you can see, this library is not intrusive and don't require modifications
struct Credential {};

struct Connection {
    // The connect needs some credential
    void connect(Credential const&) {
        std::cout << "connection established" << std::endl;
    }
};

struct Database {
    // A database needs a connection
    Database(Connection const&) {
        std::cout << "database created" << std::endl;
    }

    // For the sake of having a method to call
    void commit() {
        std::cout << "database commited" << std::endl;
    }
};


// Service definitions.
// We define all dependencies between classes,
// and tell the container how to map function parameter to those definitions.

// Simple injectable service by value
struct CredentialService : kgr::service<Credential> {};

// Connection service is single,
// and need the connect function to be called on creation
struct ConnectionService : kgr::single_service<Connection>,
    kgr::autocall<METHOD(&Connection::connect)> {};


// Database is also a single, and has a connection as dependency
struct DatabaseService : kgr::single_service<Database, kgr::dependency<ConnectionService>> {};

// The service map maps a function parameter type to a service definition
// We also want to map a Database argument type for the example.
auto service_map(Database const&) -> DatabaseService;
auto service_map(Credential const&) -> CredentialService;

int main() {
    kgr::container container;

    // Get the database.
    // The database has a connection injected,
    // and the connection had the connect function called before injection.
    auto&& database = container.service<DatabaseService>();

    // Commit the database
    database.commit();

    // Let `function` be a callable object that takes mapped services.
    auto function = [](Credential c, Database& db) {
        // Do stuff with credential and database
    };

    // The function is called with it's parameter injected automatically.
    container.invoke(function);

    // The programs outputs:
    //   connection established
    //   database created
    //   database commited
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
