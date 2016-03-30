The container
=============

The container is the central piece of the library, it's where the magic is done.

It contains the following three methods:

 * `instance<T>`
 * `service<T>`
 * `invoke<T>`
 * `clear`

## instance
This function either registers a provided instance of a service definition or explicitly instanciates a single service.

    // Will instanciate a Shop within the container
    container.instance<ShopService>();
    
    // Gives a ClownMasterService intance to the container
    container.instance(ClownMasterService{42});

Here, we ask the container to construct and register the service named "ShopService". Then, we provide the container an instance of `ClownMasterService` for future use.

## service
Service is the most crucial function of the container. It returns a service and constructs it if needed.
The function's template parameter corresponds to the service definition of the service you want to retrieve.

    Shop myShop = container.service<ShopService>(); // I just got a fully constructed Shop!
        
If your constructor has to take more parameters, you can send it to the `service()` function and the container will forward the argument after dependencies.

    struct Shop {
        Shop(Notification& n, int numberOfItems);
        
        // ...
    };
    
    Shop myShop = container.service<ShopService>(42); // I just got a fully constructed Shop with 42 items!

## invoke
The `invoke` method is here to help you call functions that need services as parameters.
Let's say we have the following services: `Notification`, `FileManager` and `Shop`.
Also, let's say we have this function:

    int someOperation(Notification&, FileManager& b, Shop c);

You can make the call to this function easier with the invoke function, like so:

    int result = container.invoke<ServiceMap>(someOperation);

As long as the `Notification`, `FileManager` and `Shop` service definitions are configured correctly, the container will call the function `someOperation` with the right set of parameters.
We will cover the `invoke` functionnality in detail in a later chapter.

## clear
This function clears the container state. In other words, it deletes every single services that are contained within the container.

        // FileManagerService is a single service.
        
        auto& fileManager = container.service<FileManagerService>();
        
        container.clear();
        
        // now fileManager is a dangling reference.

 
[Next chapter](section3_override.md)
