#ifndef KANGARU5_DETAIL_CONSTRUCTOR_HPP
#define KANGARU5_DETAIL_CONSTRUCTOR_HPP

#include "concepts.hpp"
#include "type_traits.hpp"
#include "utility.hpp"
#include "deducer.hpp"

#ifndef KANGARU5_MODULES
#include <concepts>
#include <type_traits>
#endif

#include "define.hpp"

// TODO: Remove when visual studio feedback item 11026651 is fixed
namespace kangaru::detail::constructor_private {
	template<typename T, typename... Args>
	concept brace_constructible = requires(Args&&... args) {
		T{KANGARU5_FWD(args)...};
	};
	
	template<typename T, typename... Args>
	concept raw_constructor_callable =
		   std::constructible_from<T, Args...>
		#if KANGARU5_AMBIGUOUS_BASED_PRVALUE_DETECTION() == 0
		or brace_constructible<T, Args...>
		#endif
		;
	
	template<unqualified_object Type>
	struct raw_constructor_function {
		constexpr auto operator()(auto&&... args) const& -> Type requires(
			raw_constructor_callable<Type, decltype(args)...>
		) {
		#if KANGARU5_AMBIGUOUS_BASED_PRVALUE_DETECTION() == 1
			return Type(KANGARU5_FWD(args)...);
		#else
			if constexpr (std::constructible_from<Type, decltype(args)...>) {
				return Type(KANGARU5_FWD(args)...);
			} else {
				return Type{KANGARU5_FWD(args)...};
			}
		#endif
		}
	};
	
	template<unqualified_object Type>
	inline constexpr auto raw_constructor(auto&&... args) -> Type requires(
		callable<raw_constructor_function<Type>, decltype(args)...>
	) {
		return raw_constructor_function<Type>{}(KANGARU5_FWD(args)...);
	}
}

KANGARU5_EXPORT namespace kangaru {
	template<unqualified_object Type>
	struct constructor_function {
		template<typename From = Type>
			requires (not deducer<std::remove_cvref_t<From>> and std::convertible_to<From&&, Type>)
		constexpr auto operator()(From&& object) const -> Type {
			// Here we don't use construct function since we always only want a copy/move or a conversion.
			return Type(KANGARU5_FWD(object));
		}
		
		template<deducer Deducer1, typename... Args>
			requires detail::constructor_private::raw_constructor_callable<
				Type,
				exclude_special_constructors_deducer<Type, Deducer1>,
				Args...
			>
		constexpr auto operator()(Deducer1 deduce1, Args&&... args) const -> Type {
			return detail::constructor_private::raw_constructor<Type>(
				KANGARU5_NO_ADL(exclude_special_constructors_for<Type>)(deduce1),
				KANGARU5_FWD(args)...
			);
		}
		
		template<typename FirstArg, typename... Args>
			requires(
				    not deducer<std::remove_cvref_t<FirstArg>>
				and detail::constructor_private::raw_constructor_callable<
					Type,
					FirstArg,
					Args...
				>
			)
		constexpr auto operator()(FirstArg&& first, Args&&... args) const -> Type {
			return detail::constructor_private::raw_constructor<Type>(KANGARU5_FWD(first), KANGARU5_FWD(args)...);
		}
		
		constexpr auto operator()() const -> Type requires std::default_initializable<Type> {
			return detail::constructor_private::raw_constructor<Type>();
		}
	};
	
	template<typename Type, typename... Args>
	concept constructor_callable = unqualified_object<Type> and callable<constructor_function<Type>, Args...>;
	
	template<unqualified_object Type>
	inline constexpr auto constructor(auto&&... args) -> Type requires(
		constructor_callable<Type, decltype(args)...>
	) {
		return constructor_function<Type>{}(KANGARU5_FWD(args)...);
	}
	
	template<unqualified_object Type>
	struct non_default_constructor_function {
		constexpr auto operator()(auto&& first, auto&&... args) -> Type requires(
			constructor_callable<Type, decltype(first), decltype(args)...>
		) {
			return KANGARU5_NO_ADL(constructor<Type>)(KANGARU5_FWD(first), KANGARU5_FWD(args)...);
		}
	};
	
	template<typename Type, typename... Args>
	concept non_default_constructor_callable = unqualified_object<Type> and callable<non_default_constructor_function<Type>, Args...>;
	
	template<unqualified_object Type>
	inline constexpr auto non_default_constructor(auto&&... args) -> Type requires(
		non_default_constructor_callable<Type, decltype(args)...>
	) {
		return non_default_constructor_function<Type>{}(KANGARU5_FWD(args)...);
	}
	
	template<in_place_constructible Type>
	inline constexpr auto construct_in_place(auto&&... args) requires(
		constructor_callable<Type, decltype(args)...>
	) {
		return in_place_construct{[&] {
			return KANGARU5_NO_ADL(constructor<Type>)(KANGARU5_FWD(args)...);
		}};
	}
	
	template<template<typename...> typename Type, typename... Args>
		requires(
			    in_place_constructible<decltype(Type(std::declval<Args>()...))>
			and constructor_callable<decltype(Type(std::declval<Args>()...)), Args&&...>
		)
	inline constexpr auto construct_in_place(Args&&... args) {
		return in_place_construct{[&] {
			return KANGARU5_NO_ADL(constructor<decltype(Type(KANGARU5_FWD(args)...))>)(KANGARU5_FWD(args)...);
		}};
	}
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CONSTRUCTOR_HPP
