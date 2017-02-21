#ifndef KGR_KANGARU_INCLUDE_KANGARU_GENERIC_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_GENERIC_HPP

#include "container.hpp"
#include "detail/injected.hpp"
#include "detail/traits.hpp"

namespace kgr {
namespace detail {

template<typename Generic, typename Type, typename = void>
struct GenericServiceDestruction {
	~GenericServiceDestruction() {
		static_cast<Generic&>(*this).instance().~Type();
	}
};

template<typename Generic, typename Type>
struct GenericServiceDestruction<Generic, Type, enable_if_t<std::is_trivially_destructible<Type>::value>> {};

template<typename, typename> struct autocall_function;

} // namespace detail

template<typename CRTP, typename Type>
struct GenericService : detail::GenericServiceDestruction<GenericService<CRTP, Type>, Type> {
	friend struct Container;
	using Self = CRTP;
	
	GenericService() = default;
	
	GenericService(GenericService&& other) {
		emplace(std::move(other.instance()));
	}
	
	GenericService& operator=(GenericService&& other) {
		emplace(std::move(other.instance()));
		return *this;
	}
	
	GenericService(const GenericService& other) {
		emplace(other.instance());
	}
	
	GenericService& operator=(const GenericService& other) {
		emplace(other.instance());
		return *this;
	}
	
	template<typename... Args, detail::enable_if_t<detail::is_someway_constructible<Type, Args...>::value, int> = 0>
	GenericService(in_place_t, Args&&... args) {
		emplace(std::forward<Args>(args)...);
	}
	
protected:
	Type& instance() {
		return *reinterpret_cast<Type*>(&_instance);
	}
	
	const Type& instance() const {
		return *reinterpret_cast<const Type*>(&_instance);
	}
	
private:
	template<typename, typename...> friend struct detail::has_emplace_helper;
	template<typename, typename> friend struct detail::autocall_function;
	friend struct detail::GenericServiceDestruction<GenericService<CRTP, Type>, Type>;
	
	template<typename... Args, detail::enable_if_t<std::is_constructible<Type, Args...>::value, int> = 0>
	void emplace(Args&&... args) {
		new (&_instance) Type(std::forward<Args>(args)...);
	}
	
	template<typename... Args, detail::enable_if_t<detail::is_only_brace_constructible<Type, Args...>::value, int> = 0>
	void emplace(Args&&... args) {
		new (&_instance) Type{std::forward<Args>(args)...};
	}
	
	template<typename F, typename... Ts>
	void autocall(Inject<Ts>... others) {
		CRTP::call(instance(), F::value, std::forward<Inject<Ts>>(others).forward()...);
	}
	
	template<typename F, template<typename> class Map>
	void autocall(Inject<ContainerService> cs) {
		autocall<Map, F>(detail::tuple_seq<detail::function_arguments_t<typename F::value_type>>{}, std::move(cs));
	}
	
	template<template<typename> class Map, typename F, std::size_t... S>
	void autocall(detail::seq<S...>, Inject<ContainerService> cs) {
		cs.forward().invoke<Map>([this](detail::function_argument_t<S, typename F::value_type>... args){
			CRTP::call(instance(), F::value, std::forward<detail::function_argument_t<S, typename F::value_type>>(args)...);
		});
	}
	
	typename std::aligned_storage<sizeof(Type), alignof(Type)>::type _instance;
};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_GENERIC_HPP
