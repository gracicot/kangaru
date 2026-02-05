#ifndef KANGARU5_DETAIL_CONSTRUCTOR_HPP
#define KANGARU5_DETAIL_CONSTRUCTOR_HPP

#include "concepts.hpp"
#include "kangaru/detail/type_traits.hpp"
#include "utility.hpp"
#include "deducer.hpp"

#ifndef KANGARU5_MODULES
#include <concepts>
#include <type_traits>
#endif

#include "define.hpp"

KANGARU5_EXPORT namespace kangaru {
	template<typename T, typename... Args>
	concept raw_constructor_callable =
		   std::constructible_from<T, Args...>
		or (
			brace_constructible<T, Args...>
			// TODO: Remove this preprocessor conditional when visual studio feedback item 11026651 is fixed
			#if KANGARU5_AMBIGUOUS_BASED_PRVALUE_DETECTION()
				and (
					   not std::is_aggregate_v<T>
					or sizeof...(Args) != 1
				)
			#endif
		);
	
	template<unqualified_object Type>
	struct raw_constructor_function {
		constexpr auto operator()(auto&&... args) const& -> Type requires(
			raw_constructor_callable<Type, decltype(args)...>
		) {
			if constexpr (std::constructible_from<Type, decltype(args)...>) {
				return Type(KANGARU5_FWD(args)...);
			} else {
				return Type{KANGARU5_FWD(args)...};
			}
		}
	};
	
	template<unqualified_object Type>
	inline constexpr auto raw_constructor(auto&&... args) -> Type requires(
		callable<raw_constructor_function<Type>, decltype(args)...>
	) {
		return raw_constructor_function<Type>{}(KANGARU5_FWD(args)...);
	}
	
	template<unqualified_object Type>
	struct constructor_function {
		template<typename From = Type>
			requires (not deducer<std::remove_cvref_t<From>> and std::convertible_to<From&&, Type>)
		constexpr auto operator()(From&& object) const -> Type {
			// Here we don't use construct function since we always only want a copy/move or a conversion.
			return Type(KANGARU5_FWD(object));
		}
		
		template<deducer Deducer1, typename... Args>
			requires raw_constructor_callable<
				Type,
				exclude_special_constructors_deducer<Type, Deducer1>,
				Args...
			>
		constexpr auto operator()(Deducer1 deduce1, Args&&... args) const -> Type {
			return KANGARU5_NO_ADL(raw_constructor<Type>)(
				KANGARU5_NO_ADL(exclude_special_constructors_for<Type>)(deduce1),
				KANGARU5_FWD(args)...
			);
		}
		
		template<typename FirstArg, typename... Args>
			requires(
				    not deducer<std::remove_cvref_t<FirstArg>>
				and raw_constructor_callable<
					Type,
					FirstArg,
					Args...
				>
			)
		constexpr auto operator()(FirstArg&& first, Args&&... args) const -> Type {
			return KANGARU5_NO_ADL(raw_constructor<Type>)(KANGARU5_FWD(first), KANGARU5_FWD(args)...);
		}
		
		constexpr auto operator()() const requires std::default_initializable<Type> {
			return KANGARU5_NO_ADL(raw_constructor<Type>)();
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
	
	template<callable F>
	struct in_place_construct {
		explicit constexpr in_place_construct(F function) : function(std::move(function)) {}
		
		constexpr operator detail::type_traits::call_result_t<F>() && {
			return std::move(function)();
		}
		
	private:
		F function;
	};
	
	template<unqualified_object Type>
	inline constexpr auto make_in_place(auto&&... args) requires constructor_callable<Type, decltype(args)...> {
		return in_place_construct{[&] {
			return KANGARU5_NO_ADL(constructor<Type>)(KANGARU5_FWD(args)...);
		}};
	}
	
	template<template<typename...> typename Type>
	inline constexpr auto make_in_place(auto&&... args) requires constructor_callable<decltype(Type(KANGARU5_FWD(args)...)), decltype(args)...> {
		return in_place_construct{[&] {
			return KANGARU5_NO_ADL(constructor<decltype(Type(KANGARU5_FWD(args)...))>)(KANGARU5_FWD(args)...);
		}};
	}
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CONSTRUCTOR_HPP
