#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_CHECK_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_CHECK_HPP

#include "traits.hpp"
#include "function_traits.hpp"
#include "construct_function.hpp"
#include "override_traits.hpp"

namespace kgr {
namespace detail {

/*
 * Trait that check if the service definition can be constructed given the return type of it's construct function.
 */
template<typename T, typename... Args>
struct is_service_constructible_helper {
private:
	template<typename U, typename... As, std::size_t F, std::size_t... S>
	static is_service_instantiable<T,
		tuple_element_t<F, function_result_t<construct_function_t<U, As...>>>,
		tuple_element_t<S, function_result_t<construct_function_t<U, As...>>>...> test(seq<F, S...>);

	// This overload is needed for msvc.
	// Or else it will try to call the one just above with a 0 as S for strange reason.
	template<typename U, typename... As, int_t<construct_function_t<U, As...>> = 0>
	static is_service_instantiable<T> test(seq<>);
	
	template<typename U, typename... As, enable_if_t<is_supplied_service<U>::value || !has_any_construct<U, As...>::value, int> = 0>
	static std::true_type test_helper(int);
	
	template<typename...>
	static std::false_type test(...);
	
	template<typename...>
	static std::false_type test_helper(...);
	
	// The enable if is required here or else the function call will be ambiguous on visual studio.
	template<typename U, typename... As, enable_if_t<!is_supplied_service<U>::value && has_any_construct<U, As...>::value, int> = 0>
	static decltype(test<U, As...>(tuple_seq<function_result_t<construct_function_t<U, As...>>>{})) test_helper(int);
	
public:
	using type = decltype(test_helper<T, Args...>(0));
};

/*
 * Alias for is_service_constructible_helper
 */
template<typename T, typename... Args>
using is_service_constructible = typename is_service_constructible_helper<T, Args...>::type;

/*
 * Meta trait that applies a trait recursively for each dependencies and thier dependencies.
 */
template<template<typename...> class Trait, typename T, typename... Args>
struct dependency_trait_helper {
	template<typename U, std::size_t I, typename... As>
	struct expand {
		using type = Trait<injected_argument_t<I, construct_function_t<U, As...>>>;
	};

	template<typename U, typename... As, std::size_t... S, int_t<
		enable_if_t<dependency_trait_helper<Trait, injected_argument_t<S, construct_function_t<U, As...>>>::type::value>...,
		enable_if_t<expand<U, S, As...>::type::value>...> = 0>
		static std::true_type test(seq<S...>);

	template<typename U, typename... As, enable_if_t<is_supplied_service<U>::value || !has_any_construct<U, As... >::value, int> = 0>
	static std::true_type test_helper(int);

	template<typename...>
	static std::false_type test(...);

	template<typename...>
	static std::false_type test_helper(...);
	
	template<typename U, typename... As, enable_if_t<!is_supplied_service<U>::value && has_any_construct<U, As... >::value, int> = 0>
	static decltype(test<U, As...>(tuple_seq_minus<function_arguments_t<construct_function_t<U, As...>>, sizeof...(As)>{})) test_helper(int);
	
public:
	using type = decltype(test_helper<T, Args...>(0));
};

/*
 * Alias for dependency_trait_helper
 */
template<template<typename...> class Trait, typename T, typename... Args>
using dependency_trait = typename dependency_trait_helper<Trait, T, Args...>::type;

/*
 * Validity check for default services
 */
template<typename T>
using is_default_service_valid = std::integral_constant<bool,
	(is_abstract_service<T>::value || !has_default<T>::value) &&
	is_default_overrides_abstract<T>::value &&
	is_default_convertible<T>::value
>;

template<typename T>
using is_abstract_not_final = std::integral_constant<bool,
	!is_abstract_service<T>::value || !is_final_service<T>::value
>;

/*
 * Validity check for a service, without it's dependencies
 */
template<typename T, typename... Args>
using shallow_service_check = std::integral_constant<bool,
	is_service<T>::value &&
	is_service_constructible<T, Args...>::value &&
	is_construct_function_callable<T, Args...>::value &&
	is_default_service_valid<T>::value &&
	is_override_convertible<T>::value &&
	is_override_polymorphic<T>::value &&
	is_override_services<T>::value &&
	is_override_not_final<T>::value &&
	is_abstract_not_final<T>::value
>;

/*
 * Validity check for dependencies of a service
 */
template<typename T, typename... Args>
using dependency_check = dependency_trait<shallow_service_check, T, Args...>;

template<typename T, typename... Args>
struct service_check : bool_constant<
	shallow_service_check<T, Args...>::value && dependency_check<T, Args...>::value
> {};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_CHECK_HPP
