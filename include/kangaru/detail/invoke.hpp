#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INVOKE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INVOKE_HPP

#include <type_traits>
#include "meta_list.hpp"
#include "utils.hpp"

namespace kgr {

struct Container;

template<typename T, T t>
using Method = std::integral_constant<T, t>;

template<typename M, typename... Ps>
struct Invoke : M {
	using Parameters = detail::meta_list<Ps...>;
};

template<template<typename> class M, typename... Ts>
struct AutoCall {
	using Autocall = detail::meta_list<Ts...>;
	
	template<typename T>
	using Map = M<T>;
};

template<typename... Ts>
struct AutoCallNoMap {
	using Autocall = detail::meta_list<Ts...>;
};

template<typename Parameter>
struct AdlMap {
	using Service = decltype(service_map(std::declval<Parameter>()));
};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INVOKE_HPP
