The container
=============

The container is the central piece of the library, it's where the magic is done.

There are three methods:

 * `instance<T>`
 * `service<T>`
 * `invoke<T>`

## instance
This function either register a provided instance of a service definition or explicitly instanciate a single service.

    // Will instanciate a Shop within the container
    container.instance<ShopService>();
    
    // Gives a ClownMasterService to the container
    container.instance(ClownMasterService{42});

Here we ask the container to construct and register the service ShopService. Then, we provide the container an instance of `ClownMasterService` as the instance the container should use for later.

## service
Service is the most crucial function of the container. It returns a service and construct it if needed. the template parameter is the service definition of the one you want.

        Shop myShop = container.service<ShopService>(); // I just got a fully constructed Shop!

## invoke
The `invoke` method is here to help you call functions that need services as parameter. Let's say we have the service `Notification`, `FileManager` and `Shop` and this function:

    int someOperation(Notification&, FileManager& b, Shop c);

You can make the call to this function easier with the invoke function:

    int result = container.invoke<ServiceMap>(someOperation);

As long as the service `Notification`, `FileManager` and `Shop` are configured correctly, the container will call the function `someOperation` with the right set of parameter.
We will cover `invoke` in detail later.
 
[Next chapiter](section3_override.md)
