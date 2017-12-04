#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_FUNCTION_TRAITS_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_FUNCTION_TRAITS_HPP

#include "meta_list.hpp"
#include "void_t.hpp"

#include <type_traits>

namespace kgr {
namespace detail {

/*
 * This trait determines if a class has a not overloaded operator() defined in it.
 * Userful to get the signature of a lambda.
 */
template<typename, typename = void>
struct has_call_operator : std::false_type {};

template<typename T>
struct has_call_operator<T, void_t<decltype(&T::operator())>> : std::true_type {};

/*
 * This trait extracts the signature of a function.
 * If it's not a function, then this overload is choosen.
 */
template <typename>
struct function_traits_helper {};

/*
 * This trait extract a member function signature.
 * We use this so we can get a list of parameter the function need, what is the object type, and so on.
 */
template <typename Type, typename R, typename... Args>
struct base_function_traits {
	using object_type = Type;
	using return_type = R;
	using argument_types = meta_list<Args...>;
	template<std::size_t n> using argument_type = meta_list_element_t<n, argument_types>;
};

/*
 * This trait extract the signature of a non-member function.
 * Useful to invoke free function and static functions.
 */
template <typename R, typename... Args>
struct base_non_member_function_traits {
	using return_type = R;
	using argument_types = meta_list<Args...>;
	template<std::size_t n> using argument_type = meta_list_element_t<n, argument_types>;
};

/*
 * We must extends base_function_traits for each possible cases of function signature type.
 */
template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args...) const> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args...)> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args...) const &&> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args...) &&> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args...) const &> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args...) &> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args...) const volatile> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args...) volatile> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args...) const volatile &> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args...) volatile &> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args...) const volatile &&> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args...) volatile &&> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args..., ...) const> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args..., ...)> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args..., ...) const &&> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args..., ...) &&> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args..., ...) const &> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args..., ...) &> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args..., ...) const volatile> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args..., ...) volatile> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args..., ...) const volatile &> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args..., ...) volatile &> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args..., ...) const volatile &&> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits_helper<R(Type::*)(Args..., ...) volatile &&> : base_function_traits<Type, R, Args...> {};

template <typename R, typename... Args>
struct function_traits_helper<R(*)(Args...)> : base_non_member_function_traits<R, Args...> {};

template <typename R, typename... Args>
struct function_traits_helper<R(*)(Args..., ...)> : base_non_member_function_traits<R, Args...> {};

#if defined(__cpp_noexcept_function_type) || __cplusplus >= 201703L

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) const noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) const && noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) && noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) const & noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) & noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) const volatile noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) volatile noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) const volatile & noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) volatile & noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) const volatile && noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) volatile && noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) const noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) const && noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) && noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) const & noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) & noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) const volatile noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) volatile noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) const volatile & noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) volatile & noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) const volatile && noexcept> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) volatile && noexcept> : base_function_traits<Type, R, Args...> {};

template <typename R, typename... Args>
struct function_traits<R(*)(Args...) noexcept> : base_non_member_function_traits<R, Args...> {};

template <typename R, typename... Args>
struct function_traits<R(*)(Args..., ...) noexcept> : base_non_member_function_traits<R, Args...> {};

#endif

/*
 * For any type, extends function_traits_helper to extract signature of the function.
 */
template<typename F, typename = void>
struct function_traits : function_traits_helper<F> {};

/*
 * If we send a lambda or a functor, we can extract the operator() signature.
 */
template<typename T>
struct function_traits<T, typename std::enable_if<has_call_operator<T>::value>::type> : function_traits_helper<decltype(&T::operator())> {};

/*
 * Alias for the type of the argument list.
 */
template <typename F>
using function_arguments_t = typename function_traits<F>::argument_types;

/*
 * Alias for the function result.
 */
template <typename F>
using function_result_t = typename function_traits<F>::return_type;

/*
 * Alias for the class type the function is member to.
 */
template <typename F>
using object_type_t = typename function_traits<F>::object_type;

/*
 * Alias for the type of the nth argument.
 */
template <std::size_t n, typename F>
using function_argument_t = typename function_traits<F>::template argument_type<n>;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_FUNCTION_TRAITS_HPP
