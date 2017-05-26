#ifndef KGR_KANGARU_INCLUDE_KANGARU_AUTOCALL_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_AUTOCALL_HPP

#include "detail/meta_list.hpp"
#include "detail/utils.hpp"
#include "detail/traits.hpp"
#include "detail/injected.hpp"
#include "detail/container_service.hpp"
#include "container.hpp"

namespace kgr {
namespace detail {

struct AutoCallBase {
	template<typename T, typename F, typename... Ts>
	void autocall(Inject<Ts>... others) {
		static_cast<T*>(this)->call(F::value, std::forward<Inject<Ts>>(others).forward()...);
	}
	
	template<typename T, typename F, template<typename> class Map>
	void autocall(Inject<ContainerService> cs) {
		autocall<T, Map, F>(detail::tuple_seq<detail::function_arguments_t<typename F::value_type>>{}, std::move(cs));
	}
	
	template<typename T, template<typename> class Map, typename F, std::size_t... S>
	void autocall(detail::seq<S...>, Inject<ContainerService> cs) {
		cs.forward().invoke<Map>([this](detail::function_argument_t<S, typename F::value_type>... args){
			static_cast<T*>(this)->call(F::value, std::forward<detail::function_argument_t<S, typename F::value_type>>(args)...);
		});
	}
};

} // namespace detail

template<typename T, T t>
using Method = std::integral_constant<T, t>;

template<typename M, typename... Ps>
struct Invoke : M {
	using Parameters = detail::meta_list<Ps...>;
};

template<template<typename> class M, typename... Ts>
struct AutoCall : detail::AutoCallBase {
	using Autocall = detail::meta_list<Ts...>;
	
	template<typename T>
	using Map = M<T>;
};

template<typename... Ts>
struct AutoCallNoMap : detail::AutoCallBase {
	using Autocall = detail::meta_list<Ts...>;
};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_AUTOCALL_HPP
