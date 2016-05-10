#pragma once

#include "../container.hpp"

namespace kgr {

template<typename... Args>
struct Dependency {};

namespace detail {


template<typename T, typename = void>
struct inject {
	using type = T&;
};

template<typename T>
struct inject<T, enable_if_t<is_service<T>::value>> {
	using type = typename std::conditional<std::is_base_of<Single, T>::value, T&, T&&>::type;
};

template<typename T>
using inject_t = typename inject<T>::type;

template<typename...>
struct Injector;

template<typename CRTP, typename... Deps>
struct Injector<CRTP, Dependency<Deps...>> {
	static CRTP construct(inject_t<Deps>... deps) {
		return CRTP::makeService(std::forward<inject_t<Deps>>(deps).forward()...);
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
	
	template<typename T, detail::enable_if_t<std::is_same<Type, typename std::decay<T>::type>::value, int> = 0>
	GenericService(T&& instance) {
		setInstance(std::forward<T>(instance));
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
	void autocall(detail::inject_t<T>... others) {
		CRTP::call(getInstance(), f, others.forward()...);
	}
	
	template<typename F, F f, template<typename> class Map>
	void autocall(ContainerService cs) {
		autocall<Map, F, f>(detail::tuple_seq<detail::function_arguments_t<F>>{}, cs);
	}
	
	Type& getInstance() {
		return *reinterpret_cast<Type*>(&_instance);
	}
	
	const Type& getInstance() const {
		return *reinterpret_cast<const Type*>(&_instance);
	}
	
private:
	template<template<typename> class Map, typename F, F f, int... S>
	void autocall(detail::seq<S...>, ContainerService cs) {
		cs.forward().invoke<Map>([this](detail::function_argument_t<S, F>... args){
			CRTP::call(getInstance(), f, std::forward<detail::function_argument_t<S, F>>(args)...);
		});
	}
	
	template<typename T>
	void setInstance(T&& instance) {
		new (&_instance) Type(std::forward<T>(instance));
		_initiated = true;
	}
	
	bool _initiated = false;
	typename std::aligned_storage<sizeof(Type), alignof(Type)>::type _instance;
};

}
