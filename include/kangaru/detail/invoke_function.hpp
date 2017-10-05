#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INVOKE_FUNCTION_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INVOKE_FUNCTION_HPP

#include "meta_list.hpp"
#include "void_t.hpp"
#include "traits.hpp"
#include "service_map.hpp"

#if _MSC_VER == 1900
#ifndef __clang__
// MSVC has a defect that makes the use of the template keyword an error in some corner cases.
#define KGR_KANGARU_MSVC_NO_DEPENDENT_TEMPLATE_KEYWORD
#endif // !__clang__
#endif // _MSC_VER

#if _MSC_VER
#ifndef __clang__
// MSVC has a defect that makes decltype with the address of a
// generic lambda not possible unless sending the address to a function.
#define KGR_KANGARU_MSVC_EXACT_DECLTYPE
#endif // !__clang__
#endif // _MSC_VER

namespace kgr {
namespace detail {

/*
 * Checks if some given pointer to member function is invokable
 * 
 * Assumes that P is a reflectable function type.
 */
template<typename Map, typename T, typename P, typename... Args>
struct is_pointer_invokable_helper {
private:
	template<typename U, typename V, typename... As, std::size_t... S, int_t<decltype(
		(std::declval<U>().*std::declval<V>())(
			std::declval<ServiceType<service_map_t<function_argument_t<S, V>, Map>>>()...,
			std::declval<As>()...
		)
	)> = 0>
	static std::true_type test(seq<S...>);
	
	template<typename...>
	static std::false_type test(...);
	
public:
	using type = decltype(test<T, P, Args...>(tuple_seq_minus<function_arguments_t<P>, sizeof...(Args)>{}));
};

/*
 * Alias for the is_pointer_invokable_helper trait
 */
template<typename Map, typename T, typename P, typename... Args>
struct is_pointer_invokable : is_pointer_invokable_helper<Map, T, P, Args...>::type {};

/*
 * Trait that checks if a class T has a call operator callable using given service map
 */
template<typename, typename, typename, typename, typename = void>
struct is_template_call_callable : std::false_type {};

/*
 * Specialization of is_template_call_callable when the call operator is callable
 */
template<typename Map, typename T, typename... TArgs, typename... Args>
struct is_template_call_callable<
	Map, T, meta_list<TArgs...>, meta_list<Args...>,
	enable_if_t<is_pointer_invokable<Map, T,
	
	// Some MSVC vesions cannot parse the valid syntax corretly. We must write
	// the expression in that way in order to compile.
#ifdef KGR_KANGARU_MSVC_NO_DEPENDENT_TEMPLATE_KEYWORD
	decltype(exact(&T::operator()<TArgs...>)),
#elif defined(KGR_KANGARU_MSVC_EXACT_DECLTYPE)
	// GCC won't accept taking the address of a generic lambda if the address is sent to a function like exact.
	decltype(exact(&T::template operator()<TArgs...>)),
#else
	// Vanilla syntax
	decltype(&T::template operator()<TArgs...>),
#endif // KGR_KANGARU_MSVC_NO_DEPENDENT_TEMPLATE_KEYWORD
	
	Args...>::value>
> : std::true_type {};

/*
 * Trait that returns the type of the first matching callable template call operator
 */
template<typename, typename, typename, typename, typename = void>
struct get_template_call;

/*
 * Specialization of get_template_call_helper when there is no functoin found
 */
template<typename Map, typename T, typename... Args>
struct get_template_call<
	Map, T, meta_list<>, meta_list<Args...>,
	enable_if_t<!is_template_call_callable<Map, T, meta_list<>, meta_list<Args...>>::value>
> {};

/*
 * Specialization of get_template_call_helper when current template arguments does
 * not result in a callable template call operator.
 * 
 * Will call recursively the next iteration of the trait.
 */
template<typename Map, typename T, typename Head, typename... Tail, typename... Args>
struct get_template_call<
	Map, T, meta_list<Head, Tail...>, meta_list<Args...>,
	enable_if_t<!is_template_call_callable<Map, T, meta_list<Head, Tail...>, meta_list<Args...>>::value>
> : get_template_call<Map, T, meta_list<Tail...>, meta_list<Args...>> {};

/*
 * Specialization of get_template_call_helper when a callable candidate is found.
 */
template<typename Map, typename T, typename... TArgs, typename... Args>
struct get_template_call<
	Map, T, meta_list<TArgs...>, meta_list<Args...>,
	enable_if_t<is_template_call_callable<Map, T, meta_list<TArgs...>, meta_list<Args...>>::value>
> {
	// Some MSVC vesions cannoet parse the valid syntax corretly. We must write
	// the expression in that way in order to compile.
#ifdef KGR_KANGARU_MSVC_NO_DEPENDENT_TEMPLATE_KEYWORD
	using type = decltype(exact(&T::operator()<TArgs...>));
#elif defined(KGR_KANGARU_MSVC_EXACT_DECLTYPE)
	// GCC won't accept taking the address of a generic lambda if the address is sent to a function like exact.
	using type = decltype(exact(&T::template operator()<TArgs...>));
#else
	// Vanilla syntax
	using type = decltype(&T::template operator()<TArgs...>);
#endif
};

/*
 * Alias for get_template_call_helper::type
 */
template<typename Map, typename T, typename... Args>
using get_template_call_t = typename get_template_call<Map, T, meta_list<Args...>, meta_list<Args...>>::type;

/*
 * Trait that tells if the class T has a callable template call operator
 */
template<typename, typename, typename, typename = void>
struct has_template_call_operator : std::false_type {};

template<typename Map, typename T, typename... Args>
struct has_template_call_operator<Map, T, meta_list<Args...>, void_t<get_template_call_t<Map, T, Args...>>> : std::true_type {};

/*
 * function_trait equivalent for an invoke function. It has to choose if it's a lambda, generic lambda or a function.
 */
template<typename, typename, typename, typename = void>
struct invoke_function_helper {};

/*
 * Specialization of invoke_function_helper for function types and ordinary lambda.
 * Will extract correctly argument types and return type.
 */
template<typename Map, typename T, typename... Args>
struct invoke_function_helper<
	Map, T, meta_list<Args...>,
	enable_if_t<has_call_operator<T>::value || std::is_pointer<T>::value>
> : function_traits<T> {};

/*
 * Specialization of invoke_function_helper for generic lambda.
 * Will extract correctly argument types and return type.
 */
template<typename Map, typename T, typename... Args>
struct invoke_function_helper<
	Map, T, meta_list<Args...>,
	enable_if_t<!has_call_operator<T>::value && !std::is_pointer<T>::value && has_template_call_operator<Map, T, meta_list<Args...>>::value>
> : function_traits<get_template_call_t<Map, T, Args...>> {};

/*
 * Alias for invoke_function_helper
 */
template<typename Map, typename T, typename... Args>
struct invoke_function : invoke_function_helper<Map, T, meta_list<Args...>> {};

/*
 * Alias for invoke_function::argument_types, a meta list of argument types.
 */
template<typename Map, typename T, typename... Args>
using invoke_function_arguments_t = typename invoke_function<Map, T, Args...>::argument_types;

/*
 * Alias for invoke_function::argument_type, the type of the nth argument.
 */
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
	
	// Alias for not using a template template argument in the next expression, will help simplify the expression and MSVC to parse it.
	template<typename U>
	using map_t = service_map_t<U, Map>;
	
	// We forward injected argument types (from expand) and additional arguments (As) to the sub trait call_test
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

/*
 * Alias for is_invokable_helper
 * Using inheritance here helped msvc in some cases.
 */
template<typename Map, typename T, typename... Args>
struct is_invokable : is_invokable_helper<Map, T, Args...>::type {};

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_INVOKE_FUNCTION_HPP
