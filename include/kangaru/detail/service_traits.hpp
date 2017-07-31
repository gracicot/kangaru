#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_TRAITS_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_TRAITS_HPP

#include "traits.hpp"
#include "service_map.hpp"
#include "single.hpp"
#include "injected.hpp"
#include "invoke.hpp"
#include "container_service.hpp"
#include "construct_function.hpp"
#include "override_traits.hpp"
#include "invoke_function.hpp"

#ifdef _MSC_VER
#ifndef __clang__
#define KGR_KANGARU_MSVC_NO_AUTOCALL_MAP_CHECK
#endif // !__clang__
#endif // _MSC_VER

namespace kgr {
namespace detail {

template<template<typename...> class Trait, typename T, typename... Args>
struct dependency_trait_helper {
	template<typename U, std::size_t I>
	struct expand {
		using type = Trait<injected_argument_t<I, construct_function_t<U, Args...>>>;
	};
	
	template<typename U, typename... As, std::size_t... S, int_t<
		enable_if_t<dependency_trait_helper<Trait, injected_argument_t<S, construct_function_t<U, As...>>>::type::value>...,
		enable_if_t<expand<U, S>::type::value>...> = 0>
	static std::true_type test(seq<S...>);
	
	template<typename U, typename... As, enable_if_t<!has_any_construct<U, As... >::value, int> = 0>
	static std::true_type test_helper(int);
	
	template<typename...>
	static std::false_type test(...);
	
	template<typename...>
	static std::false_type test_helper(...);
	
	template<typename U, typename... As, enable_if_t<has_any_construct<U, As... >::value, int> = 0>
	static decltype(test<U, As...>(tuple_seq_minus<function_arguments_t<construct_function_t<U, As...>>, sizeof...(As)>{})) test_helper(int);
	
public:
	using type = decltype(test_helper<T, Args...>(0));
};

template<template<typename...> class Trait, typename T, typename... Args>
using dependency_trait = typename dependency_trait_helper<Trait, T, Args...>::type;

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
	
	template<typename U, typename... As, enable_if_t<!has_any_construct<U, As... >::value, int> = 0>
	static std::true_type test_helper(int);
	
	template<typename...>
	static std::false_type test(...);
	
	template<typename...>
	static std::false_type test_helper(...);
	
	// The enable if is required here or else the function call will be ambiguous on visual studio.
	template<typename U, typename... As, enable_if_t<has_any_construct<U, As... >::value, int> = 0>
	static decltype(test<U, As...>(tuple_seq<function_result_t<construct_function_t<U, As...>>>{})) test_helper(int);
	
public:
	using type = decltype(test_helper<T, Args...>(0));
};

template<typename T, typename... Args>
using is_service_constructible = typename is_service_constructible_helper<T, Args...>::type;

template<typename T, typename... Args>
using is_single_no_args = std::integral_constant<bool,
	!is_single<T>::value || meta_list_empty<meta_list<Args...>>::value
>;

template<typename T>
using is_default_service_valid = std::integral_constant<bool,
	(is_abstract_service<T>::value || !has_default<T>::value) &&
	is_default_overrides_abstract<T>::value &&
	is_default_convertible<T>::value
>;

template<typename T, typename... Args>
using service_check = std::integral_constant<bool, 
	is_service<T>::value &&
	is_service_constructible<T, Args...>::value &&
	is_construct_function_callable<T, Args...>::value &&
	is_default_service_valid<T>::value &&
	is_override_convertible<T>::value &&
	is_override_virtual<T>::value &&
	is_override_services<T>::value
>;

template<typename T, typename... Args>
using dependency_check = std::integral_constant<bool, 
	dependency_trait<is_service, T, Args...>::value &&
	dependency_trait<is_service_constructible, T, Args...>::value &&
	dependency_trait<is_construct_function_callable, T, Args...>::value &&
	dependency_trait<is_default_service_valid, T, Args...>::value &&
	dependency_trait<is_override_convertible, T, Args...>::value &&
	dependency_trait<is_override_services, T, Args...>::value
>;

template<template<typename...> class Trait, typename T>
struct autocall_trait_helper {
private:
	template<typename U, std::size_t I>
	struct expand {
		using type = Trait<U, meta_list_element_t<I, typename U::Autocall>>;
	};

	template<typename>
	static std::false_type test_helper(...);
	
	template<typename U, std::size_t... S, int_t<
		enable_if_t<expand<U, S>::type::value>...> = 0>
	static std::true_type test_helper(seq<S...>);
	
	template<typename>
	static std::true_type test(...);
	
	template<typename U>
	static decltype(test_helper<U>(detail::tuple_seq<typename U::Autocall>{})) test(int);
	
public:
	using type = decltype(test<T>(0));
};

template<template<typename...> class Trait, typename T>
using autocall_trait = typename autocall_trait_helper<Trait, T>::type;

template<typename T, typename F>
struct is_autocall_entry_map_complete_helper {
#ifndef KGR_KANGARU_MSVC_NO_AUTOCALL_MAP_CHECK
private:
	template<typename U, typename C, enable_if_t<is_invoke_call<C>::value, int> = 0>
	static std::true_type test(...);
	
	template<typename U, typename C, enable_if_t<!is_invoke_call<C>::value, int> = 0>
	static std::false_type test(...);
	
	template<typename>
	static std::false_type test_helper(...);
	
	template<typename Map, typename U, typename C, std::size_t... S, int_t<
		service_map_t<Map, meta_list_element_t<S, function_arguments_t<typename C::value_type>>>...> = 0>
	static std::true_type test_helper(seq<S...>);
	
	template<typename U, typename C>
	static decltype(test_helper<typename U::Map, U, C>(tuple_seq<function_arguments_t<typename C::value_type>>{})) test(int);
	
public:
	using type = decltype(test<T, F>(0));
#else 
	using type = std::true_type;
#endif
};

template<typename T, typename F>
using is_autocall_entry_map_complete = typename is_autocall_entry_map_complete_helper<T, F>::type;

template<typename T, typename F>
struct is_autocall_entry_valid_helper {
private:
	// This is workaround for clang. Clang will not interpret the name of the class itself
	// as a valid template template parameter. using this alias, it forces it to use the name as a template.
	template<typename U, typename C>
	using self_t = typename is_autocall_entry_valid_helper<U, C>::type;
	
	struct invoke_method_condition {
		template<typename U, typename C, std::size_t I>
		using type = std::integral_constant<bool,
			service_check<meta_list_element_t<I, autocall_arguments_t<U, C>>>::value &&
			dependency_check<meta_list_element_t<I, autocall_arguments_t<U, C>>>::value>;
	};

	struct invoke_call_condition {
		template<typename U, typename C, std::size_t I>
		using type = is_invoke_call<C>;
	};

	template<typename U, typename C, std::size_t I>
	struct expander {
		using type = typename std::conditional<
			is_autocall_entry_map_complete<U, C>::value,
			invoke_method_condition,
			invoke_call_condition
		>::type::template type<U, C, I>::type;
	};
	
	template<typename...>
	static std::false_type test(...);
	
	template<typename>
	static std::false_type test_helper(...);
	
	template<typename U, typename C, std::size_t... S, int_t<
		enable_if_t<expander<U, C, S>::type::value>...> = 0>
	static std::true_type test_helper(seq<S...>);
	
	template<typename U, typename C, enable_if_t<is_valid_autocall_function<U, C>::value, int> = 0>
	static decltype(test_helper<U, C>(tuple_seq<function_arguments_t<typename C::value_type>>{})) test(int);
	
public:
	using type = decltype(test_helper<T, F>(tuple_seq<function_arguments_t<typename F::value_type>>{}));
};

template<typename T, typename F>
using is_autocall_entry_valid = typename is_autocall_entry_valid_helper<T, F>::type;

template<typename T>
using is_autocall_valid = autocall_trait<is_autocall_entry_valid, T>;

template<typename T, typename... Args>
struct is_service_valid : std::integral_constant<bool,
	is_single_no_args<T, Args...>::value &&
	service_check<T, Args...>::value &&
	dependency_check<T, Args...>::value &&
	is_autocall_valid<T>::value &&
	dependency_trait<is_autocall_valid, T, Args...>::value
> {};

template<typename Map, typename T, typename... Args>
struct is_invoke_service_valid_helper {
private:
	template<typename U, std::size_t I>
	struct expander {
		using type = std::integral_constant<bool, is_service_valid<service_map_t<Map, invoke_function_argument_t<I, Map, U, Args...>>>::value>;
	};

	template<typename U>
	using map_t = service_map_t<Map, U>;
	
	template<typename U, typename... As, std::size_t... S, int_t<map_t<invoke_function_argument_t<S, Map, U, As...>>..., enable_if_t<expander<U, S>::type::value>...> = 0>
	static std::true_type test_helper(seq<S...>);
	
	template<typename...>
	static std::false_type test_helper(...);
	
	template<typename U, typename... As>
	static decltype(test_helper<U, As...>(tuple_seq_minus<invoke_function_arguments_t<Map, U, As...>, sizeof...(Args)>{})) test(int);
	
	template<typename...>
	static std::false_type test(...);
	
public:
	using type = decltype(test<T, Args...>(0));
};

template<typename Map, typename T, typename... Args>
struct is_invoke_service_valid : is_invoke_service_valid_helper<Map, T, Args...>::type {};

template<typename Map, typename T, typename... Args>
using is_invoke_valid = std::integral_constant<bool,
	is_invoke_service_valid<Map, T, Args...>::value &&
	is_invokable<Map, T, Args...>::value
>;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_SERVICE_TRAITS_HPP
