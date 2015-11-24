The container
=============

The container is the central piece of the library, it's where the magic is done.

It contains the following three methods:

 * `instance<T>`
 * `service<T>`
 * `invoke<T>`

## instance
This function either registers a provided instance of a service definition or explicitly instanciates a single service.

    // Will instanciate a Shop within the container
    container->instance<ShopService>();
    
    // Gives a ClownMasterService intance to the container
    container->instance(ClownMasterService{42});

Here, we ask the container to construct and register the service named "ShopService". Then, we provide the container an instance of `ClownMasterService` for future use.

## service
Service is the most crucial function of the container. It returns a service and constructs it if needed. The function's template parameter corresponds to the service definition of the service you want to retrieve.

        Shop myA = container->service<ShopService>(); // I just got a fully constructed Shop!

## invoke
The `invoke` method is here to help you call functions that need services as parameters. Let's say we have the following services: `Notification`, `FileManager` and `Shop`. Also, let's say we have this function:

    int someOperation(Notification&, FileManager& b, Shop c);

You can make the call to this function easier with the invoke function, like so:

    int result = container->invoke<ServiceMap>(someOperation);

As long as the `Notification`, `FileManager` and `Shop` services are configured correctly, the container will call the function `someOperation` with the right set of parameters.
The next chapiter will cover `invoke` in detail.