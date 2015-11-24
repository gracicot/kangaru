Services
========

Every class managed by the container is considered a service, whether it's a "single" class or not, abstract or concrete.

#### Single services
Single service are classes that meant to have only once instance within the container. For a single, it's possible to provide the container your own instance.
#### Non-Single services
Non-Single services are just a service that is constructed every time it is injected or requested. The container will behave as a fancy factory. For a non-Single service, it's possible to provide a callback that will be used every time the container need to make a new instance.
## Declaring a service
In order to make a class to become a service, you wont need to modify your class. All you have to do, it's to make a _service definition_. The service definition tell the container how your service behave. This is an example of the simplest service definition possible.
Let's say you have this class that must become a service:

    struct FileManager {
        // ...
    };

All you have to do is to declare this service definition:

    struct FileManagerService : kgr::Service<FileManager> {};

You just made it! Now the container knows that `FileManager` is a service!

### Dependencies

In order to make a service dependent of another service, you have to add the `kgr::Dependency` parameter to the `kgr::Service` class. `kgr::Dependency` is a variadic template that has every dependency as a template argument. It is used this way:

    struct FileManagerService : kgr::Service<FileManager, kgr::Dependency<FileSystemService>> {};

If our service `FileManagerService` needs a `Notification` and a `ClownMaster`, you just have to add it to the list:

    struct FileManagerService : kgr::Service<FileManager, kgr::Dependency<
        FileSystemService,
        NotificationService,
        ClownMasterService>> {};

Now that you service `FileManager` is dependent of these classes, you have to receive them in your constructor:
    
    struct FileManager {
        // FileManager needs FileSystem, Notification and ClownMaster.
        
        FileManager(FileSystem _fs, Notification _n, ClownMaster _cm) : fs{_fs}, n{_n}, cm{_cm} {}
        
        FileSystem fs;
        Notification n;
        ClownMaster cm;
    };

### Single Services
Single services are really useful. You can make a single service simply be making your service sevice definition extending `kgr::SingleService`. Here's an example:

    struct FileManagerService : kgr::SingleService<FileManager> {};

Now every instances returned by the container is the same. You can test it like that:

    auto& fm1 = container->service<FileManagerService>();
    auto& fm2 = container->service<FileManagerService>();
    
    cout << (&fm1 == &fm2 ? "true":"false") << endl; // the output will be "true"
