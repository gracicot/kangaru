Operator Services
=================

In kangaru, there are built-in services that their purpose is to make specific operations on the container.
They help encapsulate which jobs on the container a class is able to do.

They generaly wrap the container and allow only an operation to do with the container.

## Container

There's a service called `kgr::container_service`. It's a special service that inject a reference to the container itself.

You can call it directly, use it in invoke or as dependency:

```c++
struct Type {
    kgr::container& container;
};

struct TypeService : kgr::service<Type, kgr::dependency<kgr::container_service>> {};

kgr::container container1;
kgr::container& container2 = conatiner1.service<container_service>();

assert(&container1 == &container2); // passes, both are the same

container1.invoke([&](kgr::container& container3, Type type) {
    // Both passes
    assert(&container1 == &container3);
    assert(&container3 == &type.container);
});
```

## Fork

There's a service called `kgr::container_service`. It's a service that will call `container.fork()`, and then inject the fork.

You can call it directly, use it in invoke or as dependency:

```c++
kgr::container container1;

// equivalent to:
// kgr::container container2 = container1.fork();
kgr::container container2 = container1.service<fork_service>();

container.invoke([](kgr::container fork) {
    // fork is a forked container, we recieved by value.
});
```

The fork service has another version called `kgr::filtered_fork_service<F>`, where `F` is a predicate to filter services:

```c++
kgr::container container1;

// equivalent to:
// kgr::container container2 = container1.fork<kgr::none_of<Service1, Service2>>();
// Every service from container1 are observed by the fork except Service1 and Service2
kgr::container container2 = container1.service<filtered_fork_service<kgr::none_of<Service1, Service2>>>();
```

## Generator

A generator is an object that it's purpose is to create a specific service. It's kind of a factory that internally uses the container.
The class `generator<T>` is a template class where `T` is a service definition.

The `generator` class has a service definition named `generator_service`.

It has one member function: `operator(...)`. This function is equivalent to calling the `service<T>(...)` function on the container.
It will forward every arguments to the service function.

Here's an example:
    
```c++
// SceneService is a non-single service.
kgr::generator<SceneService> sceneGenerator = container.service<kgr::generator_service<SceneService>>();

auto scene1 = sceneGenerator();
auto scene2 = sceneGenerator();
auto scene3 = sceneGenerator("special parameter");
```

There's another version of the generator for a forked container. It's called `forked_generator`, and it's definition `forked_generator_service`.

## Invoker

As it's name says, this class serves the purpose of calling the `invoke` function from the container.
Why using `kgr::invoker` when you can just use `container.invoke`?
Because again, it encapsulate what you can do with the container, and express your intent of only calling `invoke` on it.

Here's a code snippet using `kgr::invoker`:

```c++
int send_message(MessageBus&, Window&, double timeout);

kgr::invoker invoker = container.service<kgr::invoker_service>();

invoker(send_message, 10); // calls send_message with 10 as it's timeout
```
    
As other thing related to service map, there is a way to specify which maps to use in case of advanced mapping.

```c++
kgr::mapped_invoker<kgr::map<MyMap1, MyMap2>> invoker =
    container.service<kgr::mapped_invoker_service<kgr::map<MyMap1, MyMap2>>>();
```

Also, just like the generator, another version of the invoker for a forked container is provided, called `kgr::forked_invoker`, and it's definition `kgr::forked_invoker_service`.
    
## Lazy

The lazy class is used to represent a service that will be constructed later. The type `lazy<T>` takes a service definition as parameter.
That service definition will be used with the container to instantiate your object.

The lazy class has the dereference operator `*` and the arrow operator `->` to access the contained object.
Additionally, it has a `get()` method that returns a reference to the contained object.

The object is first instantiated when the first access operator of function is called. The instance is then reused after.

Lazy is provided with a service definition named `lazy_service<T>` where `T` is a service definition.

You can use it like this:

```c++
// The contained 'MessageBus&' is not constructed yet.
kgr::lazy<MessageBusService> lazy_message_bus = container.service<kgr::lazy_service<MessageBusService>>();

// MessageBusService is constructed here, the operator* is used.
std::cout << *lazy_message_bus;

// The same instance is reused again and returned by operator->
lazy_message_bus->process_messages();
```

And again, there's the equivalent of lazy with a forked container, named `forked_lazy<T>` and `forked_lazy_service<T>`

## Conclusion

While you can use the container directly everywhere, you can also be more fined grained over what a particular piece of code should be able to do with the container. It eventually reduces coupling with kangaru and help expressing your intent with other programmer about what you will do with the container.

Please visit [example7](../examples/example7/example7.cpp) to see more of operator service usage.

[Next chapter: Custom Service Definitions](section08_definitions.md)
