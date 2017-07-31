#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INVOKE_FUNCTION_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INVOKE_FUNCTION_HPP

#include "meta_list.hpp"
#include "void_t.hpp"
#include "traits.hpp"
#include "service_map.hpp"

#if _MSC_VER == 1900
#ifndef __clang__
#define KGR_KANGARU_MSVC_NO_DEPENDENT_TEMPLATE_KEYWORD
#endif // !__clang__
#endif // _MSC_VER

namespace kgr {
namespace detail {

template<typename Map, typename T, typename P, typename... Args>
struct is_pointer_invokable_helper {
private:
	template<typename U, typename V, typename... As, std::size_t... S, int_t<decltype(
		(std::declval<U>().*std::declval<V>())(
			std::declval<ServiceType<service_map_t<Map, function_argument_t<S, V>>>>()...,
			std::declval<As>()...
		)
	)> = 0>
	static std::true_type test(seq<S...>);
	
	template<typename...>
	static std::false_type test(...);
	
public:
	using type = decltype(test<T, P, Args...>(tuple_seq_minus<function_arguments_t<P>, sizeof...(Args)>{}));
};

template<typename Map, typename T, typename P, typename... Args>
struct is_pointer_invokable : is_pointer_invokable_helper<Map, T, P, Args...>::type {};

template<typename, typename, typename, typename, typename = void>
struct has_callable_template_call : std::false_type {};

template<typename Map, typename T, typename... TArgs, typename... Args>
struct has_callable_template_call<
	Map, T, meta_list<TArgs...>, meta_list<Args...>,
	enable_if_t<is_pointer_invokable<Map, T,

#ifdef KGR_KANGARU_MSVC_NO_DEPENDENT_TEMPLATE_KEYWORD
	decltype(exact(&T::operator()<TArgs...>)),
#else
	decltype(exact(&T::template operator()<TArgs...>)),
#endif // KGR_KANGARU_MSVC_NO_DEPENDENT_TEMPLATE_KEYWORD
	
	Args...>::value>
> : std::true_type {};

template<typename, typename, typename, typename, typename = void>
struct get_template_call_helper;

template<typename Map, typename T, typename... Args>
struct get_template_call_helper<
	Map, T, meta_list<>, meta_list<Args...>,
	enable_if_t<!has_callable_template_call<Map, T, meta_list<>, meta_list<Args...>>::value>
> {};

template<typename Map, typename T, typename Head, typename... Tail, typename... Args>
struct get_template_call_helper<
	Map, T, meta_list<Head, Tail...>, meta_list<Args...>,
	enable_if_t<!has_callable_template_call<Map, T, meta_list<Head, Tail...>, meta_list<Args...>>::value>
> : get_template_call_helper<Map, T, meta_list<Tail...>, meta_list<Args...>> {};

template<typename Map, typename T, typename... TArgs, typename... Args>
struct get_template_call_helper<
	Map, T, meta_list<TArgs...>, meta_list<Args...>,
	enable_if_t<has_callable_template_call<Map, T, meta_list<TArgs...>, meta_list<Args...>>::value>
> {
#ifdef KGR_KANGARU_MSVC_NO_DEPENDENT_TEMPLATE_KEYWORD
	using type = decltype(exact(&T::operator()<TArgs...>));
#else
	using type = decltype(exact(&T::template operator()<TArgs...>));
#endif // KGR_KANGARU_MSVC_NO_DEPENDENT_TEMPLATE_KEYWORD
};

template<typename Map, typename T, typename... Args>
using get_template_call = get_template_call_helper<Map, T, meta_list<Args...>, meta_list<Args...>>;

template<typename, typename, typename, typename = void>
struct has_template_call_operator : std::false_type {};

template<typename Map, typename T, typename... Args>
struct has_template_call_operator<Map, T, meta_list<Args...>, void_t<typename get_template_call<Map, T, Args...>::type>> : std::true_type {};

template<typename, typename, typename, typename = void>
struct invoke_function_helper {};

template<typename Map, typename T, typename... Args>
struct invoke_function_helper<
	Map, T, meta_list<Args...>,
	enable_if_t<has_call_operator<T>::value && !has_template_call_operator<Map, T, meta_list<Args...>>::value>
> {
	using type = decltype(&T::operator());
	using return_type = function_result_t<type>;
	using argument_types = function_arguments_t<type>;
	template<std::size_t n> using argument_type = meta_list_element_t<n, argument_types>;
};

template<typename Map, typename T, typename... Args>
struct invoke_function_helper<
	Map, T, meta_list<Args...>,
	void_t<
		function_result_t<T>,
		enable_if_t<!has_call_operator<T>::value && !has_template_call_operator<Map, T, meta_list<Args...>>::value>
	>
> {
	using type = T;
	using return_type = function_result_t<type>;
	using argument_types = function_arguments_t<type>;
	template<std::size_t n> using argument_type = meta_list_element_t<n, argument_types>;
};

template<typename Map, typename T, typename... Args>
struct invoke_function_helper<
	Map, T, meta_list<Args...>,
	enable_if_t<!has_call_operator<T>::value && has_template_call_operator<Map, T, meta_list<Args...>>::value>
> {
	using type = typename get_template_call<Map, T, Args...>::type;
	using return_type = function_result_t<type>;
	using argument_types = function_arguments_t<type>;
	template<std::size_t n> using argument_type = meta_list_element_t<n, argument_types>;
};

template<typename Map, typename T, typename... Args>
struct invoke_function : invoke_function_helper<Map, T, meta_list<Args...>> {};

template<typename Map, typename T, typename... Args>
using invoke_function_t = typename invoke_function<Map, T, Args...>::type;

template<typename Map, typename T, typename... Args>
using invoke_function_arguments_t = typename invoke_function<Map, T, Args...>::argument_types;

template<std::size_t n, typename Map, typename T, typename... Args>
using invoke_function_argument_t = typename invoke_function<Map, T, Args...>::template argument_type<n>;

template<typename Map, typename T, typename... Args>
using invoke_function_result_t = typename invoke_function<Map, T, Args...>::return_type;

template<typename Map, typename T, typename... Args>
struct is_invokable_helper {
private:
	template<std::size_t I, typename U>
	struct expand {
		using type = invoke_function_argument_t<I, Map, U, Args...>;
	};

	// This sub trait is for visual studio
	// The constraint can be simplified because every argument are simple template argument
	template<typename U, typename... As>
	struct call_test {
	private:
		template<typename...>
		static std::false_type test(...);

		template<typename V, typename... A2s, int_t<decltype(std::declval<V>()(std::declval<A2s>()...))> = 0>
		static std::true_type test(int);

		using type = decltype(test<U, As...>(0));
		
	public:
		static constexpr bool value = type::value;
	};
	
	template<typename U>
	using map_t = service_map_t<Map, U>;

	template<typename U, typename... As, std::size_t... S, enable_if_t<call_test<U, ServiceType<map_t<typename expand<S, U>::type>>..., As...>::value, int> = 0>
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
struct is_invokable : is_invokable_helper<Map, T, Args...>::type {};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INVOKE_FUNCTION_HPP
