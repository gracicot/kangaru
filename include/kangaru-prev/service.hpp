#ifndef KGR_KANGARU_INCLUDE_KANGARU_SERVICE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_SERVICE_HPP

#include "generic.hpp"

#include "detail/utils.hpp"
#include "detail/single.hpp"
#include "detail/nostd_invoke.hpp"

#include <type_traits>

namespace kgr {

/**
 * This class is simply a list of definition the current service depends on to be constructed.
 */
template<typename... Args>
using dependency = detail::meta_list<Args...>;

namespace detail {

/**
 * This is the base for default non-single and single service.
 * 
 * Contains all the shared logic between single and non single services.
 */
template<typename Type>
struct basic_service_base : generic_service<Type> {
	using generic_service<Type>::generic_service;
	
	template<typename T, typename... Args>
	auto call(T method, Args&&... args) -> detail::nostd::invoke_result_t<T, Type&, Args...> {
		return detail::nostd::invoke(method, this->instance(), std::forward<Args>(args)...);
	}
};

template<typename, typename>
struct basic_service;

template<typename Type, typename... Deps>
struct basic_service<Type, dependency<Deps...>> : basic_service_base<Type> {
	using basic_service_base<Type>::basic_service_base;
	
	template<typename... Args>
	static auto construct(inject_t<Deps>... deps, Args&&... args) -> inject_result<service_type<Deps>..., Args...> {
		return inject(deps.forward()..., std::forward<Args>(args)...);
	}
};

} // namespace detail

/**
 * This class is the default single service.
 * 
 * It hold the service as value, and returns it by reference.
 */
template<typename Type, typename Deps = dependency<>>
struct single_service : detail::basic_service<Type, Deps>, single {
	using detail::basic_service<Type, Deps>::basic_service;

	auto forward() -> Type& {
		return this->instance();
	}
};

/**
 * This class is a service definition for a single service managed by an external system.
 * 
 * It hold the service as a reference to the instance, and returns it by reference.
 */
template<typename Type>
struct extern_service : single_service<Type&>, supplied {};

/**
 * This is the default non-single service.
 * 
 * It hold and return the service by value.
 */
template<typename Type, typename Deps = dependency<>>
struct service : detail::basic_service<Type, Deps> {
	using detail::basic_service<Type, Deps>::basic_service;
	
	auto forward() -> Type {
		return std::move(this->instance());
	}
};

/**
 * This class is the service definition for a non-single heap allocated service.
 * 
 * It works for both case where you need a shared pointer non-single service,
 * because they are implicitly constructible from a unique pointer.
 * 
 * It will hold the service as a std::unique_ptr, and inject it as a std::unique_ptr
 */
template<typename, typename = dependency<>>
struct unique_service;

template<typename Type, typename... Deps>
struct unique_service<Type, dependency<Deps...>> : generic_service<std::unique_ptr<Type>> {
	using generic_service<std::unique_ptr<Type>>::generic_service;
	
	template<typename... Args>
	static auto construct(inject_t<Deps>... deps, Args&&... args)
		-> detail::enable_if_t<std::is_constructible<Type, service_type<Deps>..., Args...>::value, inject_result<std::unique_ptr<Type>>>
	{
		return inject(std::unique_ptr<Type>{new Type(deps.forward()..., std::forward<Args>(args)...)});
	}
	
	auto forward() -> std::unique_ptr<Type> {
		return std::move(this->instance());
	}
	
	template<typename T, typename... Args>
	auto call(T method, Args&&... args) -> detail::nostd::invoke_result_t<T, Type&, Args...> {
		return detail::nostd::invoke(method, *this->instance(), std::forward<Args>(args)...);
	}
};

/**
 * This class is a service definition when a single should be injected as a shared pointer.
 * 
 * It will hold the service as a std::shared_ptr and inject it a s a std::shared_ptr
 */
template<typename, typename = dependency<>>
struct shared_service;

template<typename Type, typename... Deps>
struct shared_service<Type, dependency<Deps...>> : generic_service<std::shared_ptr<Type>>, single {
	using generic_service<std::shared_ptr<Type>>::generic_service;
	
	template<typename... Args>
	static auto construct(inject_t<Deps>... deps, Args&&... args)
		-> detail::enable_if_t<std::is_constructible<Type, service_type<Deps>..., Args...>::value, inject_result<std::shared_ptr<Type>>>
	{
		return inject(std::make_shared<Type>(deps.forward()..., std::forward<Args>(args)...));
	}
	
	auto forward() -> std::shared_ptr<Type> {
		return this->instance();
	}
	
	template<typename T, typename... Args>
	auto call(T method, Args&&... args) -> detail::nostd::invoke_result_t<T, Type&, Args...> {
		return detail::nostd::invoke(method, *this->instance(), std::forward<Args>(args)...);
	}
};

/**
 * This class is a service definition for a single service managed by an external system.
 * 
 * It hold and injects the service as a shared pointer to the supplied instance.
 */
template<typename Type>
struct extern_shared_service : shared_service<Type>, supplied {
	using shared_service<Type>::shared_service;
	
	static auto construct(std::shared_ptr<Type> instance) -> inject_result<std::shared_ptr<Type>> {
		return inject(std::move(instance));
	}
};

/**
 * This class is a abstract service that a kgr::single_service can override.
 * 
 * It cannot be constructed, but only overrided.
 */
template<typename T>
struct abstract_service : abstract {
	auto forward() -> T&;
};

/**
 * This class is an abstract service that can be overrided by kgr::shared_service
 * 
 * As it is abstract, a service that overrides it must be instanciated by the container before usage.
 */
template<typename T>
struct abstract_shared_service : abstract {
	auto forward() -> std::shared_ptr<T>;
};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_SERVICE_HPP
