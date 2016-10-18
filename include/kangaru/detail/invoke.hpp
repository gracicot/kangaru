#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INVOKE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INVOKE_HPP

#include <type_traits>
#include <tuple>
#include "utils.hpp"

namespace kgr {

struct Container;

template<typename T, T t>
using Method = std::integral_constant<T, t>;

template<typename M, typename... Ps>
struct Invoke : M {
	using Parameters = std::tuple<Ps...>;
};

template<template<typename> class M, typename... Ts>
struct AutoCall {
	using Autocall = std::tuple<Ts...>;
	
	template<typename T>
	using Map = M<T>;
};

template<typename... Ts>
struct AutoCallNoMap {
	using Autocall = std::tuple<Ts...>;
};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INVOKE_HPP
