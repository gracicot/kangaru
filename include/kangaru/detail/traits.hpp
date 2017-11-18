#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_TRAITS_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_TRAITS_HPP

#include "function_traits.hpp"
#include "utils.hpp"
#include "meta_list.hpp"
#include "void_t.hpp"

#include <type_traits>
#include <tuple>

namespace kgr {
namespace detail {

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

// Workaround for visual studio to take the address of a generic lambda
template<typename T>
T exact(T);

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

template<bool b, typename T, typename F>
using conditional_t = typename std::conditional<b, T, F>::type;

template<std::size_t S, typename T>
using tuple_element_t = typename std::tuple_element<S, T>::type;

template<std::size_t size, std::size_t align>
using aligned_storage_t = typename std::aligned_storage<size, align>::type;

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

template<typename F>
using function_seq = tuple_seq<function_arguments_t<F>>;

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
struct is_service : std::false_type {};

template<typename T>
struct is_service<T, enable_if_t<!std::is_polymorphic<T>::value && has_forward<T>::value>> : std::true_type {};

// Here, usual traits using void_t don't quite work with visual studio for this particular case.
template<typename T>
struct has_construct_helper {
private:
	template<typename U, typename V = decltype(&U::construct)>
	static std::true_type test(int);

	template<typename>
	static std::false_type test(...);

public:
	using type = decltype(test<T>(0));
};

template<typename T>
using has_construct = typename has_construct_helper<T>::type;

template<typename T, typename = void>
struct is_invoke_call : std::false_type {};

template<typename T>
struct is_invoke_call<T, void_t<typename T::Parameters>> : std::true_type {};

template<typename T, typename F>
using is_member_autocall = std::integral_constant<bool, !is_invoke_call<F>::value>;

struct Sink {
	constexpr Sink() = default;
	
	template<typename T>
	constexpr operator T&& () const;
	
	template<typename T>
	constexpr operator const T& () const;
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

// We implement has_emplace with an old fashioned trait, because GCC don't handle friendship in specialized arguments.
template<typename T, typename... Args>
struct has_emplace_helper {
private:
	template<typename U, typename... As, int_t<decltype(std::declval<U>().emplace(std::declval<As>()...))> = 0>
	static std::true_type test(int);
	
	template<typename U, typename... As>
	static std::false_type test(...);
	
public:
	using type = decltype(test<T, Args...>(0));
};

template<typename T>
using is_service_embeddable = std::integral_constant<bool,
	std::is_trivially_destructible<T>::value &&
	sizeof(T) <= sizeof(void*) && alignof(T) <= alignof(void*)
>;

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

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_TRAITS_HPP
