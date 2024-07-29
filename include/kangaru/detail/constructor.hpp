#ifndef KANGARU5_DETAIL_CONSTRUCOR_HPP
#define KANGARU5_DETAIL_CONSTRUCOR_HPP

#include "concepts.hpp"
#include "utility.hpp"

#include "define.hpp"

#define KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(...) \
	template<__VA_ARGS__ Type> \
	inline constexpr auto constructor() noexcept { \
		auto const call_constructor = ::kangaru::detail::utility::overload{ \
			[](int, auto&&... args) \
			noexcept(noexcept(Type(KANGARU5_FWD(args)...))) \
			-> decltype(Type(KANGARU5_FWD(args)...)) { \
				return Type(KANGARU5_FWD(args)...); \
			}, \
			[](void*, auto&&... args) \
			noexcept(noexcept(Type{KANGARU5_FWD(args)...})) \
			-> decltype(Type{KANGARU5_FWD(args)...}) { \
				return Type{KANGARU5_FWD(args)...}; \
			}, \
		}; \
		using constructor_t = decltype(call_constructor); \
		return [call_constructor](auto&&... args) \
		noexcept(noexcept(::std::declval<constructor_t>()(0, KANGARU5_FWD(args)...))) \
		-> decltype(::std::declval<constructor_t>()(0, KANGARU5_FWD(args)...)) { \
			return call_constructor(0, KANGARU5_FWD(args)...); \
		}; \
	}


namespace kangaru {
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(::kangaru::unqualified_object)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, auto, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, auto, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, auto, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, template<typename...> typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, template<typename...> typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, template<typename...> typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, auto, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, auto, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, auto, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, template<typename...> typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, template<typename...> typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, template<typename...> typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, typename, typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, typename, typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, typename, typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, auto, typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, auto, typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, auto, typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, template<typename...> typename, typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, template<typename...> typename, typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, template<typename...> typename, typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, typename, auto, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, typename, auto, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, typename, auto, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, auto, auto, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, auto, auto, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, auto, auto, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, template<typename...> typename, auto, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, template<typename...> typename, auto, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, template<typename...> typename, auto, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, typename, template<typename...> typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, typename, template<typename...> typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, typename, template<typename...> typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, auto, template<typename...> typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, auto, template<typename...> typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, auto, template<typename...> typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, template<typename...> typename, template<typename...> typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, template<typename...> typename, template<typename...> typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, template<typename...> typename, template<typename...> typename, auto, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, typename, typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, typename, typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, typename, typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, auto, typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, auto, typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, auto, typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, template<typename...> typename, typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, template<typename...> typename, typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, template<typename...> typename, typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, typename, auto, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, typename, auto, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, typename, auto, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, auto, auto, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, auto, auto, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, auto, auto, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, template<typename...> typename, auto, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, template<typename...> typename, auto, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, template<typename...> typename, auto, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, typename, template<typename...> typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, typename, template<typename...> typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, typename, template<typename...> typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, auto, template<typename...> typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, auto, template<typename...> typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, auto, template<typename...> typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<typename, template<typename...> typename, template<typename...> typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<auto, template<typename...> typename, template<typename...> typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, template<typename...> typename, template<typename...> typename, template<typename...> typename, typename...> typename)
	KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(template<template<typename...> typename, template<typename...> typename, typename, typename, typename, typename, typename, template<typename...> typename, template<typename...> typename, typename...> typename)
}

#undef KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CONSTRUCOR_HPP
