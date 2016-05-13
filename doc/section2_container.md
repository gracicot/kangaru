The container
=============

The container is the central piece of the library, it's where the magic is done.

It contains the following six methods:

 * `instance<T>`
 * `service<T>`
 * `invoke<T>`
 * `contains<T>`
 * `clear`
 * `fork`
 * `merge`

## instance
This function will instanciate the service definition in the template parameter with the sended arguments.

    // Will instanciate a ShopService within the container with a shop as arguments.
    container.instance<ShopService>(Shop{items...});

Here, we ask the container to construct the service named `ShopService` with `Shop{items...}` as arguments.

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

## contains

This function is used to check if the container currently contains a specific service.
This function take a single service type as template parameter and returns a boolean.

    kgr::Container c;

    cout << boolalpha << c.contains<NotificationService>() << ", ";

    c.service<NotificationService>();

    cout << c.contains<NotificationService>() << endl;

    // outputs the following: false, true

## clear
This function clears the container state. In other words, it deletes every single services that are contained within the container.

        // FileManagerService is a single service.
        
        auto& fileManager = container.service<FileManagerService>();
        
        container.clear();
        
        // now fileManager is a dangling reference.
        
## fork
Single service are not singletons. They are single within an instance of a container.
To ease this mechanism, there's a method called `fork()` that create a copy of the container.

The forked container is not a complete copy of the original container. It will hold non-owning reference of services that are contained in the original.
That way, the forked container will act just as the original, but every new instances of single service will be known only by the forked container.

Here's a code snippet that shows it's usage:

    kgr::Container c1;
    
    // File manager is single
    auto& fileManager1 = c1.service<FileManagerService>();
    
    // here fork is creating a new container
    kgr::Container c2 = c1.fork();
    
    auto& fileManager2 = c2.service<FileManagerService>();
    
    cout << boolalpha << (&fileManager1 == &fileManager2) << endl; // prints true
    
    // shop is also single
    // shop1 and shop2 are different instance, even if they are single.
    auto& shop1 = c1.service<ShopService>();
    auto& shop2 = c2.service<ShopService>();
    
    cout << boolalpha << (&shop1 == &shop2) << endl; // prints false
    
### Inject the fork
You can as well inject the container into a service as a fork. There's the class `ForkService` that do just that.
Here's a usage of this service definition:

    kgr::Container fork = container.service<DefaultForkService>(); // another way to fork

### Fork with some instances

Alternatively, you can fork the container but keeping reference only to some service that was in the original container.
The `fork()` function takes a template parameter that is the predicate to pick.

#### All

By default, `kgr::All` is used as the predicate. It returns true for all service.

#### AnyOf

The predicate `kgr::AnyOf<Ts...>` will make that only the specified services get to be known to the new container. It a sort of a whitelist for services.

    auto fork = container.fork<kgr::AnyOf<ShopService, FileManagerService>>();

    cout << boolalpha << fork.contains<NotificationService>() << endl; // prints false, even if `container` had one.

#### NoneOf

The predicate `kgr::NoneOf<Ts...>` is the contrary of `kgr::AnyOf<Ts...>`. It will take every services except those specified. It's like a blacklist.

    auto fork = container.fork<kgr::NoneOf<ShopService>>();

    // fork has every services contained in `container` except `ShopService`

#### lambda

If neither of those provided by kangaru suits your needs, you can send a lambda as predicate.
The lambda must receive a `kgr::type_id_t` as parameter and return a boolean. You can get the `type_id_t` for a type using `kgr::type_id<T>`.
The implementation of those two construct are considered implementation defined.

    auto fork = container.fork([](kgr::type_id_t id){
        return id == &kgr::type_id<NotificationService>;
    });

## merge
You can as well merge containers. One container is merge into another. In case of collisions, the merger keep it's own instances.

    // c1 and c2 are both containers
    
    // c1 now holds all instance contained in c2.
    c1.merge(std::move(c2));
    
After the container is merged, no services are invalidated. Any change to this behaviour are considered breaking.
 
[Next chapter](section3_override.md)
