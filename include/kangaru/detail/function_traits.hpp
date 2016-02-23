#pragma once

#include <tuple>

#include "traits.hpp"

namespace kgr {
namespace detail {
	
template <typename T, typename = void>
struct function_traits {};

template <typename T>
struct function_traits<T, void_t<decltype(&T::operator())>>
	: function_traits<decltype(&T::operator())>
{};

template <typename Type, typename R, typename... Args>
struct base_function_traits {
	using object_type = Type;
	using return_type = R;
	using argument_types = std::tuple<Args...>;
	template<int n> using argument_type = typename std::tuple_element<n, argument_types>::type;
};

template <typename R, typename... Args>
struct base_non_member_function_traits {
	using return_type = R;
	using argument_types = std::tuple<Args...>;
	template<int n> using argument_type = typename std::tuple_element<n, argument_types>::type;
};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) const> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...)> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) const &&> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) &&> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) const &> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) &> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) const volatile> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) volatile> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) const volatile &> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) volatile &> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) const volatile &&> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) volatile &&> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) const> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...)> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) const &&> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) &&> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) const &> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) &> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) const volatile> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) volatile> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) const volatile &> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) volatile &> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) const volatile &&> : base_function_traits<Type, R, Args...> {};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args..., ...) volatile &&> : base_function_traits<Type, R, Args...> {};

template <typename R, typename... Args>
struct function_traits<R(*)(Args...)> : base_non_member_function_traits<R, Args...> {};

template <typename R, typename... Args>
struct function_traits<R(*)(Args..., ...)> : base_non_member_function_traits<R, Args...> {};

template <typename F>
using function_arguments_t = typename function_traits<F>::argument_types;

template <typename F>
using function_result_t = typename function_traits<F>::return_type;

template <typename F>
using object_type_t = typename function_traits<F>::object_type;

template <int n, typename F>
using function_argument_t = typename function_traits<F>::template argument_type<n>;

} // namespace detail
} // namespace kgr
