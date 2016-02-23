#pragma once

#include <type_traits>
#include <tuple>
#include "utils.hpp"

namespace kgr {

namespace detail {

// tags for SFINAE
struct AutocallTag {};
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

// This is either a list of method or a method with it's parameter types.
// May be cleaned to not clutter the class with every typedef in every versions.
template<typename M, typename... Others>
struct Invoke<M, Others...> : std::conditional<std::is_base_of<detail::InvokeTag, M>::value, detail::InvokeCallTag, M>::type, detail::InvokeTag {
	using Method = M;
	using Next = Invoke<Others...>;
	using Params = std::tuple<Others...>;
};

template<template<typename> class M, typename... Ts>
struct AutoCall : detail::AutocallTag {
	using invoke = Invoke<Ts...>;
	
	template<typename T>
	using Map = M<T>;
};

template<typename... Ts>
struct AutoCallNoMap : detail::AutocallTag {
	using invoke = Invoke<Ts...>;
};

}
