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

template<typename T, typename F>
struct autocall_function {
private:
	template<typename U>
	static U identity(U);

	template<typename U, typename C>
	struct get_member_autocall {
		using type = std::integral_constant<
			decltype(identity(&U::template autocall<C, U::template Map>)),
			&U::template autocall<C, U::template Map>
		>;
	};
	
	template<typename U, typename C, std::size_t... S>
	struct get_invoke_autocall {
		using type = std::integral_constant<
			decltype(identity(&U::template autocall<C, detail::meta_list_element_t<S, typename C::Parameters>...>)),
			&U::template autocall<C, detail::meta_list_element_t<S, typename C::Parameters>...>
		>;
	};
	
	template<typename U, typename C, std::size_t... S>
	static get_invoke_autocall<U, C, S...> test_helper(seq<S...>);
	
	template<typename U, typename C, enable_if_t<is_invoke_call<C>::value, int> = 0>
	static decltype(test_helper<U, C>(tuple_seq<typename C::Parameters>{})) test();
	
	template<typename U, typename C, enable_if_t<is_member_autocall<U, C>::value && !is_invoke_call<C>::value, int> = 0>
	static get_member_autocall<U, C> test();
	
	using inner_type = decltype(test<T, F>());
	
public:
	using type = typename inner_type::type;
};

template<typename, typename, typename = void>
struct autocall_arguments;

template<typename T, typename F>
struct autocall_arguments<T, F, enable_if_t<is_invoke_call<F>::value>> {
	using type = typename F::Parameters;
};

template<typename T, typename F>
struct autocall_arguments<T, F, enable_if_t<is_member_autocall<T, F>::value && !is_invoke_call<F>::value>> {
private:
	template<template<typename> class Map>
	struct mapped_type {
		template<typename U>
		using map = service_map_t<Map, U>;
	};
	
public:
	using type = meta_list_transform_t<function_arguments_t<typename F::value_type>, mapped_type<T::template Map>::template map>;
};

template<typename T, typename F>
using autocall_function_t = typename autocall_function<T, F>::type;

template<typename T, std::size_t I>
using autocall_nth_function = detail::autocall_function_t<T, detail::meta_list_element_t<I, typename T::Autocall>>;

template<typename T, std::size_t I>
using autocall_nth_function_t = typename detail::autocall_function_t<T, detail::meta_list_element_t<I, typename T::Autocall>>::value_type;

template<typename T, typename F>
using autocall_arguments_t = typename autocall_arguments<T, F>::type;

template<typename T, typename F>
using is_valid_autocall_function = std::integral_constant<bool,
	is_invoke_call<F>::value || is_member_autocall<T, F>::value
>;

template<typename, typename = void>
struct map_entry {};

template<typename P>
struct map_entry<P, enable_if_t<is_service<decltype(service_map(std::declval<P>(), std::declval<kgr::Map<> >()))>::value>> {
	using Service = decltype(service_map(std::declval<P>(), std::declval<kgr::Map<> >()));
};

template<typename P>
struct map_entry<P, enable_if_t<is_service<decltype(service_map(std::declval<P>()))>::value>> {
	using Service = decltype(service_map(std::declval<P>()));
};

} // namespace detail

template<typename P>
struct AdlMap : detail::map_entry<P> {};

} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INVOKE_HPP
