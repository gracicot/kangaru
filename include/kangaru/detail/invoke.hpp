#pragma once

#include <type_traits>

namespace kgr {

template<typename T, T t>
using Method = std::integral_constant<T, t>;

template<typename...>
struct Invoke;

template<>
struct Invoke<> {};

template<typename Method, typename... Others>
struct Invoke<Method, Others...> : Method {
	using Next = Invoke<Others...>;
};

template<template<typename> class M, typename... Ts>
struct AutoCall {
	using AutoCallType = Invoke<Ts...>;
	
	template<typename T>
	using Map = M<T>;
};

}
