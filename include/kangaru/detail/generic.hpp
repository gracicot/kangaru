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
	static decltype(auto) construct(inject_t<Deps>... deps) {
		return CRTP::makeService(std::forward<inject_t<Deps>>(deps).forward()...);
	}
};

} // detail

template<typename CRTP, typename Type, typename Deps>
struct GenericService : detail::Injector<CRTP, Deps> {
	friend struct Container;
	using Self = CRTP;
	
	GenericService() = default;
	
	GenericService(GenericService&& other) {
		if (other._initialized) {
			emplace(std::move(other.getInstance()));
		}
	}
	
	GenericService& operator=(GenericService&& other) {
		if (other._initialized) {
			emplace(std::move(other.getInstance()));
		}
		return *this;
	}
	
	GenericService(const GenericService& other) {
		if (other._initialized) {
			emplace(other.getInstance());
		}
	}
	
	GenericService& operator=(const GenericService& other) {
		if (other._initialized) {
			emplace(other.getInstance());
		}
		return *this;
	}
	
	template<typename... Args>
	GenericService(in_place_t, Args&&... args) {
		emplace(std::forward<Args>(args)...);
	}
	
	~GenericService() {
		if (_initialized) {
			getInstance().~Type();
		}
	}
	
protected:
	template<typename F, typename... T>
	void autocall(detail::inject_t<T>... others) {
		CRTP::call(getInstance(), F::value, others.forward()...);
	}
	
	template<typename F, template<typename> class Map>
	void autocall(ContainerService cs) {
		autocall<Map, F>(detail::tuple_seq<detail::function_arguments_t<typename F::value_type>>{}, cs);
	}
	
	Type& getInstance() {
		return *reinterpret_cast<Type*>(&_instance);
	}
	
	const Type& getInstance() const {
		return *reinterpret_cast<const Type*>(&_instance);
	}
	
private:
	template<typename... Args>
	void emplace(Args&&... args) {
		if (_initialized) {
			getInstance().~Type();
		}
		
		_initialized = true;
		new (&_instance) Type(std::forward<Args>(args)...);
	}
	
	template<template<typename> class Map, typename F, int... S>
	void autocall(detail::seq<S...>, ContainerService cs) {
		cs.forward().invoke<Map>([this](detail::function_argument_t<S, typename F::value_type>... args){
			CRTP::call(getInstance(), F::value, std::forward<detail::function_argument_t<S, typename F::value_type>>(args)...);
		});
	}
	
	bool _initialized = false;
	typename std::aligned_storage<sizeof(Type), alignof(Type)>::type _instance;
};

} // namespace kgr
