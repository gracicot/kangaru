#pragma once

#include "../container.hpp"
#include "invoke.hpp"

#include <type_traits>

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
	
	using Self = CRTP;
	
	// TODO: to remove kangaru 3
	template<typename F, F f, typename... T>
	void autocall(detail::inject_t<T>... others) {
		CRTP::call(getInstance(), f, others.forward()...);
	}
	
	template<typename F, typename... T>
	void autocall(detail::inject_t<T>... others) {
		CRTP::call(getInstance(), F::value, others.forward()...);
	}
	
	// TODO: to remove kangaru 3
	template<typename F, F f, template<typename> class Map>
	void autocall(ContainerService cs) {
		autocall<Map, Method<F, f>>(detail::tuple_seq<detail::function_arguments_t<F>>{}, cs);
	}
	
	template<typename F, template<typename> class Map>
	void autocall(ContainerService cs) {
		autocall<Map, F>(detail::tuple_seq<detail::function_arguments_t<typename F::value_type>>{}, cs);
	}
	
protected:
	Type& getInstance() {
		return *reinterpret_cast<Type*>(&_instance);
	}
	
	const Type& getInstance() const {
		return *reinterpret_cast<const Type*>(&_instance);
	}
	
private:
	template<template<typename> class Map, typename F, int... S>
	void autocall(detail::seq<S...>, ContainerService cs) {
		cs.forward().invoke<Map>([this](detail::function_argument_t<S, typename F::value_type>... args){
			CRTP::call(getInstance(), F::value, std::forward<detail::function_argument_t<S, typename F::value_type>>(args)...);
		});
	}
	
	void setInstance(Type instance) {
		new (&_instance) Type(std::move(instance));
		_initiated = true;
	}
	
	bool _initiated = false;
	typename std::aligned_storage<sizeof(Type), alignof(Type)>::type _instance;
};

}
