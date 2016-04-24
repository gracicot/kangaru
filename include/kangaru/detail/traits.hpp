#pragma once

#include <type_traits>
#include <tuple>

namespace kgr {
namespace detail {

// void_t implementation
template<typename...>
struct voider { using type = void; };

template<typename... Ts> using void_t = typename voider<Ts...>::type;
using type_id_t = void(*)();
template <typename T> void type_id() {}

// thing missing from c++11 (to be removed when switching to c++14)
template <bool b, typename T = void>
using enable_if_t = typename std::enable_if<b, T>::type;

template<std::size_t ...>
struct seq {};

template<std::size_t n, std::size_t ...S>
struct seq_gen : seq_gen<n-1, n-1, S...> {};

template<std::size_t ...S>
struct seq_gen<0, S...> {
	using type = seq<S...>;
};

template<typename Tuple>
struct TupleSeqGen : seq_gen<std::tuple_size<Tuple>::value> {};

template<>
struct TupleSeqGen<std::tuple<>> : seq_gen<0> {};

template<typename Tuple>
using tuple_seq = typename TupleSeqGen<Tuple>::type;

// SFINAE utilities
template<typename T, typename = void>
struct has_invoke : std::false_type {};

template<typename T>
struct has_invoke<T, void_t<typename T::invoke>> : std::true_type {};

template<typename T, typename = void>
struct has_overrides : std::false_type {};

template<typename T>
struct has_overrides<T, void_t<typename T::ParentTypes>> : std::true_type {};

template<typename T, typename = void>
struct has_next : std::false_type {};

template<typename T>
struct has_next<T, void_t<typename T::Next>> : std::true_type {};

template<typename T, typename = void>
struct is_service : std::false_type {};

template<typename T>
struct is_service<T, void_t<decltype(&T::forward)>> : std::true_type {};

template<typename T, typename = void>
struct has_construct : std::false_type {};

template<typename T>
struct has_construct<T, void_t<decltype(&T::construct)>> : std::true_type {};

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
	static std::true_type test(decltype(&U::template construct<As...>)* = nullptr);

	template<typename...>
	static std::false_type test(...);
	
public:
	using type = decltype(test<T, Args...>(nullptr));
};

template<typename T, typename... Args>
struct is_brace_constructible : is_brace_constructible_helper<T, Args...>::type {};

template<typename T> struct remove_rvalue_reference { using type = T; };
template<typename T> struct remove_rvalue_reference<T&> { using type = T&; };
template<typename T> struct remove_rvalue_reference<T&&> { using type = T; };

template<typename T> using remove_rvalue_reference_t = typename remove_rvalue_reference<T>::type;

template<typename... Ts>
struct has_template_construct : has_template_construct_helper<Ts...>::type {};

template<typename... Ts>
using is_someway_constructible = std::integral_constant<bool, is_brace_constructible<Ts...>::value || std::is_constructible<Ts...>::value>;

} // namespace detail
} // namespace kgr
