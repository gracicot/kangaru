#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_AUTOCALL_TRAITS_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_AUTOCALL_TRAITS_HPP

#include "meta_list.hpp"
#include "traits.hpp"
#include "service_map.hpp"
#include "invoke.hpp"
#include "service_check.hpp"

#ifdef _MSC_VER
#ifndef __clang__
#define KGR_KANGARU_MSVC_NO_AUTOCALL_MAP_CHECK
#endif // !__clang__
#endif // _MSC_VER

namespace kgr {
namespace detail {

/*
 * Meta trait to apply a trait over all autocall entry in a autocall list
 */
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

/*
 * Alias for autocall_trait_helper
 */
template<template<typename...> class Trait, typename T>
using autocall_trait = typename autocall_trait_helper<Trait, T>::type;

/*
 * Trait that check if a all injected argument of a particular
 * autocall entry can be found in the service map.
 */
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
		service_map_t<meta_list_element_t<S, function_arguments_t<typename C::value_type>>, Map>...> = 0>
	static std::true_type test_helper(seq<S...>);
	
	template<typename U, typename C>
	static decltype(test_helper<typename U::Map, U, C>(tuple_seq<function_arguments_t<typename C::value_type>>{})) test(int);
	
public:
	using type = decltype(test<T, F>(0));
#else 
	using type = std::true_type;
#endif
};

/*
 * Alias for is_autocall_entry_map_complete_helper
 */
template<typename T, typename F>
using is_autocall_entry_map_complete = typename is_autocall_entry_map_complete_helper<T, F>::type;

/*
 * Trait that check if every injected arguments of a particular autocall entry are valid services.
 * 
 * However, we don't validate autocall checks for these injected services.
 */
template<typename T, typename F>
struct is_autocall_entry_valid_helper {
private:
	// This is workaround for clang. Clang will not interpret the name of the class itself
	// as a valid template template parameter. using this alias, it forces it to use the name as a template.
	template<typename U, typename C>
	using self_t = typename is_autocall_entry_valid_helper<U, C>::type;
	
	// Check when it's a method invocation
	struct invoke_method_condition {
		template<typename U, typename C, std::size_t I>
		using type = std::integral_constant<bool,
			service_check<meta_list_element_t<I, autocall_arguments_t<U, C>>>::value &&
			dependency_check<meta_list_element_t<I, autocall_arguments_t<U, C>>>::value>;
	};

	// Check when it's an invoke call
	struct invoke_call_condition {
		template<typename U, typename C, std::size_t I>
		using type = is_invoke_call<C>;
	};
	
	template<typename U, typename C, std::size_t I>
	struct expander {
		// If the map is incomplete, it's either an invalid
		// autocall entry, or an invoke call
		using type = typename std::conditional<
			is_autocall_entry_map_complete<U, C>::value,
			invoke_method_condition,
			invoke_call_condition
		>::type::template type<U, C, I>::type;
	};
	
	template<typename...>
	static std::false_type test(...);
	
	template<typename...>
	static std::false_type test_helper(...);
	
	template<typename U, typename C, std::size_t... S, int_t<
		enable_if_t<expander<U, C, S>::type::value>...> = 0>
	static std::true_type test_helper(seq<S...>);
	
	template<typename U, typename C, enable_if_t<is_valid_autocall_function<U, C>::value, int> = 0>
	static decltype(test_helper<U, C>(tuple_seq<function_arguments_t<typename C::value_type>>{})) test(int);
	
public:
	using type = decltype(test_helper<T, F>(tuple_seq<function_arguments_t<typename F::value_type>>{}));
};

/*
 * Alias for is_autocall_entry_valid_helper
 */
template<typename T, typename F>
using is_autocall_entry_valid = typename is_autocall_entry_valid_helper<T, F>::type;

/*
 * Validity check for autocall entries in a service T
 */
template<typename T>
using is_autocall_valid = autocall_trait<is_autocall_entry_valid, T>;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_AUTOCALL_TRAITS_HPP
