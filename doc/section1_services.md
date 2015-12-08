Services
========

Every class managed by the container is considered a service, even if it's a "single", abstract or concrete class.

#### Single services
Single services are classes that are meant to have only one instance within the container. For a single service, it's possible to provide your own instance to the container.
#### Non-Single services
Non-Single services are just services that are constructed every time they are injected or requested. In those cases, the container will behave as a fancy factory. For a non-Single service, it's possible to provide a callback that will be used every time the container needs to make a new instance.
## Declaring a service
In order to transform a class into a service, you wont need to modify your class. All you have to do, is to create a new _service definition_. The service definition will tell the container how your service should behave. This is an example of the simplest service definition possible.
Let's say you have this class and you want it to become a service:

    struct FileManager {
        // ...
    };

All you have to do is declare the following service definition:

    struct FileManagerService : kgr::Service<FileManager> {};

You made it! Now the container knows that `FileManager` is a service!

### Dependencies

In order to make a service dependent of another service, you have to add the `kgr::Dependency` parameter to the `kgr::Service` class. `kgr::Dependency` is a variadic template that has every dependency as a template argument. It is used in this way:

    struct FileManagerService : kgr::Service<FileManager, kgr::Dependency<FileSystemService>> {};

If our service `FileManagerService` needs a `Notification` and a `ClownMaster` service, you just have to add it to the parameter list, like so:

    struct FileManagerService : kgr::Service<FileManager, kgr::Dependency<
        FileSystemService,
        NotificationService,
        ClownMasterService>> {};

Now that your service `FileManager` is dependent of these other services, you have to receive them in your service's constructor:
    
    struct FileManager {
        // FileManager needs FileSystem, Notification and ClownMaster.
        
        FileManager(FileSystem _fs, Notification _n, ClownMaster _cm) : fs{_fs}, n{_n}, cm{_cm} {}
        
        FileSystem fs;
        Notification n;
        ClownMaster cm;
    };

### Single Services

Single services are really useful. You can make a single service simply by making the corresponding service definition extend `kgr::SingleService`. Here's an example:

    struct FileManagerService : kgr::SingleService<FileManager> {};

Now every instances returned by the container are the same. You can test it like that:

    auto& fm1 = container.service<FileManagerService>();
    auto& fm2 = container.service<FileManagerService>();
    
    cout << (&fm1 == &fm2 ? "true":"false") << endl; // the output will be "true"

### Available methods

There are two protected methods that are provided by generic services from the Kangaru library:
 * `getInstance()` which returns the contained instance of the service.
 * `autocall`, will see how to use it in the chapiter [Injection with setters](section5_setters.md)
 
[Next chapiter](section2_container.md)
