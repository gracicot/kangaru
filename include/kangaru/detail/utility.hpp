#ifndef KANGARU5_DETAIL_UTILITY_HPP
#define KANGARU5_DETAIL_UTILITY_HPP

#include <type_traits>
#include <utility>
#include <tuple>

#include "define.hpp"

namespace kangaru::detail::utility {
	template<typename T, typename U> [[nodiscard]]
	constexpr auto forward_like(U&& x) noexcept -> auto&& {
		constexpr bool is_const = std::is_const_v<std::remove_reference_t<T>>;
		if constexpr (std::is_lvalue_reference_v<T&&>) {
			if constexpr (is_const) {
				return std::as_const(x);
			} else {
				return static_cast<U&>(x);
			}
		} else {
			if constexpr (is_const) {
				return std::move(std::as_const(x));
			} else {
				return std::move(x);
			}
		}
	}
	
	template<typename T>
	auto decay_copy(T&& v) -> std::decay_t<T> {
		return KANGARU5_FWD(v);
	}
	
	template<typename T> requires std::is_function_v<T>
	using function_pointer_t = std::add_pointer_t<T>;
	
	template<typename T, typename U>
	using forward_like_t = decltype(forward_like<T>(std::declval<U&>()));
	
	template<typename T, std::size_t>
	using expand = T;
	
	template<typename... Functions>
	struct overload : Functions... {
		using Functions::operator()...;
	};
	
	template<typename... Functions>
	overload(Functions...) -> overload<Functions...>;
	
	template<typename>
	struct make_sequence_tuple_impl;
	
	template<std::size_t... S>
	struct make_sequence_tuple_impl<std::index_sequence<S...>> {
		using type = std::tuple<std::integral_constant<std::size_t, S>...>;
	};
	
	template<std::size_t Size>
	using make_sequence_tuple = typename make_sequence_tuple_impl<std::make_index_sequence<Size>>::type;
	
	template<typename... Ts>
	using sequence_tuple_for = typename make_sequence_tuple_impl<std::index_sequence_for<Ts...>>::type;
	
	template<typename Tuple>
	using sequence_tuple_for_tuple = typename make_sequence_tuple_impl<std::make_index_sequence<std::tuple_size_v<Tuple>>>::type;
	
	template<template<typename...> typename Template>
	struct template_type_identity {
		template<typename... Args>
		struct ttype {
			using type = Template<Args...>;
		};
	};
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_UTILITY_HPP
