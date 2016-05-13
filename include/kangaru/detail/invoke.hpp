#ifndef KGR_INCLUDE_KANGARU_DETAIL_INVOKE_HPP
#define KGR_INCLUDE_KANGARU_DETAIL_INVOKE_HPP

#include <type_traits>
#include <tuple>
#include "utils.hpp"

namespace kgr {

template<typename T, T t>
using Method = std::integral_constant<T, t>;

// forward declaration of Invoke
template<typename...>
struct Invoke;

// This class is the last node of the invoke list
template<typename T, T t>
struct Invoke<Method<T, t>> : Method<T, t> {};

// This class is a list of methods.
template<typename M, typename... Others>
struct Invoke<M, Others...> : Invoke<M> {
	using Next = Invoke<Others...>;
};

// This specialization represent a method and a list of it's parameter.
template<typename M, typename... Others, typename... Ps>
struct Invoke<Invoke<M, Ps...>, Others...> : Invoke<M, Others...> {
	using Params = std::tuple<Ps...>;
};

template<template<typename> class M, typename First, typename... Ts>
struct AutoCall {
	using invoke = Invoke<First, Ts...>;
	
	template<typename T>
	using Map = M<T>;
};

template<typename First, typename... Ts>
struct AutoCallNoMap {
	using invoke = Invoke<First, Ts...>;
	
	template<typename>
	struct Map;
};

} // namespace kgr

#endif // KGR_INCLUDE_KANGARU_DETAIL_INVOKE_HPP
