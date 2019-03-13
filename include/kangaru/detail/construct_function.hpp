#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONSTRUCT_FUNCTION_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONSTRUCT_FUNCTION_HPP

#include "meta_list.hpp"
#include "traits.hpp"
#include "injected.hpp"
#include "container_service.hpp"

namespace kgr {
namespace detail {

/*
 * Aliases used for detection.
 */
template<typename T>
using nontemplate_constuct_function_pointer_t = decltype(&T::construct);

template<typename T, typename... Args>
using template_constuct_function_pointer_t = decltype(&T::template construct<Args...>);

template<typename T>
using has_construct = is_detected<nontemplate_constuct_function_pointer_t, T>;

/*
 * Metafunction that returns a trait that check if a function is callable and services are valid.
 * Used for is_construct_invokable
 */
template<typename F, typename... Args>
struct curry_is_construct_invokable {
	template<typename... Services>
	using trait = bool_constant<
		kgr::detail::is_callable<F, Services..., Args...>::value &&
		conjunction<is_service<detected_t<injected_service_t, Services>>...>::value
	>;
};

/*
 * Type trait that tell if the construct function F can be called with given arguments Args
 */
template<typename F, typename... Args>
using is_construct_invokable = bool_constant<
	is_detected<function_arguments_t, F>::value &&
	expand_minus_n<
		sizeof...(Args),
		detected_or<meta_list<>, function_arguments_t, F>,
		curry_is_construct_invokable<F, Args...>::template trait
	>::value
>;

/*
 * has_callable_template_construct
 * 
 * This trait test if a particular template construct function is callable given a set of parameter
 */
template<typename, typename, typename, typename = void>
struct has_callable_template_construct : std::false_type {};

/*
 * Specialization of has_callable_template_construct.
 * 
 * We select this specialization if the function construct<TArgs...> exist within T,
 * and also if that function is invokable using Args... as provided arguments.
 */
template<typename T, typename... TArgs, typename... Args>
struct has_callable_template_construct<
	T, meta_list<TArgs...>, meta_list<Args...>,
	enable_if_t<is_detected<template_constuct_function_pointer_t, T, TArgs...>::value>
> : is_construct_invokable<template_constuct_function_pointer_t<T, TArgs...>, Args...> {};

/*
 * This trait will extract the first matching construct function that is callable.
 */
template<typename, typename, typename, typename = void>
struct get_template_construct_helper {};

/*
 * Specialization of get_template_construct_helper.
 * 
 * This specialization an iteration of the meta algorithm. No function has been found yet. We continue the loop.
 */
template<typename T, typename Head, typename... Tail, typename... Args>
struct get_template_construct_helper<
	T, meta_list<Head, Tail...>, meta_list<Args...>,
	enable_if_t<!has_callable_template_construct<T, meta_list<Head, Tail...>, meta_list<Args...>>::value>
> : get_template_construct_helper<T, meta_list<Tail...>, meta_list<Args...>> {};

/*
 * Specialization of get_template_construct_helper.
 * 
 * This specialization is an iteration of the meta algorithm. We get here when we found a suitable function to call.
 * The algorithm stops the loop and return the function.
 */
template<typename T, typename... TArgs, typename... Args>
struct get_template_construct_helper<
	T, meta_list<TArgs...>, meta_list<Args...>,
	enable_if_t<has_callable_template_construct<T, meta_list<TArgs...>, meta_list<Args...>>::value>
> : std::integral_constant<decltype(&T::template construct<TArgs...>), &T::template construct<TArgs...>> {};

/*
 * The starting point for get_template_construct_helper.
 */
template<typename T, typename... Args>
using get_template_construct = get_template_construct_helper<T, meta_list<Args...>, meta_list<Args...>>;

/*
 * This trait simply tells if a template construct function exists with a set of template parameter.
 * We don't check for validity of parameter, nor if it's invocable.
 */
template<typename T, typename... Args>
using template_construct_exist = is_detected<template_constuct_function_pointer_t, T, Args...>;

/*
 * Trait that returns a pointer to the first existing template function, even if that function is not possibly callable.
 * 
 * A lot similar to get_template_construct_helper, but we only check for the existance of the function, without validating it.
 */
template<typename, typename, typename = void>
struct get_any_template_construct_helper {};

template<typename T, typename Head, typename... Tail>
struct get_any_template_construct_helper<
	T, meta_list<Head, Tail...>,
	enable_if_t<!template_construct_exist<T, Head, Tail...>::value>
> : get_any_template_construct_helper<T, meta_list<Tail...>> {};

template<typename T, typename... Args>
struct get_any_template_construct_helper<
	T, meta_list<Args...>,
	enable_if_t<template_construct_exist<T, Args...>::value>
> : std::integral_constant<decltype(exact(&T::template construct<Args...>)), &T::template construct<Args...>> {};

/*
 * Alias for get_any_template_construct_helper
 */
template<typename T, typename... Args>
using get_any_template_construct = get_any_template_construct_helper<T, meta_list<Args...>>;

/*
 * Tells if there is any template construct function that exist in the service T
 */
template<typename T, typename... Args>
using has_any_template_construct = is_detected<value_type_t, get_any_template_construct<T, Args...>>;

/*
 * This trait tell if there is a callable template construct function
 */
template<typename T, typename... Args>
using has_template_construct = is_detected<value_type_t, get_template_construct<T, Args...>>;

/*
* Trait that returns if a service has a construct function, callable or not
*/
template<typename T, typename... Args>
struct has_any_construct {
	constexpr static bool value = has_any_template_construct<T, Args...>::value || has_construct<T>::value;
};

template<typename T>
struct get_nontemplate_construct {
	// We do not use integral_constant here, &T::construct might have no linkage.
	using value_type = decltype(&T::construct);
	constexpr static value_type value = &T::construct;
};

/*
 * Metafunction that returns the construct function as an integral_constant
 */
template<typename T, typename... Args>
using construct_function = conditional_t<
	has_any_template_construct<T, Args...>::value,
	get_template_construct<T, Args...>,
	instantiate_if_t<has_construct<T>::value, get_nontemplate_construct, T>
>;

/*
 * The type of the selected construct function 
 */
template<typename T, typename... Args>
using construct_function_t = typename construct_function<T, Args...>::value_type;

/*
 * The return type of the selected construct function 
 */
template<typename T, typename... Args>
using construct_function_result_t = function_result_t<construct_function_t<T, Args...>>;

/*
 * A sequence generator for the returned injected arguments
 */
template<typename T, typename... Args>
using construct_result_seq = tuple_seq<construct_function_result_t<T, Args...>>;

/*
* Trait that returns if a service has a valid, callable construct function
*/
template<typename T, typename... Args>
using has_valid_construct = bool_constant<has_template_construct<T, Args...>::value || has_construct<T>::value>;

/*
 * Returns if a construct function is callable given a set of parameter if there should be a construct function
 * 
 * If there is no construct function required for that service, returns true
 */
template<typename T, typename... Args>
struct is_construct_function_callable : bool_constant<
	is_construct_invokable<detected_t<construct_function_t, T, Args...>, Args...>::value ||
	is_container_service<T>::value || is_abstract_service<T>::value
> {};

/*
 * Returns if a construct function is callable, but only if it's not a supplied service.
 */
template<typename T, typename... Args>
using is_construct_function_callable_if_needed = bool_constant<
	is_construct_function_callable<T, Args...>::value || (is_supplied_service<T>::value && sizeof...(Args) == 0)
>;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONSTRUCT_FUNCTION_HPP
