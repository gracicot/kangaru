API Reference
=============

> NOTE: This section is still incomplete and a work in progress.

## `kgr::container`

The kangaru container class.

This class will construct services and share single instances for a given definition.
It is the class that parses and manage dependency graphs and calls autocall functions.

### `emplace`

This function construct and save in place a service definition with the provided arguments.

The service is only constructed if it is not found.

It is usually used to instanciate supplied services.

It returns if the service has been constructed.

This function require the service to be single.

```c++
template<typename T, typename... Args> requires SingleService<T> && ConstructibleService<T, Args...>
auto emplace(Args&&... args) -> bool;
```

### `replace`

This function construct and save in place a service definition with the provided arguments.

The inserted instance of the service will be used for now on.

It does not delete the old instance if any.

This function require the service to be single.

```c++
template<typename T, typename... Args> requires SingleService<T> && ConstructibleService<T, Args...>
void replace(Args&&... args);
```

### `service`

This function returns the service given by service definition `T`.

`T` must be a valid service and must be constructible with arguments passed as parameters

In case of a non-single service, it takes additional arguments to be sent to the `T::construct` function.

`T` must not be a polymorphic type.

```c++
template<typename T, typename... Args> requires Service<T, Args...>
void service(Args&&... args);
```

### `invoke`

This function returns the result of the callable object of type `U`.

`Args` are additional arguments to be sent to the function after services arguments.

This function will deduce arguments from the function signature.

```c++
template<typename M, typename U, typename... Args> requires Invocable<M, U, Args...> && Map<M>
void invoke(M map, U&& function, Args&&... args);

template<typename M = kgr::map<>, typename U, typename... Args> requires Invocable<M, U, Args...> && Map<M>
void invoke(U&& function, Args&&... args);

template<typename... Services, typename U, typename... Args> requires Callable<U, service_type<Services>..., Args...>
void invoke(U&& function, Args&&... args);
```

### `clear`

This function clears this container.

Every single services are invalidated after calling this function.

```c++
void clear();
```

### `fork`

This function fork the container into a new container.

The new container will have the copied state of the first container.

Construction of new services within the new container will not affect the original one.

The new container must exist within the lifetime of the original container.

It takes a predicate type as template argument.

The default predicate is `kgr::all`.

```c++
template<typename Predicate = kgr::all>
auto fork(Predicate predicate = {}) const -> kgr::container;
```

### `merge`

This function merges a container with another.

The receiving container will prefer it's own instances in a case of conflicts.

```c++
void merge(container&& other);
```

### `rebase`

This function will add all services form the container sent as parameter into this one.
Note that the lifetime of the container sent as parameter must be at least as long as this one.
If the container you rebase from won't live long enough, consider using the merge function.

It takes a predicate type as template argument.
The default predicate is `kgr::all`.

```c++
template<typename Predicate = kgr::all>
auto fork(const container& other, Predicate predicate = {}) const -> kgr::container;
```

### `contains`

This function return `true` if the container contains the service `T`. Returns `false` otherwise.

`T` nust be a single service.

```c++
template<typename T> requires BasicService<T>
auto contains() const -> bool;
```

## `kgr::invoker`

A type that can call a function with injected parameters.

It can be created by the container with `kgr::invoker_service`.

## `operator()`

Takes a function to invoke with injectable parameters.

Example:
```c++
kgr::container container;
kgr::invoker invoker = container.service<kgr::invoker_service>();

invoker([](Single1& single1, Service2 service2, int i) {
    // stuff with single1 serivce2 and i
    // The variable `i` is equal to 1 since we send 1 as the additional parameter.
    // Single1 and Service2 are expected to be correctly mapped services.
}, 1);
```

An additional overload accept a map type as first parameter:
```c++
struct my_own_map;
auto service_map(Service3, kgr::map_t<my_own_map>) -> kgr::autowire;

kgr::container container;
kgr::invoker invoker = container.service<kgr::invoker_service>();

invoker(kgr::map<my_own_map>{}, [](Single1& single1, Service3 service3) {
    // service3 is only mapped in map `my_own_map` and
    // would not be invokable without the map parameter.
});
```

## `kgr::lazy<S>`

An uninitialized service that will be created on first use.

The template parameter `S` must be a complete service type.

## `get`

Returns a reference to the wrapped service. Construct the service if uninitialized.

## `operator*`

Same as `get`.

## `operator->`

Returns a pointer to the underlying service.

## `kgr::generator<S>`

A type that contruct the non-single service `S` with given parameters.

## `operator()`

Takes any parameter to be forwarded te the service `S` after injected parameters.

Example:
```c++
struct Service1 {
	Service1(/* dependencies... */, int additional_parameter = 0) {}
};

struct Service1Service : kgr::service<Service1, kgr::dependency</* ... */>> {};

kgr::generator<Service1Service> make_service1 = container.service<kgr::generator_service<Service1Service>>();

Service1 s1 = make_service1();
Service1 s2 = make_service1(2);
```
