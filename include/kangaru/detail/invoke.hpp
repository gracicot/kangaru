#ifndef KGR_INCLUDE_KANGARU_DETAIL_INVOKE_HPP
#define KGR_INCLUDE_KANGARU_DETAIL_INVOKE_HPP

#include <type_traits>
#include <tuple>
#include "utils.hpp"

namespace kgr {

template<typename T, T t>
using Method = std::integral_constant<T, t>;

template<typename M, typename... Ps>
struct Invoke : M {
	using Parameters = std::tuple<Ps...>;
};

template<template<typename> class M, typename... Ts>
struct AutoCall {
	using invoke = std::tuple<Ts...>;
	
	template<typename T>
	using Map = M<T>;
};

template<typename... Ts>
struct AutoCallNoMap {
	using invoke = std::tuple<Ts...>;
};

} // namespace kgr

#endif // KGR_INCLUDE_KANGARU_DETAIL_INVOKE_HPP
