#pragma once

#include <tuple>

namespace kgr {
namespace detail {

template <typename T>
struct function_traits
	: function_traits<decltype(&T::operator())>
{};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...) const> {
	using return_type = R;
	using argument_types = std::tuple<Args...>;
	template<int n> using argument_type = typename std::tuple_element<n, argument_types>::type;
};

template <typename Type, typename R, typename... Args>
struct function_traits<R(Type::*)(Args...)> {
	using return_type = R;
	using argument_types = std::tuple<Args...>;
	template<int n> using argument_type = typename std::tuple_element<n, argument_types>::type;
};

template <typename R, typename... Args>
struct function_traits<R(*)(Args...)> {
	using return_type = R;
	using argument_types = std::tuple<Args...>;
	template<int n> using argument_type = typename std::tuple_element<n, argument_types>::type;
};

template <typename... F>
using function_arguments_t = typename function_traits<F...>::argument_types;

template <typename... F>
using function_result_t = typename function_traits<F...>::return_type;

template <int n, typename... F>
using function_argument_t = typename function_traits<F...>::template argument_type<n>;

} // namespace detail
} // namespace kgr
