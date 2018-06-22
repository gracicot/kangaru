Generic Services
================

This section shows you how to create your own generic service class just like `kgr::service` or `kgr::single_service`.
It's the most advanced part of this documentation, and optional for everyday use of this library.
You won't have to do this unless you have very specific needs.

First, our generic service usually have two template parameter: The service class and a dependency class.

```c++
template<typename Type, typename Deps = kgr::dependency<>>
struct MyUniqueService;
```

## Extending kgr::generic_service

Generic service is a class that will be default constructible, even if the contained service type is not.
This will make user definition able to omit inheriting constructors.

To use generic service, we only need to pass one parameter, the contained service type. It can be the service, a pointer to the service or any other way you want to contain the service type.

```c++
template<typename Type, typename Deps = kgr::dependency<>>
struct MyUniqueService;

template<typename Type, typename... Deps>
struct MyUniqueService<Type, kgr::dependency<Deps...>> : kgr::generic_service<std::unique_ptr<Type>> {
    
};
```

Then, we must implement three member functions: `construct`, `forward` and `call`.

There's a function we haven't seen before. What is this mysterious `call` function? It's a function to call a function on the contained instance. It is used by `kgr::autocall`.

The first parameter is a pointer to the method to be called.
Then it recieves a pack of parameters as argument to the function to be called.

Here's the typical implementation:

```c++
template<typename T, typename... Args>
static auto call(Type& instance, T method, Args&&... args) -> decltype(auto) {
    return (instance().*method)(std::forward<Args>(args)...);
}
```
    
_`decltype(auto)` is a c++14 feature even if only c++11 is required. It is used for the sake of simplicity and is not required._

Now let's see how they look like in `MyUniqueService`!

## Full example

```c++
template<typename, typename>
struct MyUniqueService;

template<typename Type, typename... Deps>
struct MyUniqueService<Type, kgr::dependency<Deps...>> : kgr::generic_service<std::unique_ptr<Type>> {
    template<typename... Args>
    static auto construct(kgr::inject_t<Deps>... deps, Args&&... args)
        -> kgr::inject_result<std::unique_ptr<Type>>
    {
        return kgr::inject(std::make_unique<Type>(deps.forward()..., std::forward<Args>(args)...));
    }

    std::unique_ptr<Type> forward() {
        return std::move(this->instance());
    }
    
    template<typename T, typename... Args>
    static auto call(T method, Args&&... args) -> decltype(auto) {
        return ((*this->instance()).*method)(std::forward<Args>(args)...);
    }
};
```

### Using The Generic Definition

You can now use your class as the following:

```c++
struct MyClassService : MyUniqueService<MyClass, kgr::dependency<MyDependencyService>> {};
```

## Singles

If the generic definition should be single, simply make it extend from the `kgr::single` tag class.

## Autowire (experimental)

WARNING: This section is totally optional to follow, use an experimental API and is rather complicated to use at the moment. However, if you do need enabling autowiring for your own generic services, this section will describe how to do it.

While it is possible to enable autowire for custom generic services, it is still considered experimental,
since the API may still be subject to changes, and is rather complicated at that time.

It starts with `kgr::experimental::autowire_tag<Map, max_dependencies>`. This replaces dependencies when
specializing the generic service type:

```c++
template<typename, typename>
struct MyUniqueService;

template<typename Type, typename Map, std::size_t max_dependencies>
struct MyUniqueService<Type, kgr::experimental::autowire_tag<Map, max_dependencies>> : kgr::generic_service<std::unique_ptr<Type>> {
    // ...
};
```

Now we change the the construct function in order to construct the type correctly.

First off there is a metafunction that returns if the deduction is successful, the amount of deduced parameters, and the
basic construct function return type. It is called `kgr::experimental::autowiring::amount_of_deductible_service`.

It's parameters are:

 * The generic service type, to avoid recursion
 * The service type to construct
 * The service map to use
 * The maximum amount of deductible services
 * And finally, the additional arguments sent to the construct function.

We recomend defining this template alias at class scope:

```c++
template<typename... Args>
using amount_deduced = kgr::experimental::autowiring::amount_of_deductible_service<MyUniqueService, Type, Map, max_dependencies, Args...>;
```

Our new construct function will need one dependency: the container. It will also only be enabled if the service
constructor's parameters can be deduced.

Here's the autowiring construct function for a basic service:

```c++
template<typename... Args>
static auto construct(kgr::inject_t<kgr::container_service> cont, Args&&... args)
    -> std::enable_if_t<amount_deduced<Args...>::deductible, typename amount_deduced<Args...>::default_result_t>
{
    return kgr::experimental::autowiring::deduce_construct_default<service, Map>(
        amount_deduced<Args...>::amount, std::move(cont), std::forward<Args>(args)...
    );
}
```

For unique services, we must define how to call `kgr::inject`. To do that, a generic function object can be sent.

```c++
template<typename... Args>
static auto construct(kgr::inject_t<kgr::container_service> cont, Args&&... args)
    -> detail::enable_if_t<amount_deduced<Args...>::deductible, kgr::inject_result<std::unique_ptr<Type>>>
{
	auto inject = [](auto&&... args) {
		return kgr::inject(std::unique_ptr<Type>{new Type(std::forward<decltype(args)>(args)...)});
	};
	
    return kgr::experimental::autowiring::deduce_construct<MyUniqueService, Map>(
        amount_deduced<Args...>::amount, inject, std::move(cont), std::forward<Args>(args)...
    );
}
```

Putting it all together, the autowiring specialization will look like this:

```c++
template<typename Type, typename Map, std::size_t max_dependencies>
struct MyUniqueService<Type, detail::autowire_tag<Map, max_dependencies>> : generic_service<std::unique_ptr<Type>> {
private:
	template<typename... Args>
	using amount_deduced = kgr::experimental::autowiring::amount_of_deductible_service<
		MyUniqueService, Type, Map, max_dependencies, Args...
	>;
	
public:
	template<typename... Args>
	static auto construct(kgr::inject_t<kgr::container_service> cont, Args&&... args)
		-> std::enable_if_t<amount_deduced<Args...>::deductible, kgr::inject_result<std::unique_ptr<Type>>>
	{
		auto inject = [](auto&&... args) {
			return kgr::inject(std::unique_ptr<Type>{new Type(std::forward<decltype(args)>(args)...)});
		};
	
		return kgr::experimental::autowiring::deduce_construct<MyUniqueService, Map>(
			amount_deduced<Args...>::amount, inject, std::move(cont), std::forward<Args>(args)...
		);
	}
	
	auto forward() -> std::unique_ptr<Type> {
		return std::move(this->instance());
	}
	
    template<typename T, typename... Args>
    static auto call(T method, Args&&... args) -> decltype(auto) {
        return std::invoke(method, *this->instance(), std::forward<Args>(args)...);
    }
};
```

[Next chapter: Structuring projects](section13_structure.md)
