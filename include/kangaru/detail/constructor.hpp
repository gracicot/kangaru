#ifndef KANGARU5_DETAIL_CONSTRUCOR_HPP
#define KANGARU5_DETAIL_CONSTRUCOR_HPP

#include "concepts.hpp"

#include "define.hpp"

#define KANGARU5_DEFINE_CONSTRUCT_COMBINAISON(...) \
	template<__VA_ARGS__ Type> INLINE \
	inline constexpr auto construct(int, auto&&... args) -> decltype(Type(KANGARU5_FWD(args)...)) { \
		return Type(KANGARU5_FWD(args)...); \
	} \
	\
	template<__VA_ARGS__ Type> INLINE \
	inline constexpr auto construct(void*, auto&&... args) -> decltype(Type{KANGARU5_FWD(args)...}) { \
		return Type{KANGARU5_FWD(args)...}; \
	}

#define KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(...) \
	template<__VA_ARGS__ Type> \
	inline constexpr auto constructor() { \
		return [](auto&&... args) -> decltype(detail::constructor::construct<Type>(0, KANGARU5_FWD(args)...)) { \
			return detail::constructor::construct<Type>(0, KANGARU5_FWD(args)...); \
		}; \
	}

#define KANGARU5_ALL_COMBINAISONS \
	KANGARU5_X(kangaru::detail::concepts::object) \
	/* KANGARU5_X(template<typename...> typename) \
	KANGARU5_X(template<auto, typename...> typename) \
	KANGARU5_X(template<typename, auto, typename...> typename) \
	KANGARU5_X(template<auto, auto, typename...> typename) \
	KANGARU5_X(template<typename, typename, auto, typename...> typename) \
	KANGARU5_X(template<auto, typename, auto, typename...> typename) \
	KANGARU5_X(template<typename, auto, auto, typename...> typename) \
	KANGARU5_X(template<auto, auto, auto, typename...> typename) \
	KANGARU5_X(template<typename, typename, typename, auto, typename...> typename) \
	KANGARU5_X(template<auto, typename, typename, auto, typename...> typename) \
	KANGARU5_X(template<typename, auto, typename, auto, typename...> typename) \
	KANGARU5_X(template<auto, auto, typename, auto, typename...> typename) \
	KANGARU5_X(template<typename, typename, auto, auto, typename...> typename) \
	KANGARU5_X(template<auto, typename, auto, auto, typename...> typename) \
	KANGARU5_X(template<typename, auto, auto, auto, typename...> typename) \
	KANGARU5_X(template<auto, auto, auto, auto, typename...> typename) \ */
	/* special case for nlohmann basic_json */
	//KANGARU5_X(template<template<typename...> typename, template<typename...> typename, typename, typename, typename, typename, typename, template<typename...> typename, template<typename...> typename, typename...> typename)

namespace kangaru {
	namespace detail::constructor {
		#define KANGARU5_X(...) KANGARU5_DEFINE_CONSTRUCT_COMBINAISON(__VA_ARGS__)
		KANGARU5_ALL_COMBINAISONS
		#undef KANGARU5_X
	}
	
	#define KANGARU5_X(...) KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON(__VA_ARGS__)
	KANGARU5_ALL_COMBINAISONS
	#undef KANGARU5_X
}

#undef KANGARU5_DEFINE_CONSTRUCT_COMBINAISON
#undef KANGARU5_DEFINE_CONSTRUCTOR_COMBINAISON

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CONSTRUCOR_HPP
