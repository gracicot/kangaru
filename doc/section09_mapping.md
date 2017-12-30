Advanced Mapping
================

Until now, we only used basic mapping in other examples. The service map was a simple parameter to service mapping.
But there's a mechanism to make multiple maps, prioritize them, and use many of them.

## The Map Parameter

In a mapping expression, you can actually put a second parameter. That parameter is `kgr::map_t<>`.

So here's an example of a service map with that parameter:

```c++
auto service_map(Service, kgr::map_t<>) -> Definition;
```

What is it doing? What that parameter for? Well, except for disambiguation for that declaration, not a lot.
It will simply make the container prefer this one. Consider this:

```c++
auto service_map(Service)               -> Definition1;
auto service_map(Service, kgr::map_t<>) -> Definition2;
```

When using `kgr::mapped_service_t<Service>`, that will yield `Definition2`. Because it has higher priority for the service map.

## Named Map

The additional parameter can be feeded with a name. This is how to make a map with a particular name.

A named map in not gonna be used unless to tell the container to use that map.

Every function or metafunction that deal with the service map can have a named map specified. Here are some examples of their usage:

```c++
struct MyMap;

auto service_map(Service)                    -> Definition1;
auto service_map(Service, kgr::map_t<MyMap>) -> Definition2;

// Without named parameter, yields Definition1
using UsedDefinition1 = kgr::mapped_service_t<Service>;

// With the named parameter, yields Definition2
using UsedDefinition2 = kgr::mapped_service_t<Service, kgr::map<MyMap>>;

kgr::container container;

auto function = [](Service) {};

container.invoke(function);                  // Definition1 used
container.invoke<kgr::map<MyMap>>(function); // Definition2 used
```

## Multiple Maps

The map name parameter we used with other functionalities, the `kgr::map<...>`, can in fact receive many named map to pick from.
The leftmost names will have higher priority. If a map with that name cannot be found, the container will fallback to the next name.
If it has no named map left to try, it will try the default map.

Here's an example of multiple maps:

```c++
struct MyMap1;
struct MyMap2;

auto service_map(Service1, kgr::map_t<MyMap1>) -> Service1_Definition1;
auto service_map(Service1, kgr::map_t<MyMap2>) -> Service1_Definition2;

auto service_map(Service2, kgr::map_t<MyMap2>) -> Service1_Definition1;
auto service_map(Service2)                     -> Service1_Definition2;

auto function = [](Service1, Service2) {};

container.invoke<kgr::map<MyMap1>>(function); // Service1_Definition1 and Service1_Definition2 used
container.invoke<kgr::map<MyMap2>>(function); // Service1_Definition2 and Service1_Definition2 used
container.invoke<kgr::map<MyMap1, MyMap2>>(function); // Service1_Definition1 and Service1_Definition1 used
```

[Next chapter: Debugging](section10_debug.md)
