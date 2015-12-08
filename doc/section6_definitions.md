Writing service definitions from scratch
========================================

Previously, each time we did a service definition, we extended the `kgr::Service` or the `kgr::SingleService` structs. But it is important to note that we are not limited to that.

Writing a service definition is pretty easy. You only need to define two functions for it to work:

 * `construct`
 * `forward`

The `construct` function is static and returns the service definition itself. It's a factory method. It can take any other service definition as a parameter. This is where dependencies are resolved.
The `forward` function takes no parameters and returns the service. The return type of this function defines how your service should be injected. Note that this function has the power to invalidate the service definition.

Normally, your service definition should contain your service.

A tipical service definition looks like this:

    struct FileManagerService {
        static FileManagerService construct() {
            return {};
        }
        
        FileManager forward() {
            return std::move(fm);
        }
        
    private:
        FileManager fm;
    };

If the class `FileManager` has dependencies, you can add them in the `construct` function as parameters:

    struct FileManagerService {
        static FileManagerService construct(NotificationService& ns, ClownMasterService cms) {
            return { FileManager{ns.forward(), cms.forward()} };
        }
        
        // return as move, invalidating the service definition is okay.
        FileManager forward() {
            return std::move(fm);
        }
        
    private:
        FileManager fm;
    };
    
Note that single services must be received as references, like our `NotificationService&` in this particular case. By definition, a Single must not be copied.
The container will call `construct` with the right set of parameters, automatically.

#### Additional parameters

Sometimes, a type requires a parameter like a double or a string. The `construct` function can take as many additional parameters as you want. The only downside is that it does not support optional parameters, maybe in the next release ;)

For this service definition:

    struct FileManagerService {
        static FileManagerService construct(NotificationService& ns, ClownMasterService cms, std::string s, int n) {
            return { FileManager{ns.forward(), cms.forward(), s, n} };
        }
        
        // return as move, invalidating the service definition is okay.
        FileManager forward() {
            return std::move(fm);
        }
        
    private:
        FileManager fm;
    };

You have to call it that way:

    auto fm = container.service<FileManagerService>("potatos", 34);

### Singles

There are two steps required in order to make `FileManagerService` single. First, we need to make our struct inherit from `kgr::Single`. Secondly, we also need to adapt the `forward` function by making it virtual, in order to not invalidate the contained service.

Note: single services can forward copies too, but you rarely want to do that. Returning a reference or pointer is a much better idea when it comes to single service.

(Why is that so? Could be interesting to explain here. -Fred)

So let's make our sevice a Single:

    struct FileManagerService : Single {
        static FileManagerService construct(NotificationService& ns, ClownMasterService cms) {
            return { FileManager{ns.forward(), cms.forward()} };
        }
        
        // return as pointer, must not invalidate the service. Must be virtual.
        virtual FileManager* forward() {
            return &fm;
        }
        
    private:
        FileManager fm;
    };

Why should it be virtual? Remember the `Override` feature? To achieve this, the container relies on polymorphic behaviour. All you have to do is make your `forward` method virtual, and the container will be happy.

### Abstract Services

Abstract services are the simplest ones to implement. They have only one pure virtual method called `forward`:

    struct IFileManagerService : Single {
        virtual IFileManager& forward() = 0;
    }
    
Abstract services must be single.

(Why is that so? Could be interesting to explain here too! -Fred)
 
[Next chapiter](section7_generic.md)
