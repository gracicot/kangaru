#pragma once

#include <type_traits>

#include "detail/utils.hpp"
#include "container.hpp"
#include "detail/container_service.hpp"

namespace kgr {

template<typename... Args>
struct Dependency {};

namespace detail {

template<typename T> using InjectType_t = typename std::conditional<std::is_base_of<Single, T>::value, T&, T&&>::type;

template<typename...>
struct Injector;

template<typename CRTP, typename... Deps>
struct Injector<CRTP, Dependency<Deps...>> {
	static CRTP construct(InjectType_t<Deps>... deps) {
		using C = typename CRTP::C;
		return C::makeService(deps.forward()...);
	}
};

template<typename CRTP, typename ContainedType>
struct BaseGenericService {
	BaseGenericService() = default;
	
	BaseGenericService(BaseGenericService&& other) {
		setInstance(std::move(other.getInstance()));
	}
	
	BaseGenericService& operator=(BaseGenericService&& other) {
		setInstance(std::move(other.getInstance()));
		return *this;
	}
	
	BaseGenericService(const BaseGenericService& other) {
		setInstance(other.getInstance());
	}
	
	BaseGenericService& operator=(const BaseGenericService& other) {
		setInstance(other.getInstance());
		return *this;
	}
	
	BaseGenericService(ContainedType instance) {
		setInstance(std::move(instance));
	}
	
	~BaseGenericService() {
		if (_initiated) {
			getInstance().~ContainedType();
		}
	}
	
	template<typename T, typename = typename std::enable_if<std::is_base_of<CRTP, T>::value>::type>
	operator T () const & {
		T service;
		
		if (_initiated) {
			service.setInstance(getInstance());
		}
		
		return service;
	}
	
	template<typename T, typename = typename std::enable_if<std::is_base_of<CRTP, T>::value>::type>
	operator T () && {
		T service;
		
		if (_initiated) {
			service.setInstance(std::move(getInstance()));
		}
		
		return service;
	}
	
	template<typename F, F f, typename... T>
	void autocall(InjectType_t<T>... others) {
		(getInstance().*f)(others.forward()...);
	}
	
	template<typename F, F f, template<typename> class Map>
	void autocall(ContainerService cs) {
		autocall<Map>(cs, f);
	}
	
protected:
	ContainedType& getInstance() {
		return *reinterpret_cast<ContainedType*>(&_instance);
	}
	
private:
	template<template<typename> class Map, typename R, typename... Args>
	void autocall(ContainerService cs, R(ContainedType::*f)(Args...)) {
		cs.forward().invoke<Map>([this, &f](Args&&... args){
			(getInstance().*f)(std::forward<Args>(args)...);
		});
	}
	
	void setInstance(ContainedType instance) {
		new (&_instance) ContainedType(std::move(instance));
		_initiated = true;
	}
	
	bool _initiated = false;
	typename std::aligned_storage<sizeof(ContainedType), alignof(ContainedType)>::type _instance;
};

} // detail

template<typename CRTP, typename Type, typename Deps>
struct GenericService :
	detail::Injector<GenericService<CRTP, Type, Deps>, Deps>,
	detail::BaseGenericService<CRTP, Type>
{
	template<typename...> friend struct detail::Injector;
	using Self = GenericService<CRTP, Type, Deps>;
	using detail::BaseGenericService<CRTP, Type>::BaseGenericService;

private:
	using C = CRTP;
};

template<typename Type, typename Deps = Dependency<>>
struct SingleService : GenericService<SingleService<Type, Deps>, Type, Deps>, Single {
	using typename GenericService<SingleService<Type, Deps>, Type, Deps>::Self;
	using Self::Self;

	template<typename... Args>
	static Self makeService(Args&&... args) {
		return Self{Type{std::forward<Args>(args)...}};
	}

	virtual Type& forward() {
		return this->getInstance();
	}
};

template<typename Type, typename Deps = Dependency<>>
struct Service : GenericService<Service<Type, Deps>, Type, Deps> {
	using typename GenericService<Service<Type, Deps>, Type, Deps>::Self;
	using Self::Self;

	template<typename... Args>
	static Self makeService(Args&&... args) {
		return Self{Type{std::forward<Args>(args)...}};
	}

	Type forward() {
		return std::move(this->getInstance());
	}
};

template<typename T>
struct AbstractService : Single {
	virtual T& forward() = 0;
};

}
