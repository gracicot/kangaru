Injection With Setters (autocall)
=================================

Few dependency injection libraries offers automatic injection using setters. Fortunatly, this is one of those!

Sadly, since [N4469](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4469.html) has still not been accepted yet, we recommend you to declare macros similar to these:

    #define METHOD(...) ::kgr::Method<decltype(__VA_ARGS__), __VA_ARGS__>

Of course, you are free to name them as you want.
Once your macros are declared, it's time for the interesting things!

## The `AutoCall` type

Any service can extends the class `kgr::AutoCall`. This class enable the needed metadata for the container to call the methods you want to be called.

The `kgr::AutoCall` struct has the service map as it's first parameter, and has a parameter pack that is the list of methods you want the container to call when the service is constructed.

Let's see an example of it's usage. So we have this class:

    struct ClownMaster {
        void init() {
            n = 42;
        }
        
    private:
        int n = 0;
    };
    
If we want `init()` to be called at the service's construction, we need our definition to extends autocall:

    struct ClownMasterService : kgr::Service<ClownMaster>, kgr::AutoCall<ServiceMap, METHOD(&ClownMaster::init)> {};
    
But wait, there's more!

What if the needed value of `n` comes from another service?

Here comes the fancy thing. Method called by `kgr::AutoCall` can receive dependencies. So here's our class according to the new need:

    struct ClownMaster {
        void init(Shop& s) {
            n = s.countItems();
        }
        
    private:
        int n = 0;
    };

That's it! You can add any number of parameter as you wish, the definition will stay the same and the method will receive what you ask for.
We can have multiple method call with multiple parameter:

    struct ClownMaster {
        void init(Shop& s, FileManager& theFm) {
             n = s.countItems() + fm.countFiles();
        }
        
        void setFileManager(FileManager& theFm) {
            fm = &theFm;
        }
        
    private:
        FileManager* fm;
        int n = 0;
    };
    
Now we want to add `setFileManager` to the list of method to call:

    struct ClownMasterService : kgr::Service<ClownMaster>, kgr::AutoCall<ServiceMap,
        METHOD(&ClownMaster::init),
        METHOD(&ClownMaster::setFileManager)> {};
        
The method are called in the order that are listed in `AutoCall`.

### Without the service map

Alternatively, you can list needed sevices for every methods. Parameters are grouped within the `kgr::Invoke` class:

    struct ClownMasterService : kgr::Service<ClownMaster>, kgr::AutoCall<ServiceMap,
        kgr::Invoke<METHOD(&ClownMaster::init), ShopService, FileManagerService>,
        METHOD(&ClownMaster::setFileManager)> {};

If you don't want to use the service map at all, you can exdends `kgr::AutoCallNoMap` instead:
    
    struct ClownMasterService : kgr::Service<ClownMaster>, kgr::AutoCallNoMap<
        kgr::Invoke<METHOD(&ClownMaster::init), ShopService, FileManagerService>,
        kgr::Invoke<METHOD(&ClownMaster::setFileManager), FileManagerService>> {};


## How does this black magic work?

The `kgr::AutoCall` class is quite magic. Fortunately, this magic behaviour can be reproduced without the need of this class.

The type `kgr::Invoke` has anoter use. It can serve as a list of method to call.
It is used internally by the `kgr::AutoCall` type.

Let's review the example seen above without the `AutoCall` type and let's demystify this!

So let's say you have this service and its matching service definition:

    struct ClownMaster {
        void init() {
            n = 42;
        }
        
    private:
        FileManager* fm;
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

That's it! If you understand this section, you are now officially a wizard!
 
[Next chapter](section6_definitions.md)
