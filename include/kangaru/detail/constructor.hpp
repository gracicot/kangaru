#ifndef KANGARU5_DETAIL_CONSTRUCTOR_HPP
#define KANGARU5_DETAIL_CONSTRUCTOR_HPP

#include "concepts.hpp"
#include "utility.hpp"
#include "deducer.hpp"

#ifndef KANGARU5_MODULES
#include <concepts>
#include <type_traits>
#endif

#include "define.hpp"

namespace kangaru {
	KANGARU5_EXPORT template<typename T, typename... Args>
	concept raw_constructor_callable = std::constructible_from<T, Args...> or brace_constructible<T, Args...>;
	
	KANGARU5_EXPORT template<unqualified_object Type>
	struct raw_constructor_function {
		inline constexpr auto operator()(auto&&... args) const noexcept requires(
			raw_constructor_callable<Type, decltype(args)...>
		) {
			auto const call_constructor = ::kangaru::detail::utility::overload{
				[](int, auto&&... args)
					noexcept(noexcept(Type(KANGARU5_FWD(args)...)))
					-> decltype(Type(KANGARU5_FWD(args)...)) {
						return Type(KANGARU5_FWD(args)...);
					},
				[](void*, auto&&... args)
					noexcept(noexcept(Type{KANGARU5_FWD(args)...}))
					-> decltype(Type{KANGARU5_FWD(args)...}) {
						return Type{KANGARU5_FWD(args)...};
					},
			};
			return call_constructor(0, KANGARU5_FWD(args)...);
		}
	};
	
	template<unqualified_object Type>
	inline constexpr auto raw_constructor(auto&&... args) -> Type requires(
		callable<raw_constructor_function<Type>, decltype(args)...>
	) {
		return raw_constructor_function<Type>{}(KANGARU5_FWD(args)...);
	}
	
	KANGARU5_EXPORT template<unqualified_object Type>
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
			return KANGARU5_NO_ADL(raw_constructor<Type>)(KANGARU5_NO_ADL(exclude_special_constructors_for<Type>)(deduce1), KANGARU5_FWD(args)...);
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
	
	template<unqualified_object Type>
	inline constexpr auto constructor(auto&&... args) -> Type requires(
		callable<constructor_function<Type>, decltype(args)...>
	) {
		return constructor_function<Type>{}(KANGARU5_FWD(args)...);
	}
	
	template<typename Type, typename... Args>
	concept constructor_callable = unqualified_object<Type> and callable<constructor_function<Type>, Args...>;
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CONSTRUCTOR_HPP
