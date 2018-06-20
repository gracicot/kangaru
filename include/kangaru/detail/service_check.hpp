#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_CHECK_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_CHECK_HPP

#include "traits.hpp"
#include "function_traits.hpp"
#include "construct_function.hpp"
#include "override_traits.hpp"

namespace kgr {
namespace detail {

#ifndef KGR_KANGARU_MSVC_DEPENDENCY_TRAIT_STRUCT
#if _MSC_VER == 1900
#ifndef __clang__
// MSVC has a defect that makes the use of the template keyword an error in some corner cases.
#define KGR_KANGARU_MSVC_DEPENDENCY_TRAIT_STRUCT
#endif // !__clang__
#endif // _MSC_VER
#endif // KGR_KANGARU_MSVC_NO_DEPENDENT_TEMPLATE_KEYWORD

/*
 * Trait that check if the service definition can be constructed given the return type of it's construct function.
 */
template<typename T, typename... Args>
using is_service_constructible = bool_constant<
	is_supplied_service<T>::value || !has_any_construct<T, Args...>::value ||
	(is_tuple<detected_t<function_result_t, detected_t<construct_function_t, T, Args...>>>::value &&
	expand_all<
		meta_list_push_front_t<T, to_meta_list_t<detected_or<std::tuple<>, function_result_t, detected_t<construct_function_t, T, Args...>>>>,
		is_service_instantiable
	>::value)
>;

/*
 * Meta trait that applies a trait recursively for each dependencies and thier dependencies.
 */
template<template<typename...> class Trait, typename T, typename... Args>
struct dependency_trait {
	template<typename... Service>
	struct service_check_dependencies {
		static constexpr bool value = conjunction<
			Trait<detected_t<injected_service_t, Service>>...,
			dependency_trait<Trait, detected_t<injected_service_t, Service>, Args...>...
		>::value;
	};

	static constexpr bool value =
		is_supplied_service<T>::value ||
		expand_n<
			safe_minus(detected_or<std::integral_constant<int, 0>, meta_list_size, detected_or<meta_list<>, function_arguments_t, detected_t<construct_function_t, T, Args...>>>::value, sizeof...(Args)),
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
