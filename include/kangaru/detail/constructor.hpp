#ifndef KANGARU5_DETAIL_CONSTRUCOR_HPP
#define KANGARU5_DETAIL_CONSTRUCOR_HPP

#include "define.hpp"

namespace kangaru {
	template<typename Type>
	constexpr auto constructor() {
		return [](auto&&... args) -> decltype(Type(KANGARU5_FWD(args)...)) {
			return Type(KANGARU5_FWD(args)...);
		};
	}
	
	template<template<typename...> typename Type>
	constexpr auto constructor() {
		return [](auto&&... args) -> decltype(Type(KANGARU5_FWD(args)...)) {
			return Type(KANGARU5_FWD(args)...);
		};
	}
	
	template<template<auto, typename...> typename Type>
	constexpr auto constructor() {
		return [](auto&&... args) -> decltype(Type(KANGARU5_FWD(args)...)) {
			return Type(KANGARU5_FWD(args)...);
		};
	}
	
	template<template<typename, auto, typename...> typename Type>
	constexpr auto constructor() {
		return [](auto&&... args) -> decltype(Type(KANGARU5_FWD(args)...)) {
			return Type(KANGARU5_FWD(args)...);
		};
	}
	
	template<template<auto, auto, typename...> typename Type>
	constexpr auto constructor() {
		return [](auto&&... args) -> decltype(Type(KANGARU5_FWD(args)...)) {
			return Type(KANGARU5_FWD(args)...);
		};
	}
	
	template<template<typename, typename, auto, typename...> typename Type>
	constexpr auto constructor() {
		return [](auto&&... args) -> decltype(Type(KANGARU5_FWD(args)...)) {
			return Type(KANGARU5_FWD(args)...);
		};
	}
	
	template<template<auto, typename, auto, typename...> typename Type>
	constexpr auto constructor() {
		return [](auto&&... args) -> decltype(Type(KANGARU5_FWD(args)...)) {
			return Type(KANGARU5_FWD(args)...);
		};
	}
	
	template<template<typename, auto, auto, typename...> typename Type>
	constexpr auto constructor() {
		return [](auto&&... args) -> decltype(Type(KANGARU5_FWD(args)...)) {
			return Type(KANGARU5_FWD(args)...);
		};
	}
	
	template<template<auto, auto, auto, typename...> typename Type>
	constexpr auto constructor() {
		return [](auto&&... args) -> decltype(Type(KANGARU5_FWD(args)...)) {
			return Type(KANGARU5_FWD(args)...);
		};
	}
	
	template<template<typename, typename, typename, auto, typename...> typename Type>
	constexpr auto constructor() {
		return [](auto&&... args) -> decltype(Type(KANGARU5_FWD(args)...)) {
			return Type(KANGARU5_FWD(args)...);
		};
	}
	
	template<template<auto, typename, typename, auto, typename...> typename Type>
	constexpr auto constructor() {
		return [](auto&&... args) -> decltype(Type(KANGARU5_FWD(args)...)) {
			return Type(KANGARU5_FWD(args)...);
		};
	}
	
	template<template<typename, auto, typename, auto, typename...> typename Type>
	constexpr auto constructor() {
		return [](auto&&... args) -> decltype(Type(KANGARU5_FWD(args)...)) {
			return Type(KANGARU5_FWD(args)...);
		};
	}
	
	template<template<auto, auto, typename, auto, typename...> typename Type>
	constexpr auto constructor() {
		return [](auto&&... args) -> decltype(Type(KANGARU5_FWD(args)...)) {
			return Type(KANGARU5_FWD(args)...);
		};
	}
	
	template<template<typename, typename, auto, auto, typename...> typename Type>
	constexpr auto constructor() {
		return [](auto&&... args) -> decltype(Type(KANGARU5_FWD(args)...)) {
			return Type(KANGARU5_FWD(args)...);
		};
	}
	
	template<template<auto, typename, auto, auto, typename...> typename Type>
	constexpr auto constructor() {
		return [](auto&&... args) -> decltype(Type(KANGARU5_FWD(args)...)) {
			return Type(KANGARU5_FWD(args)...);
		};
	}
	
	template<template<typename, auto, auto, auto, typename...> typename Type>
	constexpr auto constructor() {
		return [](auto&&... args) -> decltype(Type(KANGARU5_FWD(args)...)) {
			return Type(KANGARU5_FWD(args)...);
		};
	}
	
	template<template<auto, auto, auto, auto, typename...> typename Type>
	constexpr auto constructor() {
		return [](auto&&... args) -> decltype(Type(KANGARU5_FWD(args)...)) {
			return Type(KANGARU5_FWD(args)...);
		};
	}
}

#include "undef.hpp"

#endif // KANGARU5_DETAIL_CONSTRUCOR_HPP
