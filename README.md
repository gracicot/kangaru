kangaru
=======

kangaru is a dependency injection container library for C++11 and C++14.
It manages recursive dependency injection, injection into function parameter, and more.
The name Kangaru came from the feature of injecting itself as a dependency into a service.

[Documentation and tutorial](https://github.com/gracicot/kangaru/wiki) is in the wiki and the `doc` folder!

```c++
#include <kangaru/kangaru.hpp>

// This macro is used as a shortcut to use kgr::Method.
#define METHOD(...) KGR_KANGARU_METHOD(__VA_ARGS__)

// The following classes are user classes.
// As you can see, this library is not intrusive and don't require modifications
struct Credential {};

struct Connection {
    // The connect needs some credential
    void connect(Credential const&);
};

struct Database {
    // A database needs a connection
    Database(Connection const&);
    
    // For the sake of having a method to call
    void commit();
};


// Service definitions.
// We define all dependencies between classes,
// and tell the container how to map function parameter to those definitions.

// Simple injectable service by value
struct CredentialService : kgr::Service<Credential> {};

// Connection service is single,
// and need the connect function to be called on creation
struct ConnectionService : kgr::SingleService<Credential>,
    kgr::Autocall<METHOD(&Connection::connect)> {};


// Database is also a single, and has a connection as dependency
struct DatabaseService : kgr::SingleService<Database, kgr::Dependency<ConnectionService>> {};

// The service map maps a function parameter type to a service definition
// We also want to map a Database argument type for the example.
auto service_map(Database const&) -> DatabaseService;
auto service_map(Credential const&) -> CredentialService;

int main() {
    kgr::Container container;
    
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
 * Clean and simple API
 * Low runtime overhead
 * Header only library
 * Clean diagnostics at compile-time.

Installation
------------
To make kangaru available on a machine, you must clonse the repository and create a build directory:

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
 * Better messages for compile-time errors (ongoing)
 * Service sources (replacable container)
