#pragma once

#include <type_traits>
#include <tuple>
#include "utils.hpp"

namespace kgr {

namespace detail {

// tags for SFINAE
struct InvokeTag {};
struct InvokeCallTag {};

}

template<typename T, T t>
using Method = std::integral_constant<T, t>;

// forward declaration of Invoke
template<typename...>
struct Invoke;

// This class is the last node of the invoke list
template<>
struct Invoke<> {};

// This class is a list of methods.
template<typename M, typename... Others>
struct Invoke<M, Others...> : M, detail::InvokeTag {
	using Next = Invoke<Others...>;
};

// This specialization represent a method and a list of it's parameter.
template<typename M, typename... Others, typename... Ps>
struct Invoke<Invoke<M, Ps...>, Others...> : detail::InvokeCallTag, detail::InvokeTag, M {
	using Params = std::tuple<Ps...>;
	using Next = Invoke<Others...>;
};

template<template<typename> class M, typename... Ts>
struct AutoCall {
	using invoke = Invoke<Ts...>;
	
	template<typename T>
	using Map = M<T>;
};

template<typename... Ts>
struct AutoCallNoMap {
	using invoke = Invoke<Ts...>;
	
	template<typename>
	struct Map;
};

}
