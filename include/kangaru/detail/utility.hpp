#ifndef KANGARU5_DETAIL_UTILITY_HPP
#define KANGARU5_DETAIL_UTILITY_HPP

#include "concepts.hpp"
#include "type_traits.hpp"

#ifndef KANGARU5_MODULES
#include <type_traits>
#include <utility>
#include <tuple>
#endif

#include "define.hpp"

namespace kangaru::detail {
	namespace utility_private {
		template<typename>
		struct make_sequence_tuple_impl;
		
		template<std::size_t... S>
		struct make_sequence_tuple_impl<std::index_sequence<S...>> {
			using type = std::tuple<std::integral_constant<std::size_t, S>...>;
		};
		
		template<unqualified_object T>
		struct function_returning_type {
			auto operator()() const -> T;
		};
		
		[[noreturn]]
		extern auto nonconsteval() -> void; // Left unimplemented
	}
	
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
	
	template<typename T = void> [[noreturn]]
	consteval inline auto noreturn() -> T { utility_private::nonconsteval(); }
	
	template<typename T>
	auto decay_copy(T&& v) -> std::decay_t<T> {
		return KANGARU5_FWD(v);
	}
	
	template<typename T> requires std::is_function_v<T>
	using function_pointer_t = T*;
	
	template<typename T, typename U>
	using forward_like_t = decltype(::kangaru::detail::forward_like<T>(std::declval<U&>()));
	
	template<typename T, std::size_t>
	using expand = T;
	
	template<std::size_t Size>
	using make_sequence_tuple = typename utility_private::make_sequence_tuple_impl<std::make_index_sequence<Size>>::type;
	
	template<typename... Ts>
	using sequence_tuple_for = typename utility_private::make_sequence_tuple_impl<std::index_sequence_for<Ts...>>::type;
	
	template<typename Tuple>
	using sequence_tuple_for_tuple = typename utility_private::make_sequence_tuple_impl<std::make_index_sequence<std::tuple_size_v<Tuple>>>::type;
	
	template<template<typename...> typename, typename>
	inline constexpr auto is_specialisation_of_v = false;
	
	template<template<typename...> typename Primary, typename... Args>
	inline constexpr auto is_specialisation_of_v<Primary, Primary<Args...>> = true;
	
	template<typename T>
	using type_identity = T;
	
	template<typename T>
	struct always_type {
		template<typename>
		using type = T;
	};
	
	template<typename T> requires false
	using never_type_identity = T;
	
	template<typename TType, typename... Ts>
	using ttype_t = typename TType::template ttype<Ts...>::type;
}

KANGARU5_EXPORT namespace kangaru {
	template<callable F> requires(unqualified_object<detail::call_result_t<F>>)
	struct in_place_construct {
		explicit constexpr in_place_construct(F function) : function(std::move(function)) {}
		
		constexpr operator detail::call_result_t<F>() && {
			return std::move(function)();
		}
		
	private:
		F function;
	};
	
	template<typename T>
	concept in_place_constructible =
		    unqualified_object<T>
		and std::constructible_from<
			T,
			in_place_construct<detail::utility_private::function_returning_type<T>>
		>;
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_UTILITY_HPP
