#ifndef KGR_KANGARU_INCLUDE_KANGARU_AUTOWIRE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_AUTOWIRE_HPP

#include "container.hpp"
#include "generic.hpp"
#include "service.hpp"

#include "detail/service_map.hpp"
#include "detail/utils.hpp"
#include "detail/traits.hpp"
#include "detail/container_service.hpp"
#include "detail/injected.hpp"
#include "detail/autowire_traits.hpp"

namespace kgr {
namespace detail {

/**
 * Alias to autowire_map. Act as a autowire tag. This one is required to be used for autowired basic_service.
 */
template<template<typename, typename> class Service, typename Map, std::size_t max_dependencies>
using basic_autowire_tag = autowire_map<Service, decay_t, Map, max_dependencies>;

/**
 * This is the base for autowired default non-single and single service.
 * 
 * Contains all the shared logic between single and non single autowired services.
 */
template<typename Type, template<typename, typename> class Service, typename Map, std::size_t max_dependencies>
struct basic_service<Type, basic_autowire_tag<Service, Map, max_dependencies>> : basic_service_base<Type> {
private:
	using Derived = Service<Type, autowire_tag<Map, max_dependencies>>;
	
	template<typename... Args>
	using amount_deduced = amount_of_deductible_service<Derived, Type, Map, max_dependencies, Args...>;
	
public:
	using basic_service_base<Type>::basic_service_base;
	
	template<typename... Args>
	static auto construct(inject_t<container_service> cont, Args&&... args)
		-> enable_if_t<amount_deduced<Args...>::deductible, typename amount_deduced<Args...>::default_result_t>
	{
		return deduce_construct_default<Derived, Map>(
			amount_deduced<Args...>::amount, std::move(cont), std::forward<Args>(args)...
		);
	}
};

} // namespace detail

/**
 * This is the autowired default non-single service.
 * 
 * It hold and return the service by value.
 */
template<typename Type, typename Map, std::size_t max_dependencies>
struct service<Type, detail::autowire_tag<Map, max_dependencies>> : detail::basic_service<Type, detail::basic_autowire_tag<kgr::service, Map, max_dependencies>> {
	using detail::basic_service<Type, detail::basic_autowire_tag<kgr::service, Map, max_dependencies>>::basic_service;
	
	auto forward() -> Type {
		return std::move(this->instance());
	}
};

/**
 * This class is the autowired default single service.
 * 
 * It hold the service as value, and returns it by reference.
 */
template<typename Type, typename Map, std::size_t max_dependencies>
struct single_service<Type, detail::autowire_tag<Map, max_dependencies>> : detail::basic_service<Type, detail::basic_autowire_tag<kgr::single_service, Map, max_dependencies>>, single {
	using detail::basic_service<Type, detail::basic_autowire_tag<kgr::single_service, Map, max_dependencies>>::basic_service;
	
	auto forward() -> Type& {
		return this->instance();
	}
};


/**
 * This class is the service definition for a autowired, non-single heap allocated service.
 * 
 * It works for both case where you need a shared pointer non-single service,
 * because they are implicitly constructible from a unique pointer.
 * 
 * It will hold the service as a std::unique_ptr, and inject it as a std::unique_ptr
 */
template<typename Type, typename Map, std::size_t max_dependencies>
struct unique_service<Type, detail::autowire_tag<Map, max_dependencies>> : unique_service<Type> {
private:
	template<typename... Args>
	using amount_deduced = detail::amount_of_deductible_service<unique_service, Type, Map, max_dependencies, Args...>;
	
	// This would be replaced by a generic lambda in C++14
	struct inject_function {
		template<typename... Args>
		auto operator()(Args&&... args) -> inject_result<std::unique_ptr<Type>> {
			return inject(std::unique_ptr<Type>{new Type(std::forward<Args>(args)...)});
		}
	};
	
public:
	using unique_service<Type>::unique_service;
	
	template<typename... Args>
	static auto construct(inject_t<container_service> cont, Args&&... args)
		-> detail::enable_if_t<amount_deduced<Args...>::deductible, inject_result<std::unique_ptr<Type>>>
	{
		return detail::deduce_construct<unique_service, Map>(
			amount_deduced<Args...>::amount, inject_function{}, std::move(cont), std::forward<Args>(args)...
		);
	}
};

/**
 * This class is a autowired service definition when a single should be injected as a shared pointer.
 * 
 * It will hold the service as a std::shared_ptr and inject it a std::shared_ptr
 */
template<typename Type, typename Map, std::size_t max_dependencies>
struct shared_service<Type, detail::autowire_tag<Map, max_dependencies>> : shared_service<Type> {
private:
	template<typename... Args>
	using amount_deduced = detail::amount_of_deductible_service<shared_service, Type, Map, max_dependencies, Args...>;
	
	// This would be replaced by a generic lambda in C++14
	struct inject_function {
		template<typename... Args>
		auto operator()(Args&&... args) -> inject_result<std::shared_ptr<Type>> {
			return inject(std::make_shared<Type>(std::forward<Args>(args)...));
		}
	};
	
public:
	using shared_service<Type>::shared_service;
	
	template<typename... Args>
	static auto construct(inject_t<container_service> cont, Args&&... args)
		-> detail::enable_if_t<amount_deduced<Args...>::deductible, inject_result<std::shared_ptr<Type>>>
	{
		return detail::deduce_construct<shared_service, Map>(
			amount_deduced<Args...>::amount, inject_function{}, std::move(cont), std::forward<Args>(args)...
		);
	}
};

/*
 * The following aliases are indirect maps for autowired services.
 * 
 * kgr::autowire is also a autowire tag to use instead of dependencies
 */
using autowire = detail::autowire_map<service, detail::decay_t, map<>, detail::default_max_dependency>;
using autowire_single = detail::autowire_map<single_service, detail::decay_t, map<>, detail::default_max_dependency>;
using autowire_unique = detail::autowire_map<unique_service, detail::unwrap_pointer_t, map<>, detail::default_max_dependency>;
using autowire_shared = detail::autowire_map<shared_service, detail::unwrap_pointer_t, map<>, detail::default_max_dependency>;

/**
 * autowire tag that can receive which service map should be used.
 */
template<typename Map, std::size_t max_dependencies = detail::default_max_dependency>
using mapped_autowire = detail::autowire_map<service, detail::decay_t, Map, max_dependencies>;

/*
 * The following four aliases are shortcuts for autowired services.
 * 
 * It act as a shorter alternative to service<Type, kgr::mapped_autowire<...>>
 */
template<typename T, typename Map = map<>, std::size_t max_dependencies = detail::default_max_dependency>
using autowire_service = single_service<T, kgr::mapped_autowire<Map, max_dependencies>>;

template<typename T, typename Map = map<>, std::size_t max_dependencies = detail::default_max_dependency>
using autowire_single_service = single_service<T, kgr::mapped_autowire<Map, max_dependencies>>;

template<typename T, typename Map = map<>, std::size_t max_dependencies = detail::default_max_dependency>
using autowire_unique_service = unique_service<T, kgr::mapped_autowire<Map, max_dependencies>>;

template<typename T, typename Map = map<>, std::size_t max_dependencies = detail::default_max_dependency>
using autowire_shared_service = shared_service<T, kgr::mapped_autowire<Map, max_dependencies>>;

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_AUTOWIRE_HPP
