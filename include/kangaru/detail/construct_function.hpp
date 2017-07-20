#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONSTRUCT_FUNCTION_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONSTRUCT_FUNCTION_HPP

#include "meta_list.hpp"
#include "traits.hpp"
#include "injected.hpp"
#include "container_service.hpp"

namespace kgr {
namespace detail {

/*
 * Type trait that tell if the construct function F can be called with given arguments Args
 */
template<typename F, typename... Args>
struct is_construct_invokable_helper {
private:
	template<typename...>
	static std::false_type test_helper(...);
	
	template<typename C, typename... As, std::size_t... S, int_t<
		decltype(std::declval<C>()(std::declval<function_argument_t<S, C>>()..., std::declval<As>()...)),
		enable_if_t<is_service<injected_argument_t<S, C>>::value>...> = 0>
	static std::true_type test_helper(seq<S...>);
	
	template<typename C, typename... As>
	static decltype(test_helper<C, As...>(tuple_seq_minus<function_arguments_t<C>, sizeof...(As)>{})) test(int);
	
	template<typename...>
	static std::false_type test(...);
	
public:
	using type = decltype(test<F, Args...>(0));
};

/*
 * Alias for is_construct_invokable_helper
 */
template<typename F, typename... Args>
using is_construct_invokable = typename is_construct_invokable_helper<F, Args...>::type;

/*
 * has_callable_template_construct
 * 
 * This trait test if a particular template construct function is callable given a set of parameter
 */
template<typename, typename, typename, typename = void>
struct has_callable_template_construct : std::false_type {};

template<typename T, typename... TArgs, typename... Args>
struct has_callable_template_construct<
	T, meta_list<TArgs...>, meta_list<Args...>,
	enable_if_t<is_construct_invokable<decltype(&T::template construct<TArgs...>), Args...>::value>
> : std::true_type {};

/*
 * This trait will extract the first matching construct function that is callable.
 */
template<typename, typename, typename, typename = void>
struct get_template_construct_helper;

template<typename T, typename... Args>
struct get_template_construct_helper<
	T, meta_list<>, meta_list<Args...>,
	enable_if_t<!has_callable_template_construct<T, meta_list<>, meta_list<Args...>>::value>
> {};

template<typename T, typename Head, typename... Tail, typename... Args>
struct get_template_construct_helper<
	T, meta_list<Head, Tail...>, meta_list<Args...>,
	enable_if_t<!has_callable_template_construct<T, meta_list<Head, Tail...>, meta_list<Args...>>::value>
> : get_template_construct_helper<T, meta_list<Tail...>, meta_list<Args...>> {};

template<typename T, typename... TArgs, typename... Args>
struct get_template_construct_helper<
	T, meta_list<TArgs...>, meta_list<Args...>,
	enable_if_t<has_callable_template_construct<T, meta_list<TArgs...>, meta_list<Args...>>::value>
> : std::integral_constant<decltype(&T::template construct<TArgs...>), &T::template construct<TArgs...>> {};

/*
 * This is an alias for get_template_construct_helper
 */
template<typename T, typename... Args>
using get_template_construct = get_template_construct_helper<T, meta_list<Args...>, meta_list<Args...>>;

/*
 * This trait simply tells if a template construct function exists with a set of template parameter.
 */
template<typename, typename, typename = void>
struct template_construct_exist : std::false_type {};

template<typename T, typename... Args>
struct template_construct_exist<T, meta_list<Args...>, void_t<decltype(&T::template construct<Args...>)>> : std::true_type {};

/*
 * Trait that returns a pointer to the first existing template function, even if that function is not possibly callable.
 */
template<typename, typename, typename = void>
struct get_any_template_construct_helper;

template<typename T, typename Head, typename... Tail>
struct get_any_template_construct_helper<
	T, meta_list<Head, Tail...>,
	enable_if_t<!template_construct_exist<T, meta_list<Head, Tail...>>::value>
> : get_any_template_construct_helper<T, meta_list<Tail...>> {};

template<typename T, typename... Args>
struct get_any_template_construct_helper<
	T, meta_list<Args...>,
	enable_if_t<template_construct_exist<T, meta_list<Args...>>::value>
> : std::integral_constant<decltype(&T::template construct<Args...>), &T::template construct<Args...>> {};

/*
 * Alias for get_any_template_construct_helper
 */
template<typename T, typename... Args>
using get_any_template_construct = get_any_template_construct_helper<T, meta_list<Args...>>;

/*
 * Tells if there is any template construct function that exist in the service T
 */
template<typename, typename, typename = void>
struct has_any_template_construct_helper : std::false_type {};

template<typename T, typename... Args>
struct has_any_template_construct_helper<T, meta_list<Args...>, void_t<typename get_any_template_construct<T, Args...>::value_type>> : std::true_type {};

/*
 * Alias for has_any_template_construct_helper
 */
template<typename T, typename... Args>
using has_any_template_construct = has_any_template_construct_helper<T, meta_list<Args...>>;

/*
 * This trait tell if there is a callable template construct function
 */
template<typename, typename, typename = void>
struct has_template_construct_helper : std::false_type {};

template<typename T, typename... Args>
struct has_template_construct_helper<T, meta_list<Args...>, void_t<typename get_template_construct<T, Args...>::value_type>> : std::true_type {};

/*
 * Alias for has_template_construct_helper
 */
template<typename T, typename... Args>
using has_template_construct = has_template_construct_helper<T, meta_list<Args...>>;

template<bool, typename T, typename... Args>
struct construct_function_helper {
private:
	template<typename U>
	struct get_construct {
		using type = std::integral_constant<decltype(&U::construct), &U::construct>;
	};
	
	template<typename U, typename... As, enable_if_t<
		has_construct<U>::value &&
		!has_template_construct<U, As...>::value, int> = 0>
	static get_construct<U> test();
	
	template<typename U, typename... As, enable_if_t<has_template_construct<U, As...>::value, int> = 0>
	static get_template_construct<U, As...> test();
	
	using inner_type = decltype(test<T, Args...>());
	
public:
	using type = typename inner_type::type;
};

template<typename T, typename... Args>
struct construct_function_helper<false, T, Args...> {};

/*
 * Trait that returns if a service has a construct function, callable or not
 */
template<typename T, typename... Args>
struct has_any_construct {
	constexpr static bool value = has_any_template_construct<T, Args...>::value || has_construct<T>::value;
};

/*
 * Trait that returns if a service has a valid, callable construct function
 */
template<typename T, typename... Args>
using has_valid_construct = std::integral_constant<bool, has_template_construct<T, Args...>::value || has_construct<T>::value>;


/*
 * An alias to the selected construct function
 */
template<typename T, typename... Args>
using construct_function = typename construct_function_helper<has_valid_construct<T, Args...>::value, T, Args...>::type;

/*
 * The type of the selected construct function 
 */
template<typename T, typename... Args>
using construct_function_t = typename construct_function<T, Args...>::value_type;

/*
 * A sequence generator for the returned injected arguments
 */
template<typename T, typename... Args>
using construct_result_seq = tuple_seq<function_result_t<construct_function_t<T, Args...>>>;

/*
 * Returns if a construct function is callable given a set of parameter if there should be a construct function
 * 
 * If there is no construct function required for that service, returns true
 */
template<typename, typename, typename = void>
struct is_construct_function_callable_helper : std::false_type {};

template<typename T, typename... Args>
struct is_construct_function_callable_helper<
	T, meta_list<Args...>,
	enable_if_t<is_construct_invokable<construct_function_t<T, Args...>, Args...>::value>
> : std::true_type {};

template<typename T, typename... Args>
struct is_construct_function_callable_helper<
	T, meta_list<Args...>,
	enable_if_t<is_container_service<T>::value || is_abstract_service<T>::value>
> : std::true_type {};

/*
 * Alias for is_construct_function_callable_helper
 */
template<typename T, typename... Args>
using is_construct_function_callable = is_construct_function_callable_helper<T, meta_list<Args...>>;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_CONSTRUCT_FUNCTION_HPP
