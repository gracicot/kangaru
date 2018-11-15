#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_TRAITS_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_TRAITS_HPP

#include "function_traits.hpp"
#include "utils.hpp"
#include "meta_list.hpp"
#include "seq.hpp"
#include "void_t.hpp"

#include <type_traits>
#include <memory>
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

template<typename F>
using function_seq = tuple_seq<function_arguments_t<F>>;

template<typename>
struct is_tuple : std::false_type {};

template<typename... Types>
struct is_tuple<std::tuple<Types...>> : std::true_type {};

template<typename T>
using remove_cvref_t = typename std::remove_cv<typename std::remove_reference<T>::type>::type;

template<typename From, typename To>
using is_explicitly_convertible = std::is_constructible<To, From>;

template<typename T>
struct is_service : bool_constant<!std::is_polymorphic<T>::value && has_forward<T>::value> {};

template<typename T>
struct constify {
	using type = T const;
};

template<typename T>
struct constify<T&&> {
	using type = T const&&;
};

template<typename T>
struct constify<T&> {
	using type = T const&;
};

template<typename T>
using constify_t = typename constify<T>::type;

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
	template<typename U, typename... As>
	static std::true_type test(int_t<decltype(std::declval<U>().emplace(std::declval<As>()...))>);
	
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

	template<typename U, typename... As>
	static std::true_type test(int_t<decltype(std::declval<U>()(std::declval<As>()...))>);

	using type = decltype(test<F, Args...>(0));
	
public:
	static constexpr bool value = type::value;
};

template<typename T, typename... Args>
using call_result_t = decltype(std::declval<T>()(std::declval<Args>()...));

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

template<typename>
struct unwrap_pointer {};

template<typename T>
struct unwrap_pointer<std::unique_ptr<T>> {
	using type = T;
};

template<typename T>
struct unwrap_pointer<std::shared_ptr<T>> {
	using type = T;
};

template<typename T>
struct unwrap_pointer<std::weak_ptr<T>> {
	using type = T;
};

template<typename T>
struct unwrap_pointer<T*> {
	using type = T;
};

template<typename T>
using unwrap_pointer_t = typename unwrap_pointer<T>::type;

template<typename T, typename... Args>
using is_someway_constructible = bool_constant<is_brace_constructible<T, Args...>::value || std::is_constructible<T, Args...>::value>;

template<typename T, typename... Args>
using is_emplaceable = bool_constant<std::is_default_constructible<T>::value && has_emplace<T, Args...>::value>;

template<typename T, typename... Args>
using is_service_instantiable = bool_constant<is_emplaceable<T, Args...>::value || is_someway_constructible<T, kgr::in_place_t, Args...>::value>;


} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_TRAITS_HPP
