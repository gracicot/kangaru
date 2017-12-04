Operator Services
=================

In kangaru, there are built-in services that their purpose is to make specific operations on the container.
They help encapsulate which jobs on the container a class is able to do.

They generaly wrap the container and allow only an operation to do with the container.

## Generator

A generator is an object that it's purpose is to create other object. It's kind of a factory.
The class `generator<T>` is a template class where `T` is a service definition.

The `generator` class has a service definition named `generator_service`.

It has one member function: `operator(...)`. This function is equivalent to calling the `service<T>(...)` function on the container.
It will forward every arguments to the service function.

Here's an example:
    
    // SceneService is a non-single service.
    auto sceneGenerator = container.service<kgr::GeneratorService<SceneService>>();
    
    auto scene1 = sceneGenerator();
    auto scene2 = sceneGenerator();
    auto scene3 = sceneGenerator("special parameter");
    

There's another version of the generator for a forked container. It's called `forked_generator`, and it's definition `forked_generator_service`.

## Invoker

As it's name says, this class serves the purpose of calling the `invoke` function from the container.
Why using `kgr::invoker` when you can just use `container.invoke`?
Because again, it encapsulate what you can do with the container, and express your intent of calling `invoke` on it.

Here's a code snippet using `kgr::invoker`:

    int send_message(MessageBus&, Window&, double timeout);
    
    auto invoker = container.service<kgr::invoker_service>();
    
    invoker(send_message, 10); // calls send_message with 10 as it's timeout
    
As other thing related to service map, there is a way to specify which maps to use in case of advanced mapping.

    auto invoker = container.service<kgr::mapped_invoker_service<kgr::map<MyMap1, MyMap2>>>();

Also, just like the generator, another version of the invoker for a forked container is provided, called `kgr::forked_invoker`, and it's definition `kgr::forked_invoker_service`.
    
## Lazy

The lazy class is used to represent an object that will be there later. The type `lazy<T>` takes a service definition as parameter.
That service definition will be used with the container to instanciate your object.

The lazy class has the dereference operator `*` and the arrow operator `->` to access the contained object.
Additionnaly, it has a `get()` method that returns a reference to the contained object.

The object is first instanciated when the first access operator of function is called. The instance is then reused after.

Lazy is provided with a service definition named `lazy_service<T>` where `T` is a service definition.

You can use it like this:

    // The contained 'MessageBus&' is not constructed yet.
    kgr::lazy<MessageBusService> lazy_message_bus = container.service<kgr::lazy_service<MessageBusService>>();
    
    // MessageBusService is constructed here, the operator* is used.
    cout << *lazy_message_bus;
    
    // The same instance is reused again and returned by operator->
    lazy_message_bus->process_messages();

And again, there's the equivalent of lazy with a forked container, named `forked_lazy<T>` and `forked_lazy_service<T>`

You can see extended examples of operator services in example8.

[Next chapter: Custom Service Definitions](section08_definitions.md)
