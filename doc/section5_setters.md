Injection With Setters
======================

Few dependency injection libraries offers automatic injuction using setters. Fortunatly, this is one of those!

Sadly, since [N4469](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4469.html) has still not been accepted yet, we recommend you to declare macros similar to these:

    #define METHOD(...) decltype(__VA_ARGS__), __VA_ARGS__
    #define INVOKE(...) ::kgr::Method<decltype(__VA_ARGS__), __VA_ARGS__>

Of course, you are free to name them as you want.
Once your macros are declared, it's time for the interesting thing!

### The invoke type

kangaru declares a type named `kgr::Invoke<Methods...>`, which is a compile time list of methods to call when the service is constructed.

So let's say you have this service and its matching service definition:

    struct ClownMaster {
        void init() {
            n = 42;
        }
        
    private:
        int n = 0;
    };

    struct ClownMasterService : kgr::Service<ClownMaster> {};
    
We would like to call `init()` when our `ClownMaster` is constructed. But calling it manually each time it's constructed would be error prone and not pretty.
The solution is to tell the container that it has to call some specific methods automatically.

We'll start with something simple, like printing something to the console when our service is constructed.

To do this, you must be using invoke in your service definition.

    struct ClownMasterService : kgr::Service<ClownMaster> {
        
        void callMe() {
            cout << "called" << endl;
        }
        
        using invoke = kgr::Invoke< INVOKE(&ClownMasterService::callMe) >;
    };
    
When the service is constructed, `called` will be printed in the terminal.

    auto cm1 = container.service<ClownMasterService>(); // prints "called"
    auto cm2 = container.service<ClownMasterService>(); // prints "called" again
    
As we can see, the `invoke` member type tells the container to call a member function of the service definition.
    
#### What kind of power does this give us?

We can use the `callMe()` method to call the `init()` method of the service!

    struct ClownMasterService : kgr::Service<ClownMaster> {
        void callMe() {
            getInstance().init();
        }
        
        using invoke = kgr::Invoke< INVOKE(&ClownMasterService::callInit) >;
    };
    
Remember the `getInstance()` function? It returns the service contained in the definition.
    
### More automatisation

If there's a lot of method to call, it will clutter our service definition struct. That's not good. Instead, we defined the member function `autocall`, which in our case will replace the `callInit()` method.
The function `autocall` takes at least two parameters, first, the type of a function and, second, the pointer to the function. With the macro defined higher, it will look like that:

    struct ClownMasterService : kgr::Service<ClownMaster> {
        using invoke = kgr::Invoke<
            INVOKE( &ClownMasterService::autocall<METHOD(&ClownMaster::init)> )
        >;
    };

### Injection of parameters

Now that we know how to call functions automatically, we will now learn how to inject parameters in our `ClownMaster` setters.
Let's revisit the class `ClownMaster`:

    struct ClownMaster {
        void setFileManager(FileManager& theFm) {
            fm = &theFm;
        }
        
    private:
        FileManager* fm;
    };
    
Wouldn't it be awesome if we could call magically `setFileManager()` at the time of the sevice's construction? We can. Invoke "scans" the parameters of the called function and will call that function with the right set of parameters, automatically. Let's rollback our example to when we did not used `autocall` yet and add a parameter:

    struct ClownMasterService : kgr::Service<ClownMaster> {
    
        // as "FileManagerService" is a service definition, the container will send it as parameter automatically
        
        void callSetter(FileManagerService& fms) {
            getInstance().setFileManager(fms.forward());
        }
        
        using invoke = kgr::Invoke< INVOKE(&ClownMasterService::callSetter) >;
    };
    
### More automatisation again

The `autocall` function as said earlier takes _at least_ two parameters. The other parameters are the arguments that `autocall` should forward to the service's setters or methods. Pay attention to the additional parameter of the `autocall`.

    struct ClownMasterService : kgr::Service<ClownMaster> {
        using invoke = kgr::Invoke<
            INVOKE(&ClownMasterService::autocall<METHOD(&ClownMaster::setFileManager), FileManagerService>)
        >;
    };
    
Just like that, every `ClownMaster` constructed by the container will have a `FileManager` injected using it's setter. Clever!

Note that `autocall` can as well receive any number of parameter:
    
    struct ClownMaster {
        void setThings(FileManager& theFm, Shop s) {
            // ...
        }
        
        // ...
    };
    
    struct ClownMasterService : kgr::Service<ClownMaster> {
        using invoke = kgr::Invoke<
            INVOKE(&ClownMasterService::autocall<METHOD(&ClownMaster::setThings), FileManagerService, ShopService>)
        >;
    };
    
### Using service map

Instead of listing every service the called function needs, we can alternatively send the service map:

    struct ClownMasterService : kgr::Service<ClownMaster> {
        using invoke = kgr::Invoke<
            INVOKE(&ClownMasterService::autocall<METHOD(&ClownMaster::setThings), ServiceMap>)
        >;
    };
    
The advantage of this method is that, if `setThings` parameters change, there is no refactor needed.

That's it! If you understand this, you are now officially a wizard!
 
[Next chapter](section6_definitions.md)
