Services
========

Every class managed by the container is considered a service, even if it's a single, non-single, abstract or concrete class.

#### Single services
Single services are classes that are meant to have only one instance within the container. For a single service, it's possible to provide your own instance to the container.

#### Non-Single services
Non-Single services are just services that are constructed every time they are injected or requested.
In those cases, the container will behave as a fancy factory.

## Declaring a service
In order to transform a class into a service, you wont need to modify your class. All you have to do, is to declare a new _service definition_.
The service definition will tell the container how your service should behave. It should be a non-polymorphic class, except for abstract services.
This is an example of the simplest service definition possible.
Let's say you have this class and you want it to become a service:

```c++
struct FileManager {
    // ...
};
```

All you have to do is declare the following service definition:

```c++
struct FileManagerService : kgr::Service<FileManager> {};
```

You made it! Now the container knows that `FileManager` is a service!

### Dependencies

In order to make a service dependent of another service, you have to add the `kgr::Dependency` parameter to the `kgr::Service` class.
`kgr::Dependency` is a variadic template that has every dependency as a template argument. It is used like this:

```c++
struct FileManagerService : kgr::Service<FileManager, kgr::Dependency<FileSystemService>> {};
```

If our service `FileManagerService` needs a `Notification` and a `ClownMaster` service, you just have to add it to the parameter list, like so:

```c++
struct FileManagerService : kgr::Service<FileManager, kgr::Dependency<
    FileSystemService,
    NotificationService,
    ClownMasterService
>> {};
```

Now that your service `FileManager` is dependent of these other services, you have to receive them in your service's constructor:

```c++
struct FileManager {
    // FileManager needs FileSystem, Notification and ClownMaster.
    
    FileManager(FileSystem _fs, Notification _n, ClownMaster _cm) : fs{_fs}, n{_n}, cm{_cm} {}
    
    FileSystem fs;
    Notification n;
    ClownMaster cm;
};
```

Take note that the order of parameter in the constructor must match the order in the dependencies declaration.

### Single Services

Single services are really useful. You can make a single service simply by making the corresponding service definition extend `kgr::SingleService`. Here's an example:

```c++
struct FileManagerService : kgr::SingleService<FileManager> {};
```

Now every instances returned by the container are the same. You can test it like that:

```c++
auto& fm1 = container.service<FileManagerService>();
auto& fm2 = container.service<FileManagerService>();

cout << (&fm1 == &fm2 ? "true":"false") << endl; // the output will be "true"
```

### Available methods

There is one protected method that is provided by generic services from the Kangaru library: `instance()` which returns the contained instance of the service.

### Other Service Types

There are three other service types:
 * `kgr::SharedService`: a single service injected as a `std::shared_ptr`.
 * `kgr::UniqueService`: a service injected as a `std::unique_ptr`.

`kgr::Service` and `kgr::SingleService` require that your classes are destructible.
 
[Next chapter](section2_container.md)
