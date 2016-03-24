Operation Services
==================

In kangaru, there are built-in services that their purpose is to make specific operations on the container.
They help encapsulate which jobs on the container a class is able to do.

Let's dive into it!

## Generator

A generator is an object that it's purpose is to create other object. It's kind of a factory.
The class `Generator<T>` is a template class where `T` is a service definition.

The `Generator` class has a service definition named `GeneratorService`.

It has one member function: `operator()`. This function is equivalent to calling the `service<T>()` function on the container.

Here's an example:

    
    // NotificationService is a non-single service.
    auto notificationGenerator = container.service<GeneratorService<NotificationService>>();
    
    auto notification1 = notificationGenerator();
    auto notification2 = notificationGenerator();
    auto notification3 = notificationGenerator();
    
    
There's another version of the generator for a forked container. It's called `ForkedGenerator`, and it's definition `ForkedGeneratorService`.

## Invoker

As it's name says, this class serves the purpose of calling the `invoke` function from the container.

[Next chapter](section6_setters.md)
