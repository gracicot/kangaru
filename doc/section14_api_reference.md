API Reference
=============

> NOTE: This section is still incomplete and a work in progress.

## Container

The kangaru container class.

This class will construct services and share single instances for a given definition.
It is the class that parses and manage dependency graphs and calls autocall functions.

### emplace

This function construct and save in place a service definition with the provided arguments.

The service is only constructed if it is not found.

It is usually used to instanciate supplied services.

It returns if the service has been constructed.

This function require the service to be single.

```c++
template<typename T, typename... Args> requires SingleService<T> && ConstructibleService<T, Args...>
auto emplace(Args&&... args) -> bool;
```

### replace

This function construct and save in place a service definition with the provided arguments.

The inserted instance of the service will be used for now on.

It does not delete the old instance if any.

This function require the service to be single.

```c++
template<typename T, typename... Args> requires SingleService<T> && ConstructibleService<T, Args...>
void replace(Args&&... args);
```

### service

This function returns the service given by service definition `T`.

`T` must be a valid service and must be constructible with arguments passed as parameters

In case of a non-single service, it takes additional arguments to be sent to the `T::construct` function.

`T` must not be a polymorphic type.

```c++
template<typename T, typename... Args> requires Service<T, Args...>
void service(Args&&... args);
```

### invoke

This function returns the result of the callable object of type `U`.

`Args` are additional arguments to be sent to the function after services arguments.

This function will deduce arguments from the function signature.

```c++
template<typename M = kgr::map<>, typename U, typename... Args> requires Invocable<M, U, Args...> && Map<M>
void service(U&& function, Args&&... args);

template<typename... Services, typename U, typename... Args> requires Callable<U, service_type<Services>..., Args...>
void service(U&& function, Args&&... args);
```

### clear

This function clears this container.

Every single services are invalidated after calling this function.

```c++
void clear();
```

### fork

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

### merge

This function merges a container with another.

The receiving container will prefer it's own instances in a case of conflicts.

```c++
void merge(container&& other);
```

### rebase

This function will add all services form the container sent as parameter into this one.
Note that the lifetime of the container sent as parameter must be at least as long as this one.
If the container you rebase from won't live long enough, consider using the merge function.

It takes a predicate type as template argument.
The default predicate is `kgr::all`.

```c++
template<typename Predicate = kgr::all>
auto fork(const container& other, Predicate predicate = {}) const -> kgr::container;
```

### contains

This function return `true` if the container contains the service `T`. Returns `false` otherwise.

`T` nust be a single service.

```c++
template<typename T> requires BasicService<T>
auto contains() const -> bool;
```
