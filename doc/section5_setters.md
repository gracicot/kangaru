Injection With Setters
======================

Few dependency injection libraries offers automatic injuction using setter. Fortunatly, this is one of those!

Sadly, since [N4469](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4469.html) has still not been accepted yet, we recomend you to declare similar macros:

    #define METHOD(...) decltype(__VA_ARGS__), __VA_ARGS__
    #define INVOKE(...) ::kgr::Method<decltype(__VA_ARGS__), __VA_ARGS__>

Now we can start.

### The invoke type

kangaru declares a type named `kgr::Invoke<Methods...>`, which is a compile time list of method to call when the service is constructed.

So let's say you have this service and definition:

    struct ClownMaster {
        void init() {
            n = 42;
        }
        
    private:
        int n = 0;
    };

    struct ClownMasterService : kgr::Service<ClownMaster> {};
    
We would like to call `init()` when our `ClownMaster` is constructed. But calling it manually each time it's constructed would be error prone and not pretty.
The solution is to tell the container to call some specific methods automatically.

To do this, you must using invoke in your service definition.

    struct ClownMasterService : kgr::Service<ClownMaster> {
        
        void callMe() {
            cout << "called" << endl;
        }
        
        using invoke = kgr::Invoke< INVOKE(&ClownMasterService::callMe) >;
    };
    
When the service is constructed, `called` will be printed in the terminal.

    auto cm1 = container.service<ClownMasterService>(); // prints "called"
    auto cm2 = container.service<ClownMasterService>(); // prints "called" again
    
#### What power does this gives to us?

We can now call our `init()` method!

    struct ClownMasterService : kgr::Service<ClownMaster> {
        void callInit() {
            getInstance().init();
        }
        
        using invoke = kgr::Invoke< INVOKE(&ClownMasterService::callInit) >;
    };
    
### More automatisation

If theres many method to call, it will clutter our service definition struct. That's not good. Instead, we defined the member function `autocall`, which in our case will replace `callInit()`.
The function `autocall` takes at least two parameters, first the type of a function and second the pointer to the function. With the macro it will look like that:

    struct ClownMasterService : kgr::Service<ClownMaster> {
        using invoke = kgr::Invoke<
            INVOKE( &Self::autocall<METHOD(&ClownMaster::init)> )
        >;
    };
    
In this example, `Self` is an alias to the parent, to the `kgr::Service<ClownMaster>` in this case.

### Injection of parameters

Now that we know how to call function automatically, we will now learn how to inject parameters in our `ClownMaster` setters.
Let's revisit the class `ClownMaster`:

    struct ClownMaster {
        void setFileManager(FileManager& theFm) {
            fm = &theFm;
        }
        
    private:
        FileManager* fm;
    };
    
Would it be awesome if we could call magically `setFileManager()` at the sevice construction? We can. Invoke "scans" the parameter of the called function and will call that function with the right set of parameters. Let's rollback our example when we did not used `autocall` yet and add a parameter:

    struct ClownMasterService : kgr::Service<ClownMaster> {
    
        // as "FileManagerService" is a service definition, the container will send it as parameter automatically
        
        void callSetter(FileManagerService& fms) {
            getInstance().setFileManager(fms.forward());
        }
        
        using invoke = kgr::Invoke< INVOKE(&ClownMasterService::callSetter) >;
    };
    
### More automatisation again

The `autocall` function as said takes _at least_ two parameters. The other parameters are what argument `autocall` should forward to the service's setters or methods. Pay attention to the additional parameter of `autocall`

    struct ClownMasterService : kgr::Service<ClownMaster> {
        using invoke = kgr::Invoke<
            INVOKE(&Self::autocall<METHOD(&ClownMaster::setFileManager), FileManagerService>)
        >;
    };
    
Just by that, every `ClownMaster` constructed by the container will have a `FileManager` injected using it's setter. Clever!

Note that autocall cas as well receive any number of parameter:
    
    struct ClownMaster {
        void setThings(FileManager& theFm, Shop s) {
            // ...
        }
        
        // ...
    };
    
    struct ClownMasterService : kgr::Service<ClownMaster> {
        using invoke = kgr::Invoke<
            INVOKE(&Self::autocall<METHOD(&ClownMaster::setThings), FileManagerService, ShopService>)
        >;
    };
    
### Using service map

Instead of listing every service the called function needs, we can alternatively sending the service map:

    struct ClownMasterService : kgr::Service<ClownMaster> {
        using invoke = kgr::Invoke<
            INVOKE(&Self::autocall<METHOD(&ClownMaster::setThings), ServiceMap>)
        >;
    };
    
The advantage of this method is that if `setThings` parameters change, there is no refactor needed.

Thats it! If you understand this, you are now officially a wizard!
