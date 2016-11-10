#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_TRAITS_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_TRAITS_HPP

#include "function_traits.hpp"
#include "utils.hpp"
#include "meta_list.hpp"

#include <type_traits>
#include <tuple>

namespace kgr {
namespace detail {

// void_t implementation
template<typename...>
struct voider { using type = void; };

template<typename... Ts> using void_t = typename voider<Ts...>::type;

template<typename...>
struct to_false {
	using type = std::false_type;
};

template<typename... Ts>
using false_t = typename to_false<Ts...>::type;

template<typename...>
struct to_int {
	using type = int;
};

template<typename... Ts>
using int_t = typename to_int<Ts...>::type;

template<typename T>
struct identity { using type = T; };

template<typename T>
using identity_t = typename identity<T>::type;

// things missing from c++11 (to be removed when switching to c++14)
template <bool b, typename T = void>
using enable_if_t = typename std::enable_if<b, T>::type;

template<typename T>
using decay_t = typename std::decay<T>::type;

template<std::size_t S, typename T>
using tuple_element_t = typename std::tuple_element<S, T>::type;

template<std::size_t ...>
struct seq {};

template<std::size_t n, std::size_t ...S>
struct seq_gen : seq_gen<n-1, n-1, S...> {};

template<std::size_t ...S>
struct seq_gen<0, S...> {
	using type = seq<S...>;
};

template<typename>
struct TupleSeqGen;

template<typename... Types>
struct TupleSeqGen<std::tuple<Types...>> : seq_gen<sizeof...(Types)> {};

template<typename... Types>
struct TupleSeqGen<detail::meta_list<Types...>> : seq_gen<sizeof...(Types)> {};

template<typename Tuple>
using tuple_seq = typename TupleSeqGen<Tuple>::type;

template<typename List, int n>
using tuple_seq_minus = typename detail::seq_gen<meta_list_size<List>::value - (n > meta_list_size<List>::value ? meta_list_size<List>::value : n)>::type;

// SFINAE utilities
template<typename From, typename To>
using is_explicitly_convertible = std::is_constructible<To, From>;

template<typename T, typename = void>
struct has_autocall : std::false_type {};

template<typename T>
struct has_autocall<T, void_t<typename T::Autocall>> : std::true_type {};

template<typename T, typename = void>
struct has_forward : std::false_type {};

template<typename T>
struct has_forward<T, void_t<decltype(std::declval<T>().forward())>> : std::true_type {};

template<typename T, typename = void>
struct is_service : std::false_type {};

template<typename T>
struct is_service<T, enable_if_t<(!std::is_polymorphic<T>::value || std::is_abstract<T>::value) && has_forward<T>::value>> : std::true_type {};

template<typename T, typename = void>
struct has_construct : std::false_type {};

template<typename T>
struct has_construct<T, void_t<decltype(&T::construct)>> : std::true_type {};

template<typename T, typename = void>
struct is_invoke_call : std::false_type {};

template<typename T>
struct is_invoke_call<T, void_t<typename T::Parameters>> : std::true_type {};

template<template<typename> class Map, typename T, typename = void>
struct is_complete_map : std::false_type {};

template<template<typename> class Map, typename T>
struct is_complete_map<Map, T, void_t<typename Map<T>::Service>> : std::true_type {};

struct Sink {
	constexpr Sink() = default;
	
	template<typename T>
	constexpr operator T&& () const;
};

template<typename T, typename... Args>
struct is_brace_constructible_helper {
private:
	template<typename U, typename... As>
	static decltype(static_cast<void>(U{std::declval<As>()...}), std::true_type{}) test(int);
	
	template<typename...>
	static std::false_type test(...);
	
public:
	using type = decltype(test<T, Args...>(0));
};

template<typename T, typename... Args>
struct has_template_construct_helper {
private:
	template<typename U, typename... As>
	static std::true_type test(decltype(&U::template construct<As...>)*);
	
	template<typename...>
	static std::false_type test(...);
	
public:
	using type = decltype(test<T, Args...>(nullptr));
};

template<typename T, typename... Args>
using has_template_construct = typename has_template_construct_helper<T, Args...>::type;

template<typename T, typename... Args>
struct has_emplace_helper {
private:
	template<typename U, typename... As>
	static decltype(static_cast<void>(std::declval<U>().emplace(std::declval<As>()...)), std::true_type{}) test(int);
	
	template<typename U, typename... As>
	static std::false_type test(...);
	
public:
	using type = decltype(test<T, Args...>(0));
};

template<template<typename> class Map, typename T, typename... Args>
struct is_invokable_helper {
private:
	template<typename U, typename... As, std::size_t... S>
	static std::true_type test(seq<S...>, decltype(std::declval<U>()(std::declval<ServiceType<service_map_t<Map, function_argument_t<S, U>>>>()..., std::declval<As>()...))*);
	
	template<typename...>
	static std::false_type test(...);
	
public:
	using type = decltype(test<T, Args...>(tuple_seq_minus<function_arguments_t<T>, sizeof...(Args)>{}, nullptr));
};

template<template<typename> class Map, typename T, typename... Args>
struct defer_is_invokable_helper {
	static constexpr bool value = is_invokable_helper<Map, T, Args...>::type::value;
};

template<bool, typename T, typename... Args>
struct construct_function_helper {
private:
	template<typename U, typename... As>
	struct get_template_construct {
		using type = std::integral_constant<decltype(&U::template construct<As...>), &U::template construct<As...>>;
	};
	
	template<typename U>
	struct get_construct {
		using type = std::integral_constant<decltype(&U::construct), &U::construct>;
	};
	
	template<typename U, typename... As, enable_if_t<has_construct<U>::value, int> = 0, enable_if_t<!has_template_construct<U, As...>::value, int> = 0>
	static get_construct<U> test();
	
	template<typename U, typename... As, enable_if_t<has_template_construct<U, As...>::value, int> = 0>
	static get_template_construct<U, As...> test();
	
	using inner_type = decltype(test<T, Args...>());
	
public:
	using type = typename inner_type::type;
};

template<typename T, typename... Args>
struct construct_function_helper<false, T, Args...> {};

template<typename T, typename... Args>
using has_any_construct = std::integral_constant<bool, has_template_construct<T, Args...>::value || has_construct<T>::value>;

template<typename T, typename... Args>
using construct_function = typename construct_function_helper<has_any_construct<T, Args...>::value, T, Args...>::type;

template<typename T, typename... Args>
using construct_function_t = typename construct_function<T, Args...>::value_type;

template<typename T, typename... Args>
using construct_result_seq = tuple_seq<function_result_t<construct_function_t<T, Args...>>>;

template<typename T, typename... Args>
using has_emplace = typename has_emplace_helper<T, Args...>::type;

template<typename T, typename... Args>
using is_brace_constructible = typename is_brace_constructible_helper<T, Args...>::type;

template<typename T, typename... Args>
using is_only_brace_constructible = std::integral_constant<bool, is_brace_constructible<T, Args...>::value && !std::is_constructible<T, Args...>::value>;

template<typename T> struct remove_rvalue_reference { using type = T; };
template<typename T> struct remove_rvalue_reference<T&&> { using type = T; };

template<typename T> using remove_rvalue_reference_t = typename remove_rvalue_reference<T>::type;

template<typename T, typename... Args>
using is_someway_constructible = std::integral_constant<bool, is_brace_constructible<T, Args...>::value || std::is_constructible<T, Args...>::value>;

template<typename T, typename... Args>
using is_emplaceable = std::integral_constant<bool, std::is_default_constructible<T>::value && has_emplace<T, Args...>::value>;

template<typename T, typename... Args>
using is_service_instantiable = std::integral_constant<bool, is_emplaceable<T, Args...>::value || is_someway_constructible<T, kgr::in_place_t, Args...>::value>;

template<template<typename> class Map, typename U, typename... Args>
using is_invokable = std::integral_constant<bool, defer_is_invokable_helper<Map, U, Args...>::value>;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_TRAITS_HPP
