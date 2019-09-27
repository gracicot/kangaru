Debugging
=========

kangaru is a heavily templated library. It's commonly known that library that rely a lot on templates are hard to diagnostic,
and as soon as something goes wrong with user defined types, you'll get a huge monologue of errors from the compiler.

With kangaru we decided to emulate concepts to provide better error messages the users of our library.

Most of kangaru function are guarded by a constraints. It will recursively detect errors in constructors, dependencies, definitions, and others.
For GCC 7 and older, we used a small trick with templated constructors and default values that allows us to call a static assertation when a deleted function is invoked.

## Explicit debugging

If you don't have GCC or you explicitly want to know what's wrong with a particular service, you can call `kgr::debug::service<T>(Args...)`.
Simply call that function just like it would be to call `container.service<T>()` then build your project. 

Let's trigger the same error as above with this code:

    kgr::debug::service<PathPrinterService>(2);

This is the compiler output (Clang):

    In file included from example1.cpp:5:
    In file included from ../../include/kangaru/kangaru.hpp:4:
    In file included from ../../include/kangaru/detail/../container.hpp:11:
    ../../include/kangaru/detail/error.hpp:50:3: error: static_assert failed "The service type is not constructible given passed arguments to kgr::container::service(...)."
                    static_assert(false_t<Arg>::value, "The service type is not constructible given passed arguments to kgr::Container::service(...).");
                    ^             ~~~~~~~~~~~~~~~~~~~
    ../../include/kangaru/debug.hpp:11:35: note: in instantiation of function template specialization 'kgr::detail::service_error<PathPrinterService>::service_error<int, PathPrinterService, 0>' requested here
            detail::service_error<T, Args...> error{std::forward<U>(u)};
                                             ^
    example1.cpp:50:14: note: in instantiation of function template specialization 'kgr::debug::service<PathPrinterService, int>' requested here
            kgr::debug::service<PathPrinterService>(2);
                        ^
    1 error generated.

As you can see, the compiler is effectively stating that the service is not constructible given passed arguments.

If `kgr::debug::service<T>(Args...)` Detects no error, it will output `"No known error detected."` as a static assert.
If you have found a case whene it does not compile and yet no error is detected, just let me know and report a bug in our issue tracker.

[Next chapter: Generic Services](section12_generic.md)
