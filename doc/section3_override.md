Override Services
=================

What would be a dependency injection container without the ability to dispatch services polymorphically with interfaces and stuff?
Overriding services in kangaru could not be simpler! Just make your service definitions extend from the `kgr::Overrides<Parents...>` class!

```c++
struct FileManagerService : kgr::SingleService<FileManager>, kgr::Overrides<IFileManagerService> {};
```
    
That's it! Now we must tell the container that `FileManagerService` exists.

```c++
container.instance<FileManagerService>();
```
    
 If we want to use a `FileManager`, we can request it from the container like that:
 
```c++
IFileManager& fm = container.service<IFileManagerService>();
```
    
As long as the overrider's service type is convertible to the parent's service type, the service override will work.
     
### Order matters

Let's take a look at this particuliar case:

```c++
struct FileManagerService : kgr::SingleService<FileManager>, kgr::Overrides<IFileManagerService> {};
struct ClownMasterService : kgr::SingleService<ClownMaster>, kgr::Overrides<IFileManagerService> {};
```

Ouch! Which one is chosen by the container?
It depends.

In fact, the rule that defines which one will be taken is **the last one registered is the one which overrides**

Yes. Look at these two cases:

```c++
{
    kgr::Container c;
    
    c.service<ClownMasterService>();
    c.service<FileManagerService>();
    
    // auto is a FileManager
    auto& fm = c.service<IFileManagerService>();
}

{
    kgr::Container c;
    
    c.service<FileManagerService>();
    c.service<ClownMasterService>();
    
    // auto is a ClownMaster
    auto& fm = c.service<IFileManagerService>();
}
```

### Abstract Service

If you want to make a service definition for an abstract type, you may extend from `kgr::AbstractService<T>`:

```c++
struct IFileManagerService : kgr::AbstractService<IFileManager>;
```

Note that for shared service types there's `kgr::AbstractSharedService`.

#### Default Service Type

If you ask the container for an abstract service and the container has no instance that could be used for that abstract service, the container dill throw.

If that's not the correct behaviour, you can also specify a default service to instantiate instead of throwing.

```c++
struct IFileManagerService : kgr::AbstractService<IFileManager>, kgr::Default<TreeFileManagerService> {};

kgr::Container c;

// Will instantiate a TreeFileManager and return a IFileManager.
auto& fileManager = c.service<IFileManagerService>();
```

### Final Services

Any Service can be marked as final to avoid being overriden. To make a service final, simply inherit from the `kgr::Final` tag:

```c++
struct FileManagerService : kgr::SingleService<FileManager>, kgr::Overrides<IFileManagerService>, kgr::Final {};
```

Tempting to override that service will result in a compile time error.

[Next chapter: Invoke](section4_invoke.md)
