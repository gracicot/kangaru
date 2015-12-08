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
		return CRTP::makeService(deps.forward()...);
	}
};

} // detail

template<typename CRTP, typename Type, typename Deps>
struct GenericService : detail::Injector<CRTP, Deps> {
	template<typename...> friend struct detail::Injector;
	
	GenericService() = default;
	
	GenericService(GenericService&& other) {
		setInstance(std::move(other.getInstance()));
	}
	
	GenericService& operator=(GenericService&& other) {
		setInstance(std::move(other.getInstance()));
		return *this;
	}
	
	GenericService(const GenericService& other) {
		setInstance(other.getInstance());
	}
	
	GenericService& operator=(const GenericService& other) {
		setInstance(other.getInstance());
		return *this;
	}
	
	GenericService(Type instance) {
		setInstance(std::move(instance));
	}
	
	~GenericService() {
		if (_initiated) {
			getInstance().~Type();
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
	
	
protected:
	using Self = CRTP;
	
	template<typename F, F f, typename... T>
	void autocall(detail::InjectType_t<T>... others) {
		(getInstance().*f)(others.forward()...);
	}
	
	template<typename F, F f, template<typename> class Map>
	void autocall(ContainerService cs) {
		autocall<Map>(cs, f);
	}
	
	Type& getInstance() {
		return *reinterpret_cast<Type*>(&_instance);
	}
	
private:
	template<template<typename> class Map, typename R, typename... Args>
	void autocall(ContainerService cs, R(Type::*f)(Args...)) {
		cs.forward().invoke<Map>([this, &f](Args&&... args){
			(getInstance().*f)(std::forward<Args>(args)...);
		});
	}
	
	void setInstance(Type instance) {
		new (&_instance) Type(std::move(instance));
		_initiated = true;
	}
	
	bool _initiated = false;
	typename std::aligned_storage<sizeof(Type), alignof(Type)>::type _instance;
};

template<typename Type, typename Deps = Dependency<>>
struct SingleService : GenericService<SingleService<Type, Deps>, Type, Deps>, Single {
	private: using Parent = GenericService<SingleService<Type, Deps>, Type, Deps>;
	
public:
	using typename Parent::Self;
	using Parent::Parent;

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
	private: using Parent = GenericService<Service<Type, Deps>, Type, Deps>;
	
public:
	using typename Parent::Self;
	using Parent::Parent;

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
