Override Services
=================

What would be a dependency injection container without the ability to dispatch services polymorphically with interfaces and stuff?
Overriding services in kangaru could not be simpler! Just make your service definitions extend from the `kgr::Overrides<Parents...>` class!

    struct FileManagerService : kgr::SingleService<FileManager>, kgr::Overrides<IFileManagerService> {};
    
That's it! Now we must tell the container that `FileManagerService` exists.

    container.instance<FileManagerService>();
    
 If we want to use a `FileManager`, we can request it from the container like that:
 
    IFileManager& fm = container.service<IFileManagerService>();
    
As long as the overrider's service type is convertible to the parent's service type, the service override will work.
     
### Order matters

Let's take a look at this particuliar case:

    struct FileManagerService : kgr::SingleService<FileManager>, kgr::Overrides<IFileManagerService> {};
    struct ClownMasterService : kgr::SingleService<ClownMaster>, kgr::Overrides<IFileManagerService> {};

Ouch! Which one is chosen by the container?
It depends.

In fact, the rule that defines which one will be taken is **the last one registered is the one which overrides**

Yes. Look at these two cases:

    {
        Container c;
        
        c.instance<ClownMasterService>();
        c.instance<FileManagerService>();
        
        // auto is a FileManager
        auto& fm = c.service<IFileManagerService>();
    }

    {
        Container c;
        
        c.instance<FileManagerService>();
        c.instance<ClownMasterService>();
        
        // auto is a ClownMaster
        auto& fm = c.service<IFileManagerService>();
    }
    
### Abstract Service

If you want to make a service definition for an abstract type, you may extend from `kgr::AbstractService<T>`:

    struct IFileManagerService : kgr::AbstractService<IFileManager>;
 
[Next chapter](section4_invoke.md)
