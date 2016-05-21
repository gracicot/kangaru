Injection With Setters (autocall)
=================================

Few dependency injection libraries offers automatic injection using setters. Fortunatly, this is one of those!

Sadly, since [N4469](http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2015/n4469.html) has still not been accepted yet, we recommend you to declare macros similar to these:

```c++
#define METHOD(...) ::kgr::Method<decltype(__VA_ARGS__), __VA_ARGS__>
```

Of course, you are free to name them as you want.
Once your macros are declared, it's time for the interesting things!

## The `AutoCall` type

Any service can extends the class `kgr::AutoCall`. This class enable the needed metadata for the container to call the methods you want to be called.

The `kgr::AutoCall` struct has the service map as it's first parameter, and has a parameter pack that is the list of methods you want the container to call when the service is constructed.

Let's see an example of it's usage. So we have this class:

```c++
struct ClownMaster {
    void init() {
        n = 42;
    }
    
private:
    int n = 0;
};
```
    
If we want `init()` to be called at the service's construction, we need our definition to extends autocall:

```c++
    struct ClownMasterService : kgr::Service<ClownMaster>, kgr::AutoCall<ServiceMap, METHOD(&ClownMaster::init)> {};
    
But wait, there's more!

What if the needed value of `n` comes from another service?

Here comes the fancy thing. Method called by `kgr::AutoCall` can receive dependencies. So here's our class according to the new need:

```c++
struct ClownMaster {
    void init(Shop& s) {
        n = s.countItems();
    }
    
private:
    int n = 0;
};
```

That's it! You can add any number of parameter as you wish, the definition will stay the same and the method will receive what you ask for.
We can have multiple method call with multiple parameter:

```c++
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
```
    
Now we want to add `setFileManager` to the list of method to call:

```c++
struct ClownMasterService : kgr::Service<ClownMaster>, kgr::AutoCall<ServiceMap,
    METHOD(&ClownMaster::init),
    METHOD(&ClownMaster::setFileManager)
> {};
```
        
The method are called in the order that are listed in `AutoCall`.

### Without the service map

Alternatively, you can list needed sevices for every methods. Parameters are grouped within the `kgr::Invoke` class:

```c++
struct ClownMasterService : kgr::Service<ClownMaster>, kgr::AutoCall<ServiceMap,
    kgr::Invoke<METHOD(&ClownMaster::init), ShopService, FileManagerService>,
    METHOD(&ClownMaster::setFileManager)
> {};
```

If you don't want to use the service map at all, you can exdends `kgr::AutoCallNoMap` instead:

```c++
struct ClownMasterService : kgr::Service<ClownMaster>, kgr::AutoCallNoMap<
    kgr::Invoke<METHOD(&ClownMaster::init), ShopService, FileManagerService>,
    kgr::Invoke<METHOD(&ClownMaster::setFileManager), FileManagerService>
> {};
```

[Next chapter](section7_definitions.md)
