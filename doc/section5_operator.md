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
Why using `Invoker` when you can just use `container.invoke`? Because of it's more conveniant of course!

Instead of sending the map every time the `invoke` function is called, the map is a template parameter of the class `Invoker`

Here's a code snippet using the `Invoker`:

    int sendRequest(ClownMaster&, Notification, double timeout);
    
    auto invoker = container.service<InvokerService<ServiceMap>>();
    
    invoker(sendRequest, 10); // calls sendRequest with 10 as it's timeout
    
Just like `Generator`, another version of the invoker for a forked container is provided. It's called `ForkedInvoker`, and it's definition `ForkedInvokerService`.
    
## Lazy

The lazy class is used to represent an object that will be there later. The type `Lazy<T>` takes a service definition as parameter.
That service definition will be used with the container to instanciate your object.

The lazy class has the dereference operator `*` and the arrow operator `->` to access the contained object.
Additionnaly, it has a `get()` method that returns a reference to the contained object.

The object is first instanciated when the first access operator of function is called. The instance is then reused after.

Lazy is provided with a service definition named `LazyService<T>` where `T` is a service definition.

You can use it like this:

    // The contained 'ClownMaster' is not constructed yet.
    auto lazyClownMaster = container.service<LazyService<ClownMasterService>>();
    
    // ClownMaster is constructed here, the operator* is used.
    cout << *lazyClownMaster;
    
    // The same instance is reused again and returned by operator->
    lazyClownMaster->print();

And again, there's the equivalent of lazy with a forked container, named `ForkedLazy<T>` and `ForkedLazyService<T>`

You can see extended examples of operator services in example8.

[Next chapter](section6_setters.md)
