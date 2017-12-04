#ifndef KGR_KANGARU_INCLUDE_KANGARU_DETAIL_FUNCTION_TRAITS_HPP
#define KGR_KANGARU_INCLUDE_KANGARU_DETAIL_FUNCTION_TRAITS_HPP

#include "meta_list.hpp"
#include "void_t.hpp"

#include <type_traits>

namespace kgr {
namespace detail {

template<typename, typename = void>
struct has_call_operator : std::false_type {};

template<typename T>
struct has_call_operator<T, void_t<decltype(&T::operator())>> : std::true_type {};

template <typename>
struct function_traits_helper {};

template <typename Type, typename R, typename... Args>
struct base_function_traits {
	using object_type = Type;
	using return_type = R;
	using argument_types = meta_list<Args...>;
	template<std::size_t n> using argument_type = meta_list_element_t<n, argument_types>;
};

template <typename R, typename... Args>
struct base_non_member_function_traits {
	using return_type = R;
	using argument_types = meta_list<Args...>;
	template<std::size_t n> using argument_type = meta_list_element_t<n, argument_types>;
};

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

template<typename F, typename = void>
struct function_traits : function_traits_helper<F> {};

template<typename T>
struct function_traits<T, typename std::enable_if<has_call_operator<T>::value>::type> : function_traits_helper<decltype(&T::operator())> {};

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

template <typename F>
using function_arguments_t = typename function_traits<F>::argument_types;

template <typename F>
using function_result_t = typename function_traits<F>::return_type;

template <typename F>
using object_type_t = typename function_traits<F>::object_type;

template <std::size_t n, typename F>
using function_argument_t = typename function_traits<F>::template argument_type<n>;

} // namespace detail
} // namespace kgr

#endif // KGR_KANGARU_INCLUDE_KANGARU_DETAIL_FUNCTION_TRAITS_HPP
