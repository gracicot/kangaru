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

template<int ...>
struct seq {};

template<int n, int ...S>
struct seq_gen : seq_gen<n-1, n-1, S...> {};

template<int ...S>
struct seq_gen<0, S...> {
	using type = seq<S...>;
};

template<typename Tuple>
using tuple_seq = typename seq_gen<std::tuple_size<Tuple>::value>::type;

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

template<typename T, typename... Args>
struct is_brace_constructible_helper {
private:
	template<typename U, typename... As>
	static decltype(void(U{std::declval<As>()...}), std::true_type{}) test(int);

	template<typename...>
	static std::false_type test(...);
	
public:
	using type = decltype(test<T, Args...>(0));
};

template<typename T, typename... Args>
struct is_brace_constructible : is_brace_constructible_helper<T, Args...>::type {};

} // namespace detail
} // namespace kgr
