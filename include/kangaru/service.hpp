#ifndef KGR_KANGARU_INCLUDE_KANGARU_SERVICE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_SERVICE_HPP

#include <type_traits>

#include "detail/utils.hpp"
#include "detail/single.hpp"
#include "generic.hpp"

namespace kgr {

/**
 * This class is simply a list of definition the current service depends on to be constructed.
 */
template<typename... Args>
using Dependency = detail::meta_list<Args...>;

/**
 * This class is the default single service.
 * 
 * It hold the service as value, and returns it by reference.
 */
template<typename, typename = Dependency<>>
struct SingleService;

template<typename Type, typename... Deps>
struct SingleService<Type, Dependency<Deps...>> : GenericService<Type>, Single {
private:
	using Parent = GenericService<Type>;
	
protected:
	using Parent::instance;
	
public:
	using Parent::Parent;
	
	template<typename... Args>
	static auto construct(Inject<Deps>... deps, Args&&... args) -> inject_result<ServiceType<Deps>..., Args...> {
		return inject(deps.forward()..., std::forward<Args>(args)...);
	}

	Type& forward() {
		return instance();
	}
	
	template<typename T, typename... Args>
	detail::function_result_t<T> call(T method, Args&&... args) {
		return (instance().*method)(std::forward<Args>(args)...);
	}
};

/**
 * This is the default non-single service.
 * 
 * It hold and return the service by value.
 */
template<typename, typename = Dependency<>>
struct Service;

template<typename Type, typename... Deps>
struct Service<Type, Dependency<Deps...>> : GenericService<Type> {
private:
	using Parent = GenericService<Type>;
	
protected:
	using Parent::instance;
	
public:
	using Parent::Parent;
	
	template<typename... Args>
	static auto construct(Inject<Deps>... deps, Args&&... args) -> inject_result<ServiceType<Deps>..., Args...> {
		return inject(deps.forward()..., std::forward<Args>(args)...);
	}

	Type forward() {
		return std::move(instance());
	}
	
	template<typename T, typename... Args>
	detail::function_result_t<T> call(T method, Args&&... args) {
		return (instance().*method)(std::forward<Args>(args)...);
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
template<typename, typename = Dependency<>>
struct UniqueService;

template<typename Type, typename... Deps>
struct UniqueService<Type, Dependency<Deps...>> : GenericService<std::unique_ptr<Type>> {
private:
	using Parent = GenericService<std::unique_ptr<Type>>;
	
protected:
	using Parent::instance;
	
public:
	using Parent::Parent;
	
	template<typename... Args>
	static auto construct(Inject<Deps>... deps, Args&&... args)
	-> decltype(inject(std::unique_ptr<Type>{new Type{std::declval<Deps>().forward()..., std::declval<Args>()...}})) {
		return inject(std::unique_ptr<Type>{new Type{deps.forward()..., std::forward<Args>(args)...}});
	}
	
	std::unique_ptr<Type> forward() {
		return std::move(instance());
	}
	
	template<typename T, typename... Args>
	detail::function_result_t<T> call(T method, Args&&... args) {
		return (instance()->*method)(std::forward<Args>(args)...);
	}
};

/**
 * This class is a service definition when a single should be injected as a shared pointer.
 * 
 * It will hold the service as a std::shared_ptr and inject it a s a std::shared_ptr
 */
template<typename, typename = Dependency<>>
struct SharedService;

template<typename Type, typename... Deps>
struct SharedService<Type, Dependency<Deps...>> : GenericService<std::shared_ptr<Type>>, Single {
private:
	using Parent = GenericService<std::shared_ptr<Type>>;
	
protected:
	using Parent::instance;
	
public:
	using Parent::Parent;

	static auto construct(Inject<Deps>... deps) -> decltype(inject(std::make_shared<Type>(std::declval<Deps>().forward()...))) {
		return inject(std::make_shared<Type>(deps.forward()...));
	}
	
	std::shared_ptr<Type> forward() {
		return instance();
	}
	
	template<typename T, typename... Args>
	detail::function_result_t<T> call(T method, Args&&... args) {
		return (instance()->*method)(std::forward<Args>(args)...);
	}
};

/*
 * This class is a abstract service that a kgr::SingleService can override.
 * 
 * It cannot be constructed, but only overrided.
 */
template<typename T>
struct AbstractService : Abstract {
	T& forward();
};

/*
 * This class is an abstract service that can be overrided by kgr::SharedService
 * 
 * As it is abstract, a service that overrides it must be instanciated by the container before usage.
 */
template<typename T>
struct AbstractSharedService : Abstract {
	std::shared_ptr<T> forward();
};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_SERVICE_HPP
