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
struct tuple_seq_gen;

template<typename... Types>
struct tuple_seq_gen<std::tuple<Types...>> : seq_gen<sizeof...(Types)> {};

template<typename... Types>
struct tuple_seq_gen<detail::meta_list<Types...>> : seq_gen<sizeof...(Types)> {};

template<typename Tuple>
using tuple_seq = typename tuple_seq_gen<Tuple>::type;

template<typename F>
using function_seq = tuple_seq<function_arguments_t<F>>;

template<typename>
struct seq_drop_first;

template<std::size_t... S>
struct seq_drop_first<seq<0, S...>> {
	using type = seq<S...>;
};

inline constexpr auto safe_minus(std::size_t lhs, std::size_t rhs) -> std::size_t {
	return lhs - (lhs > rhs ? rhs : lhs);
}

template<typename S>
using seq_drop_first_t = typename seq_drop_first<S>::type;

template<typename List, int n>
using tuple_seq_minus = typename detail::seq_gen<safe_minus(meta_list_size<List>::value, n)>::type;

template<typename From, typename To>
using is_explicitly_convertible = std::is_constructible<To, From>;

template<typename T>
struct is_service : bool_constant<!std::is_polymorphic<T>::value && has_forward<T>::value> {};

struct sink {
	constexpr sink() = default;
	
	template<typename T>
	operator T ();
	
	template<typename T>
	operator T&& () const;
	
	template<typename T>
	operator T& () const;
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

template<typename F, typename... Args>
struct is_callable {
private:
	template<typename...>
	static std::false_type test(...);

	template<typename U, typename... As, int_t<decltype(std::declval<U>()(std::declval<As>()...))> = 0>
	static std::true_type test(int);

	using type = decltype(test<F, Args...>(0));
	
public:
	static constexpr bool value = type::value;
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
