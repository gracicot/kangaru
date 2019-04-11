#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_CHECK_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_CHECK_HPP

#include "traits.hpp"
#include "function_traits.hpp"
#include "construct_function.hpp"
#include "override_traits.hpp"

namespace kgr {
namespace detail {

template<typename T, typename... Args>
using curry_is_constructible_from_construct = expand_all<
	to_meta_list_t<detected_or<std::tuple<>, function_result_t, detected_t<construct_function_t, T, Args...>>>,
	is_service_instantiable, T
>;

/*
 * Trait that check if the service construct function can be called and returns arguments that construct the service.
 */
template<typename T, typename... Args>
using is_constructible_from_construct = instantiate_if_or<
	is_tuple<detected_t<function_result_t, detected_t<construct_function_t, T, Args...>>>::value,
	std::false_type, curry_is_constructible_from_construct, T, Args...
>;

/*
 * Trait that check if the service definition can be constructed given the return type of it's construct function.
 */
template<typename T, typename... Args>
using is_service_constructible = instantiate_if_or<
	!is_abstract_service<T>::value && !is_container_service<T>::value, std::true_type,
	is_constructible_from_construct, T, Args...
>;

template<typename T, typename... Args>
using is_service_constructible_if_required = bool_constant<
	is_service_constructible<T, Args...>::value || is_supplied_service<T>::value
>;

/*
 * Meta trait that applies a trait recursively for each dependencies and thier dependencies.
 */
template<template<typename...> class Trait, typename T, typename... Args>
struct dependency_trait {
	// This subtrait recursively call dependency_trait for each dependencies
	template<typename... Service>
	struct service_check_dependencies {
		static constexpr bool value = conjunction<
			Trait<detected_t<injected_service_t, Service>>...,
			dependency_trait<Trait, detected_t<injected_service_t, Service>>...
		>::value;
	};
	
	static constexpr bool value =
		is_supplied_service<T>::value ||
		expand_minus_n<
			sizeof...(Args),
			detected_or<meta_list<>, function_arguments_t, detected_t<construct_function_t, T, Args...>>,
			service_check_dependencies
		>::value;
};

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

template<typename T>
using polymorphic_service_check = bool_constant<
	is_default_service_valid<T>::value &&
	is_override_convertible<T>::value &&
	is_override_polymorphic<T>::value &&
	is_override_services<T>::value &&
	is_override_not_final<T>::value &&
	is_abstract_not_final<T>::value
>;

/*
 * Validity check for a service, without it's dependencies
 */
template<typename T, typename... Args>
using shallow_service_check = std::integral_constant<bool,
	is_service<T>::value &&
	(is_service_constructible<T, Args...>::value || is_supplied_service<T>::value) &&
	(is_construct_function_callable_if_needed<T, Args...>::value || is_supplied_service<T>::value) &&
	instantiate_if_or<is_polymorphic<T>::value, std::true_type, polymorphic_service_check, T>::value
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
