#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INVOKE_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INVOKE_HPP

#include <type_traits>
#include "meta_list.hpp"
#include "utils.hpp"
#include "traits.hpp"
#include "injected.hpp"

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

namespace detail {

template<typename, typename = void>
struct has_map_entry : std::false_type {};

template<typename P>
struct has_map_entry<P, void_t<decltype(service_map(std::declval<P>(), std::declval<kgr::Map<> >()))>> : std::true_type {};

template<typename P>
struct has_map_entry<P, void_t<decltype(service_map(std::declval<P>()))>> : std::true_type {};

template<typename, typename = void>
struct AdlMapHelper {};

template<typename Parameter>
struct AdlMapHelper<Parameter, detail::enable_if_t<detail::has_map_entry<Parameter>::value>> {
private:
	template<typename P> // The space is needed on the next line for visual studio.
	static auto map(int) -> decltype(service_map(std::declval<P>(), std::declval<kgr::Map<> >()));
	
	template<typename P>
	static auto map(...) -> decltype(service_map(std::declval<P>()));
	
public:
	using Service = decltype(map<Parameter>(0));
};

} // namespace detail

template<typename P>
struct AdlMap : detail::AdlMapHelper<P> {};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INVOKE_HPP
